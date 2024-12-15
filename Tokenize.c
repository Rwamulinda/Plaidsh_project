#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "clist.h"
#include "Tokenize.h"
#include "Token.h"

// Converts TokenType to a human-readable string
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

// Handles escape sequences and validates them
char handle_escape_sequence(char next_char, char *errmsg, size_t errmsg_sz)
{
    switch (next_char)
    {
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case '\\': return '\\';
    case '"': return '\"';
    case '<': return '<';
    case '>': return '>';
    case '|': return '|';
    case ' ': return ' ';
    default:
        snprintf(errmsg, errmsg_sz, "Illegal escape character '\\%c'", next_char);
        return '\0'; // Signal error
    }
}

// Determines if a character signifies the end of a word
int is_at_end(char current)
{
    return current == '\0' || isspace(current) || current == '<' || current == '>' || current == '|';
}

// Tokenizes the input string into a list of tokens
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

        char buffer[256];
        size_t buf_idx = 0;
        int is_quoted = 0;

        // Process quoted and unquoted tokens
        while (input[i] != '\0')
        {
            if (input[i] == '"') // Start or end of a quoted word
            {
                if (is_quoted)
                {
                    // End of quoted word
                    token.type = TOK_QUOTED_WORD;
                    buffer[buf_idx] = '\0';
                    token.value = strdup(buffer);
                    CL_append(tokens, token);
                    buf_idx = 0; // Reset buffer for next token
                }
                is_quoted = !is_quoted;
                i++;
                continue;
            }

            if (!is_quoted && is_at_end(input[i])) // End of unquoted word
                break;

            if (input[i] == '\\') // Escape sequence
            {
                if (input[i + 1] == '\0')
                {
                    snprintf(errmsg, errmsg_sz, "Illegal escape at end of input");
                    free_token_values(tokens);
                    return NULL;
                }

                char escaped = handle_escape_sequence(input[i + 1], errmsg, errmsg_sz);
                if (escaped == '\0') // Invalid escape
                {
                    free_token_values(tokens);
                    return NULL;
                }
                buffer[buf_idx++] = escaped;
                i += 2;
                continue;
            }
            else // Regular character
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

        // Add the last token if any
        if (buf_idx > 0)
        {
            buffer[buf_idx] = '\0';
            if (is_quoted)
            {
                token.type = TOK_QUOTED_WORD;
            }
            else
            {
                token.type = TOK_WORD;
            }
            token.value = strdup(buffer);
            CL_append(tokens, token);
        }
    }

    // Add end-of-input token
    Token end_token = {.type = TOK_END, .value = NULL};
    CL_append(tokens, end_token);

    return tokens;
}

// Frees memory allocated for token values
void free_token_values(CList tokens)
{
    if (!tokens) return;

    size_t len = CL_length(tokens);
    for (size_t i = 0; i < len; i++)
    {
        Token token = CL_nth(tokens, i);
        if (token.value)
            free(token.value);
    }
    CL_free(tokens);
}

// Returns the type of the next token in the list
TokenType TOK_next_type(CList tokens)
{
    if (CL_length(tokens) == 0)
        return TOK_END;
    return CL_nth(tokens, 0).type;
}

// Returns the next token in the list
Token TOK_next(CList tokens)
{
    return CL_nth(tokens, 0);
}

// Consumes the next token in the list
void TOK_consume(CList tokens)
{
    if (CL_length(tokens) > 0)
        CL_pop(tokens);
}

// Prints all tokens in the list
void TOK_print(CList tokens)
{
    size_t len = CL_length(tokens);
    for (size_t i = 0; i < len; i++)
    {
        Token token = CL_nth(tokens, i);
        printf("Token type: %s, Value: %s\n", TT_to_str(token.type), token.value ? token.value : "(null)");
    }
}
