#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdio.h>

// Command structure remains the same
typedef struct {
    char **args;        // Array of arguments for the command
    size_t arg_count;   // Number of arguments
} Command;

// Linked list node for commands
typedef struct CommandNode {
    Command *command;
    struct CommandNode *next;
} CommandNode;

// Updated Pipeline structure
struct Pipeline {
    FILE *input_file;
    FILE *output_file;
    CommandNode *commands;
    size_t command_count;
};

// Forward declaration of Pipeline structure
typedef struct Pipeline Pipeline;

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

// Get the number of commands in the pipeline
size_t pipeline_command_count(Pipeline *pipeline);

// Get a specific command by index
Command* pipeline_get_command(Pipeline *pipeline, size_t index);

#endif // PIPELINE_H