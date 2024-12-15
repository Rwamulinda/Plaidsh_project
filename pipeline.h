#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include "clist.h"

typedef struct {
    char **args;    // Array of arguments for the command
    size_t arg_count;  // Number of arguments
} Command;

typedef struct {
    FILE *input_file;   // Input file for the whole pipeline (default stdin)
    FILE *output_file;  // Output file for the whole pipeline (default stdout)
    CList commands;     // List of commands in the pipeline
} Pipeline;

// Create a new empty pipeline
Pipeline* pipeline_create();

// Add a command to the pipeline
void pipeline_add_command(Pipeline *pipeline, Command *command);

// Set input file for the pipeline
void pipeline_set_input(Pipeline *pipeline, FILE *input);

// Set output file for the pipeline
void pipeline_set_output(Pipeline *pipeline, FILE *output);

// Free the pipeline and all its resources
void pipeline_free(Pipeline *pipeline);

// Create a new command
Command* command_create();

// Add an argument to a command
void command_add_arg(Command *command, const char *arg);

// Free a command and its arguments
void command_free(Command *command);

#endif // PIPELINE_H