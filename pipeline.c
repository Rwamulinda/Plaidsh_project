#include <stdlib.h>
#include <string.h>
#include "pipeline.h"
#include "Tokenize.h"
#include "clist.h"
#include "parse.h"

Pipeline* pipeline_create() {
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (!pipeline) return NULL;

    pipeline->input_file = stdin;
    pipeline->output_file = stdout;
    pipeline->commands = CL_new();
    return pipeline;
}

void pipeline_add_command(Pipeline *pipeline, Command *command) {
    if (!pipeline || !command) return;
    CL_append(pipeline->commands, *command);
}

void pipeline_set_input(Pipeline *pipeline, FILE *input) {
    if (!pipeline) return;
    pipeline->input_file = input ? input : stdin;
}

void pipeline_set_output(Pipeline *pipeline, FILE *output) {
    if (!pipeline) return;
    pipeline->output_file = output ? output : stdout;
}

void pipeline_free(Pipeline *pipeline) {
    if (!pipeline) return;

    // Free each command in the list
    size_t len = CL_length(pipeline->commands);
    for (size_t i = 0; i < len; i++) {
        Command *cmd = (Command*)& CL_nth(pipeline->commands, i);
        command_free(cmd);
    }

    // Free the commands list
    CL_free(pipeline->commands);

    // Close files if they're not stdin/stdout
    if (pipeline->input_file && pipeline->input_file != stdin) 
        fclose(pipeline->input_file);
    if (pipeline->output_file && pipeline->output_file != stdout) 
        fclose(pipeline->output_file);

    free(pipeline);
}

Command* command_create() {
    Command *cmd = malloc(sizeof(Command));
    if (!cmd) return NULL;

    cmd->args = NULL;
    cmd->arg_count = 0;
    return cmd;
}

void command_add_arg(Command *command, const char *arg) {
    if (!command || !arg) return;

    // Reallocate args array to add new argument
    command->args = realloc(command->args, (command->arg_count + 1) * sizeof(char*));
    if (!command->args) return;

    // Duplicate the arg string and add to args
    command->args[command->arg_count] = strdup(arg);
    command->arg_count++;
}

void command_free(Command *command) {
    if (!command) return;

    // Free each argument string
    for (size_t i = 0; i < command->arg_count; i++) {
        free(command->args[i]);
    }

    // Free the args array
    free(command->args);
}