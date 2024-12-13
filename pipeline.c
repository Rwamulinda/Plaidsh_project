#include <stdlib.h>
#include <string.h>
#include "pipeline.h"

Pipeline *pipeline_new() {
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (pipeline == NULL) {
        return NULL;
    }
    pipeline->num_commands = 0;
    pipeline->input_file = stdin;
    pipeline->output_file = stdout;
    return pipeline;
}

void pipeline_add_command(Pipeline *pipeline, char **argv, int argc) {
    if (pipeline->num_commands >= MAX_COMMANDS) {
        return;
    }

    Command *cmd = &pipeline->commands[pipeline->num_commands];
    cmd->argc = argc;

    for (int i = 0; i < argc; i++) {
        cmd->argv[i] = strdup(argv[i]);
    }
    cmd->argv[argc] = NULL;  // NULL-terminate argv

    pipeline->num_commands++;
}

void pipeline_set_input(Pipeline *pipeline, FILE *input) {
    pipeline->input_file = input;
}

void pipeline_set_output(Pipeline *pipeline, FILE *output) {
    pipeline->output_file = output;
}

void pipeline_free(Pipeline *pipeline) {
    for (int i = 0; i < pipeline->num_commands; i++) {
        Command *cmd = &pipeline->commands[i];
        for (int j = 0; j < cmd->argc; j++) {
            free(cmd->argv[j]);
        }
    }
    free(pipeline);
}