/*
 * token.h
 *
 * Enum containing all possible tokens
 *
 * Author: <Uwase Pauline>
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_

typedef enum
{
    TOK_WORD,
    TOK_QUOTED_WORD,
    TOK_LESSTHAN,
    TOK_GREATERTHAN,
    TOK_PIPE,
    TOK_END
} TokenType;

typedef struct
{
    TokenType type; // Type of token (WORD, QUOTED_WORD, etc.)
    char *value;    // The actual string value of the token
} Token;

#endif /* _TOKEN_H_ */
