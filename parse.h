#ifndef PARSE_H
#define PARSE_H

#include "pipeline.h"
#include "Tokenize.h"

// Parse tokens into a pipeline
Pipeline *parse_tokens(CList tokens);

#endif // PARSE_H