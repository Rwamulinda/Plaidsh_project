/*
 * tokenize.c
 *
 * Functions to tokenize and manipulate lists of tokens
 *
 * Author: <Uwase Pauline>
 */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "clist.h"
#include "Tokenize.h"
#include "Token.h"
#include <stddef.h>

// Documented in .h file
const char *TT_to_str(TokenType tt)
{
    switch (tt)
    {

    case TOK_WORD:
        return "WORD";
    case TOK_QUOTED_WORD:
        return "QUOTED_WORD";
    case TOK_LESSTHAN:
        return "LESSTHAN";
    case TOK_GREATERTHAN:
        return "GREATERTHAN";
    case TOK_PIPE:
        return "PIPE";
    case TOK_END:
        return "(end)";
    default:
        return "UNKNOWN";
    }

    __builtin_unreachable();
}

char handle_escape_sequence(char next_char, char *errmsg, size_t errmsg_sz)
{
    switch (next_char)
    {
    case 'n':
        return '\n'; // New line
    case 'r':
        return '\r'; // Carriage return
    case 't':
        return '\t'; // Tab
    case '"':
        return '\"'; // Double quote
    case '\\':
        return '\\'; // Backslash
    case ' ':
        return ' '; // Space
    case '|':
        return '|'; // Pipe
    case '<':
        return '<'; // Less than
    case '>':
        return '>'; // Greater than
    default:
        snprintf(errmsg, errmsg_sz, "Unrecognized escape sequence: \\%c", next_char);
        return '\0'; // Indicate an error
    }
}

// This function checks if the current character marks the end of a word
int is_at_end(const char current)
{
    // Check if it's one of the special characters (unescaped <, >, |, " or space)
    if (current == '<' || current == '>' || current == '|' || current == '"')
        return 1; // End of word

    // Check for unescaped whitespace
    if (isspace(current))
        return 1; // End of word

    // Check if we are at the end of the input string
    if (current == '\0')
        return 1; // End of word

    return 0; // Not at the end of the word yet
}

// Documented in .h file
CList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
    CList tokens = CL_new();
    size_t i = 0;

    if (input == NULL) {
        snprintf(errmsg, errmsg_sz, "Null input provided");
        CL_free(tokens);  // Free the list if input is NULL
        return NULL;
    }

    while (input[i] != '\0')
    {
        // Skip whitespace
        if (isspace(input[i]))
        {
            i++;
            continue;
        }

        Token token = {0};

        // Handle special characters first
        if (input[i] == '<' || input[i] == '>' || input[i] == '|')
        {
            switch (input[i])
            {
            case '<':
                token.type = TOK_LESSTHAN;
                break;
            case '>':
                token.type = TOK_GREATERTHAN;
                break;
            case '|':
                token.type = TOK_PIPE;
                break;
            }
            token.value = strdup(input[i] == '<' ? "<" : (input[i] == '>' ? ">" : "|"));
            CL_append(tokens, token);
            i++;
            continue;
        }

        char temp[256];
        size_t temp_idx = 0;
        int is_quoted = 0;
        int had_quote = 0;

        // Improved quote handling
        while (1)
        {
            if (input[i] == '"')
            {
                if (!is_quoted)
                {
                    is_quoted = 1;
                    had_quote = 1;
                    i++; // Skip opening quote
                    continue;
                }
                else
                {
                    // Check for EOF while in quotes
                    if (input[i + 1] == '\0') 
                    {
                        snprintf(errmsg, errmsg_sz, "Unterminated quote");
                        free_token_values(tokens);  // Free all allocated tokens
                        return NULL;
                    }
                    
                    is_quoted = 0;
                    i++; // Skip closing quote
                    break;
                }
            }

            // End of input or word conditions
            if (input[i] == '\0' ||
                (!is_quoted && (input[i] == '<' || input[i] == '>' || input[i] == '|' || isspace(input[i]))))
                break;

            // Handle backslash
            if (input[i] == '\\')
            {
                if (input[i + 1] == '\0')
                {
                    snprintf(errmsg, errmsg_sz, "Illegal escape character");
                    free_token_values(tokens);  // Free all allocated tokens
                    return NULL;
                }

                // Improved escape sequence handling
                char escaped = handle_escape_sequence(input[i + 1], errmsg, errmsg_sz);
                if (escaped == '\0')
                {
                    // For quoted strings, allow more characters to be escaped
                    if (is_quoted)
                    {
                        temp[temp_idx++] = input[i + 1];
                        i += 2;
                        continue;
                    }
                    
                    snprintf(errmsg, errmsg_sz, "Illegal escape character '\\%c'", input[i + 1]);
                    free_token_values(tokens);  // Free all allocated tokens
                    return NULL;
                }
                temp[temp_idx++] = escaped;
                i += 2;
            }
            else
            {
                temp[temp_idx++] = input[i];
                i++;
            }
        }

        // Create token if we have content
        if (temp_idx > 0)
        {
            temp[temp_idx] = '\0';
            
            // Determine token type
            if (is_quoted || had_quote)
            {
                token.type = TOK_QUOTED_WORD;
            }
            else
            {
                token.type = TOK_WORD;
            }
            
            token.value = strdup(temp);
            CL_append(tokens, token);
        }
    }

    // Add end-of-input token
    Token end_token = {.type = TOK_END, .value = NULL};
    CL_append(tokens, end_token);
    return tokens;
}
// Documented in .h file
void free_token_values(CList tokens)
{
    if (tokens == NULL) 
        return;

    // Iterate through the tokens and free each token's value
    size_t length = CL_length(tokens);
    for (size_t i = 0; i < length; i++) 
    {
        Token token = CL_nth(tokens, i);
        
        // Free the dynamically allocated value string
        // But only for tokens that have a non-NULL value
        if (token.value != NULL) 
        {
            free(token.value);
        }
    }

    // Optional: Clear the list after freeing values
    CL_free(tokens);
}
// Documented in .h file
TokenType TOK_next_type(CList tokens)
{
    if (CL_length(tokens) == 0)
    {
        return TOK_END;
    }
    Token token = CL_nth(tokens, 0);
    return token.type;
}

// Documented in .h file
Token TOK_next(CList tokens)
{
    return CL_nth(tokens, 0);
}

// Documented in .h file
void TOK_consume(CList tokens)
{
    if (CL_length(tokens) > 0)
    {
        CL_pop(tokens);
    }
}

void printToken(int pos, CListElementType element, void *cb_data)
{
    if (element.type == TOK_WORD || element.type == TOK_QUOTED_WORD)
    {
        printf("Position %d: Token type: %s, Text: %s\n", pos, TT_to_str(element.type), element.value);
    }
    else
    {
        printf("Position %d: Token type: %s\n", pos, TT_to_str(element.type));
    }
}

// Documented in .h file
void TOK_print(CList tokens)
{
    CL_foreach(tokens, printToken, NULL);
}
