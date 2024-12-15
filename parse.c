#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "parse.h"
#include "Tokenize.h"
#include "pipeline.h"

Pipeline* parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz) {
    Pipeline *pipeline = pipeline_create();
    Command *current_command = NULL;
    int input_redirected = 0;
    int output_redirected = 0;

    while (TOK_next_type(tokens) != TOK_END) {
        Token token = TOK_next(tokens);

        switch (token.type) {
            case TOK_WORD:
            case TOK_QUOTED_WORD:
                // Start a new command if needed
                if (!current_command) {
                    current_command = command_create();
                }
                
                // Handle globbing for non-quoted words
                if (token.type == TOK_WORD) {
                    glob_t globbuf;
                    int glob_result = glob(token.value, GLOB_TILDE_CHECK, NULL, &globbuf);
                    
                    if (glob_result == 0) {
                        // Add globbed files
                        for (size_t i = 0; i < globbuf.gl_pathc; i++) {
                            command_add_arg(current_command, globbuf.gl_pathv[i]);
                        }
                        globfree(&globbuf);
                    } else {
                        // If no glob match, add the original word
                        command_add_arg(current_command, token.value);
                    }
                } else {
                    // For quoted words, add as-is
                    command_add_arg(current_command, token.value);
                }
                break;

            case TOK_LESSTHAN:
                // Input redirection
                if (input_redirected) {
                    snprintf(errmsg, errmsg_sz, "Multiple input redirections not allowed");
                    pipeline_free(pipeline);
                    return NULL;
                }
                TOK_consume(tokens);
                
                if (TOK_next_type(tokens) != TOK_WORD && TOK_next_type(tokens) != TOK_QUOTED_WORD) {
                    snprintf(errmsg, errmsg_sz, "Expected filename after input redirection");
                    pipeline_free(pipeline);
                    return NULL;
                }
                
                FILE *input_file = fopen(TOK_next(tokens).value, "r");
                if (!input_file) {
                    snprintf(errmsg, errmsg_sz, "Cannot open input file: %s", TOK_next(tokens).value);
                    pipeline_free(pipeline);
                    return NULL;
                }
                pipeline_set_input(pipeline, input_file);
                input_redirected = 1;
                break;

            case TOK_GREATERTHAN:
                // Output redirection
                if (output_redirected) {
                    snprintf(errmsg, errmsg_sz, "Multiple output redirections not allowed");
                    pipeline_free(pipeline);
                    return NULL;
                }
                TOK_consume(tokens);
                
                if (TOK_next_type(tokens) != TOK_WORD && TOK_next_type(tokens) != TOK_QUOTED_WORD) {
                    snprintf(errmsg, errmsg_sz, "Expected filename after output redirection");
                    pipeline_free(pipeline);
                    return NULL;
                }
                
                FILE *output_file = fopen(TOK_next(tokens).value, "w");
                if (!output_file) {
                    snprintf(errmsg, errmsg_sz, "Cannot open output file: %s", TOK_next(tokens).value);
                    pipeline_free(pipeline);
                    return NULL;
                }
                pipeline_set_output(pipeline, output_file);
                output_redirected = 1;
                break;

            case TOK_PIPE:
                // Add current command to pipeline and prepare for next command
                if (!current_command) {
                    snprintf(errmsg, errmsg_sz, "Unexpected pipe");
                    pipeline_free(pipeline);
                    return NULL;
                }
                pipeline_add_command(pipeline, current_command);
                current_command = NULL;
                break;

            default:
                snprintf(errmsg, errmsg_sz, "Unexpected token type");
                pipeline_free(pipeline);
                return NULL;
        }

        // Consume the current token
        TOK_consume(tokens);
    }

    // Add the last command if exists
    if (current_command) {
        pipeline_add_command(pipeline, current_command);
    }

    return pipeline;
}