/*
 * clist.c
 * 
 * Linked list implementation for ISSE Assignment 5
 *
 * Author: Pauline Uwase
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "clist.h"
#include "Token.h"


#define DEBUG

struct _cl_node {
  CListElementType element;
  struct _cl_node *next;
};

struct _clist {
  struct _cl_node *head;
  int length;
};



/*
 * Create (malloc) a new _cl_node and populate it with the supplied
 * values
 *
 * Parameters:
 *   element, next  the values for the node to be created
 * 
 * Returns: The newly-malloc'd node, or NULL in case of error
 */
static struct _cl_node*
_CL_new_node(CListElementType element, struct _cl_node *next)
{
  struct _cl_node* new = (struct _cl_node*) malloc(sizeof(struct _cl_node));

  assert(new);

  new->element = element;
  new->next = next;

  return new;
}



// Documented in .h file
CList CL_new()
{
  CList list = (CList) malloc(sizeof(struct _clist));
  assert(list);

  list->head = NULL;
  list->length = 0;

  return list;
}



// Documented in .h file
void CL_free(CList list)
{
    // If the list is NULL, there is nothing to free.
    if (list == NULL)
        return;

    // Traverse the list and free each node.
    struct _cl_node *current = list->head;
    while (current != NULL)
    {
        struct _cl_node *next_node = current->next; // Store reference to the next node.
        free(current);                              // Free the current node.
        current = next_node;                        // Move to the next node.
    }

    // Free the list structure itself.
    free(list);
}




// Documented in .h file
int CL_length(CList list)
{
  assert(list);
#ifdef DEBUG
  // In production code, we simply return the stored value for
  // length. However, as a defensive programming method to prevent
  // bugs in our code, in DEBUG mode we walk the list and ensure the
  // number of elements on the list is equal to the stored length.

  int len = 0;
  for (struct _cl_node *node = list->head; node != NULL; node = node->next)
    len++;

  assert(len == list->length);
#endif // DEBUG

  return list->length;
}

// Documented in .h file
void CL_push(CList list, CListElementType element)
{
  assert(list);
  list->head = _CL_new_node(element, list->head);
  list->length++;
}



// Documented in .h file
CListElementType CL_pop(CList list)
{
  assert(list);

  struct _cl_node *popped_node = list->head;

  if (popped_node == NULL)
    return INVALID_RETURN;

  CListElementType ret = popped_node->element;

  // unlink previous head node, then free it
  list->head = popped_node->next;
  free(popped_node);
  // we cannot refer to popped node any longer

  list->length--;

  return ret;
}



// Documented in .h file
void CL_append(CList list, CListElementType element)
{
    assert(list);  // Ensure the list is valid

    struct _cl_node *new_node = _CL_new_node(element, NULL);
    assert(new_node);

    if (list->head == NULL) {
        // If the list is empty, the new node is the head.
        list->head = new_node;
    } else {
        // Traverse to the end of the list and insert the new node.
        struct _cl_node *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    // Increment the length of the list.
    list->length++;
}
// Documented in .h file
CListElementType CL_nth(CList list, int pos)
{
    assert(list);

    // If position is out of range, return INVALID_RETURN.
    if (pos < -list->length || pos >= list->length)
        return INVALID_RETURN;

    // Convert negative position to positive equivalent.
    if (pos < 0)
        pos = list->length + pos;

    struct _cl_node *current = list->head;
    for (int i = 0; i < pos; i++) {
        current = current->next;
    }

    return current->element;
}
// Documented in .h file
bool CL_insert(CList list, CListElementType element, int pos)
{
    assert(list);

    // Check if position is out of bounds
    if (pos < -list->length - 1 || pos > list->length)
        return false;

    // Convert negative position to positive equivalent.
    if (pos < 0)
        pos = list->length + pos + 1;

    if (pos == 0) {
        // Insert at the head.
        CL_push(list, element);
    } else {
        struct _cl_node *current = list->head;

        // Traverse to the node before the position where we want to insert.
        for (int i = 0; i < pos - 1; i++) {
            current = current->next;
        }

        // Insert the new node.
        struct _cl_node *new_node = _CL_new_node(element, current->next);
        current->next = new_node;

        list->length++;
    }

    return true;
}
// Documented in .h file
CListElementType CL_remove(CList list, int pos)
{
    assert(list);

    // Check if position is out of bounds
    if (pos < -list->length || pos >= list->length)
        return INVALID_RETURN;

    // Convert negative position to positive equivalent.
    if (pos < 0)
        pos = list->length + pos;

    struct _cl_node *current = list->head;
    CListElementType removed_element;

    if (pos == 0) {
        // Remove the head element.
        removed_element = CL_pop(list);
    } else {
        // Traverse to the node before the one we want to remove.
        for (int i = 0; i < pos - 1; i++) {
            current = current->next;
        }

        struct _cl_node *node_to_remove = current->next;
        removed_element = node_to_remove->element;
        current->next = node_to_remove->next;

        free(node_to_remove);
        list->length--;
    }

    return removed_element;
}
// Documented in .h file
CList CL_copy(CList src_list)
{
    assert(src_list);

    CList new_list = CL_new();

    struct _cl_node *current = src_list->head;

    // Traverse the source list and append each element to the new list.
    while (current != NULL) {
        CL_append(new_list, current->element);
        current = current->next;
    }

    return new_list;
}
// Documented in .h file
void CL_join(CList list1, CList list2)
{
    assert(list1);
    assert(list2);

    if (list2->head == NULL)
        return;  // list2 is empty, nothing to do.

    if (list1->head == NULL) {
        // If list1 is empty, just set list1->head to list2->head.
        list1->head = list2->head;
    } else {
        // Traverse to the end of list1.
        struct _cl_node *current = list1->head;
        while (current->next != NULL) {
            current = current->next;
        }

        // Link list2 at the end of list1.
        current->next = list2->head;
    }

    // Update length of list1 and set list2 to empty.
    list1->length += list2->length;
    list2->head = NULL;
    list2->length = 0;
}


// Documented in .h file
void CL_reverse(CList list)
{
    assert(list);

    struct _cl_node *prev = NULL, *current = list->head, *next = NULL;

    while (current != NULL) {
        next = current->next;  // Store reference to next node.
        current->next = prev;  // Reverse the link.
        prev = current;        // Move `prev` to current node.
        current = next;        // Move `current` to next node.
    }

    list->head = prev;  // Update the head of the list.
}



// Documented in .h file
void CL_foreach(CList list, CL_foreach_callback callback, void *cb_data)
{
    assert(list);
    assert(callback);

    struct _cl_node *current = list->head;
    int pos = 0;

    while (current != NULL) {
        callback(pos, current->element, cb_data);
        current = current->next;
        pos++;
    }
}