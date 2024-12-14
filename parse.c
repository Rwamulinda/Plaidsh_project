#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "parse.h"

char **perform_globbing(char *pattern) {
    glob_t globresult;
    int rc;
    char **expanded_args = NULL;
    int count = 0;

    rc = glob(pattern, GLOB_TILDE_CHECK, NULL, &globresult);
    if (rc == 0) {
        expanded_args = malloc((globresult.gl_pathc + 1) * sizeof(char *));
        for (size_t i = 0; i < globresult.gl_pathc; i++) {
            expanded_args[count++] = strdup(globresult.gl_pathv[i]);
        }
        expanded_args[count] = NULL;
        globfree(&globresult);
    } else {
        // Single expansion with duplicate of original pattern
        expanded_args = malloc(2 * sizeof(char *));
        expanded_args[0] = strdup(pattern);
        expanded_args[1] = NULL;
    }

    return expanded_args;
}

void free_expanded_args(char **expanded_args) {
    if (expanded_args) {
        for (int i = 0; expanded_args[i] != NULL; i++) {
            free(expanded_args[i]);
        }
        free(expanded_args);
    }
}

Pipeline *parse_tokens(CList tokens) {
    Pipeline *pipeline = pipeline_new();
    char *command_argv[MAX_ARGS];
    int command_argc = 0;
    int redirect_input = 0, redirect_output = 0;
    FILE *input_file = NULL, *output_file = NULL;

    while (TOK_next_type(tokens) != TOK_END) {
        Token token = TOK_next(tokens);

        switch (token.type) {
            case TOK_WORD:
            case TOK_QUOTED_WORD: {
                // Perform globbing for non-quoted words
                char **expanded_args = (token.type == TOK_WORD) ? 
                    perform_globbing(token.value) : malloc(2 * sizeof(char *));
                
                if (token.type == TOK_QUOTED_WORD) {
                    expanded_args[0] = strdup(token.value);
                    expanded_args[1] = NULL;
                }

                // Add expanded arguments to current command
                for (int i = 0; expanded_args[i] != NULL; i++) {
                    if (command_argc < MAX_ARGS - 1) {
                        command_argv[command_argc++] = expanded_args[i];
                    }
                }
                free(expanded_args);
                break;
            }
            case TOK_LESSTHAN: {
                TOK_consume(tokens);
                token = TOK_next(tokens);
                input_file = fopen(token.value, "r");
                redirect_input = 1;
                break;
            }
            case TOK_GREATERTHAN: {
                TOK_consume(tokens);
                token = TOK_next(tokens);
                output_file = fopen(token.value, "w");
                redirect_output = 1;
                break;
            }
            case TOK_PIPE: {
                // Add current command to pipeline and start a new command
                command_argv[command_argc] = NULL;
                pipeline_add_command(pipeline, command_argv, command_argc);
                command_argc = 0;
                break;
            }
            default:
                break;
        }

        TOK_consume(tokens);
    }

    // Add last command
    if (command_argc > 0) {
        command_argv[command_argc] = NULL;
        pipeline_add_command(pipeline, command_argv, command_argc);
    }

    // Set input/output files if redirected
    if (redirect_input && input_file) {
        pipeline_set_input(pipeline, input_file);
    }
    if (redirect_output && output_file) {
        pipeline_set_output(pipeline, output_file);
    }

    return pipeline;
}