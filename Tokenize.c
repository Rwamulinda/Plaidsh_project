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

// Modified handle_escape_sequence to check for allowed escape sequences
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
        return ' '; // Space (allowed after backslash)
    case '|':
        return '|'; // Pipe
    case '<':
        return '<'; // Less than
    case '>':
        return '>'; // Greater than
    default:
        // Report error for illegal escape sequences
        snprintf(errmsg, errmsg_sz, "Unrecognized escape sequence: \\%c", next_char);
        return '\0'; // Indicate an error
    }
}

// Modified TOK_tokenize_input to handle illegal backslash escape cases
CList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
    CList tokens = CL_new();
    size_t i = 0;

    if (!input)
    {
        snprintf(errmsg, errmsg_sz, "Null input provided");
        return NULL;
    }

    while (input[i] != '\0')
    {
        if (isspace(input[i]))
        {
            i++;
            continue;
        }

        Token token = {0};

        // Handle special characters
        if (input[i] == '<' || input[i] == '>' || input[i] == '|')
        {
            token.type = (input[i] == '<') ? TOK_LESSTHAN : (input[i] == '>') ? TOK_GREATERTHAN : TOK_PIPE;
            token.value = strndup(&input[i], 1);
            CL_append(tokens, token);
            i++;
            continue;
        }

        // Check for escape sequences
        if (input[i] == '\\') // Backslash detected, check the next character
        {
            if (input[i + 1] == '\0') // If the backslash is the last character, it's an error
            {
                snprintf(errmsg, errmsg_sz, "Trailing backslash at the end of input");
                free_token_values(tokens);
                return NULL;
            }

            // Process the escape sequence (next character after the backslash)
            char next_char = input[i + 1];
            char result = handle_escape_sequence(next_char, errmsg, errmsg_sz);

            if (result == '\0') // If handle_escape_sequence returns '\0', there's an error
            {
                free_token_values(tokens);
                return NULL;
            }

            token.type = TOK_WORD; // Treat the result as a valid token
            token.value = strndup(&result, 1); // Copy the escaped character
            CL_append(tokens, token);
            i += 2; // Move past the backslash and the escaped character
            continue;
        }

        char buffer[256];
        size_t buf_idx = 0;

        // Process quoted and unquoted tokens
        if (input[i] == '"') // Start of a quoted word
        {
            i++; // Skip the opening quote
            while (input[i] != '\0' && (input[i] != '"' || (i > 0 && input[i - 1] == '\\'))) // Look for the closing quote or escape sequence
            {
                if (input[i] == '\\' && (input[i + 1] == '"' || input[i + 1] == '\\')) // Handle escaped quotes and backslashes
                {
                    buffer[buf_idx++] = input[i + 1];
                    i += 2;
                }
                else
                {
                    buffer[buf_idx++] = input[i++];
                }

                if (buf_idx >= sizeof(buffer) - 1)
                {
                    snprintf(errmsg, errmsg_sz, "Token too long");
                    free_token_values(tokens);
                    return NULL;
                }
            }

            if (input[i] != '"') // If we end without a closing quote, report an error
            {
                snprintf(errmsg, errmsg_sz, "Unterminated quote");
                free_token_values(tokens);
                return NULL;
            }

            token.type = TOK_QUOTED_WORD;
            buffer[buf_idx] = '\0';
            token.value = strdup(buffer);
            CL_append(tokens, token);
            i++; // Skip the closing quote
            continue;
        }

        // Process unquoted word
        while (input[i] != '\0' && !isspace(input[i]) && input[i] != '<' && input[i] != '>' && input[i] != '|')
        {
            buffer[buf_idx++] = input[i++];
            if (buf_idx >= sizeof(buffer) - 1)
            {
                snprintf(errmsg, errmsg_sz, "Token too long");
                free_token_values(tokens);
                return NULL;
            }
        }

        if (buf_idx > 0) // If a valid word was found
        {
            buffer[buf_idx] = '\0';
            token.type = TOK_WORD;
            token.value = strdup(buffer);
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
        printf("Position %d: Token type: %s, %s\n", pos, TT_to_str(element.type), element.value);
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
