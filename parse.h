#ifndef PARSE_H
#define PARSE_H

#include "clist.h"
#include "pipeline.h"
#include "Tokenize.h"

// Parse a list of tokens into a Pipeline
// Returns NULL if parsing fails
Pipeline* parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz);

#endif // PARSE_H