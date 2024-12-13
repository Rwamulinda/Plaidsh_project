#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include "Tokenize.h"

#define MAX_ARGS 256
#define MAX_COMMANDS 64

typedef struct {
    char *argv[MAX_ARGS];
    int argc;
} Command;

typedef struct {
    Command commands[MAX_COMMANDS];
    int num_commands;
    FILE *input_file;
    FILE *output_file;
} Pipeline;

// Initialize a new pipeline
Pipeline *pipeline_new();

// Add a command to the pipeline
void pipeline_add_command(Pipeline *pipeline, char **argv, int argc);

// Set input file for the pipeline
void pipeline_set_input(Pipeline *pipeline, FILE *input);

// Set output file for the pipeline
void pipeline_set_output(Pipeline *pipeline, FILE *output);

// Free pipeline and its resources
void pipeline_free(Pipeline *pipeline);

#endif // PIPELINE_H