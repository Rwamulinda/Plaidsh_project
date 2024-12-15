#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pipeline.h"

// Create a new pipeline
Pipeline* pipeline_create() {
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (!pipeline) return NULL;
    
    pipeline->input_file = stdin;
    pipeline->output_file = stdout;
    pipeline->commands = NULL;
    pipeline->command_count = 0;
    
    return pipeline;
}

// Add a command to the pipeline
void pipeline_add_command(Pipeline *pipeline, Command *command) {
    if (!pipeline || !command) return;
    
    // Create a new command node
    CommandNode *new_node = malloc(sizeof(CommandNode));
    if (!new_node) return;
    
    new_node->command = command;
    new_node->next = NULL;
    
    // If list is empty, set as first node
    if (!pipeline->commands) {
        pipeline->commands = new_node;
    } else {
        // Find the last node and append
        CommandNode *current = pipeline->commands;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
    
    pipeline->command_count++;
}

// Set input file
void pipeline_set_input(Pipeline *pipeline, FILE *input) {
    if (!pipeline) return;
    pipeline->input_file = input ? input : stdin;
}

// Set output file
void pipeline_set_output(Pipeline *pipeline, FILE *output) {
    if (!pipeline) return;
    pipeline->output_file = output ? output : stdout;
}

// Free the entire pipeline
void pipeline_free(Pipeline *pipeline) {
    if (!pipeline) return;
    
    // Free all command nodes
    CommandNode *current = pipeline->commands;
    while (current) {
        CommandNode *next = current->next;
        
        // Free the command inside the node
        if (current->command) {
            command_free(current->command);
        }
        
        // Free the node itself
        free(current);
        current = next;
    }
    
    // Close files if they're not stdin/stdout
    if (pipeline->input_file && pipeline->input_file != stdin) 
        fclose(pipeline->input_file);
    if (pipeline->output_file && pipeline->output_file != stdout) 
        fclose(pipeline->output_file);
    
    // Free the pipeline structure
    free(pipeline);
}

// Create a new command
Command* command_create() {
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) return NULL;
    
    cmd->args = NULL;
    cmd->arg_count = 0;
    
    return cmd;
}

// Add an argument to a command
void command_add_arg(Command *command, const char *arg) {
    if (!command || !arg) return;
    
    // Reallocate args array to add new argument
    char **new_args = realloc(command->args, (command->arg_count + 1) * sizeof(char*));
    if (!new_args) return;
    
    command->args = new_args;
    
    // Duplicate the arg string and add to args
    command->args[command->arg_count] = strdup(arg);
    command->arg_count++;
}

// Free a command
void command_free(Command *command) {
    if (!command) return;
    
    // Free each argument string
    for (size_t i = 0; i < command->arg_count; i++) {
        free(command->args[i]);
    }
    
    // Free the args array
    free(command->args);
    
    // Free the command structure itself
    free(command);
}

// Get the number of commands in the pipeline
size_t pipeline_command_count(Pipeline *pipeline) {
    if (!pipeline) return 0;
    return pipeline->command_count;
}

// Get a specific command by index
Command* pipeline_get_command(Pipeline *pipeline, size_t index) {
    if (!pipeline) return NULL;
    
    CommandNode *current = pipeline->commands;
    for (size_t i = 0; i < index && current; i++) {
        current = current->next;
    }
    
    return current ? current->command : NULL;
}