#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "Tokenize.h"
#include "parse.h"
#include "pipeline.h"


// Function implementations before they are used
void do_author(void) {
    printf("Author: Uwase Pauline \n");
}

int do_cd(const char *path) {
    if (path == NULL) {
        // Handle case of no path (go to home directory)
        path = getenv("HOME");
    }
    
    if (chdir(path) != 0) {
        perror("cd");
        return 1;
    }
    return 0;
}

void do_pwd(void) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
}


// Add exit as a built-in command
int do_exit(int status) {
    exit(status);
    return 0; // Never reached, but keeps compiler happy
}



// Add exit as a built-in command

// Check if a command is a built-in
int is_builtin(const char *command) {
    const char *builtin_commands[] = {"author", "cd", "pwd", "exit"};
    for (size_t i = 0; i < sizeof(builtin_commands)/sizeof(builtin_commands[0]); i++) {
        if (strcmp(command, builtin_commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Execute built-in commands
int execute_builtin(Command *cmd) {
    if (strcmp(cmd->args[0], "author") == 0) {
        do_author();
        return 0;
    } else if (strcmp(cmd->args[0], "cd") == 0) {
        return do_cd(cmd->args[1]);
    } else if (strcmp(cmd->args[0], "pwd") == 0) {
        do_pwd();
        return 0;
    } else if (strcmp(cmd->args[0], "exit") == 0) {
        // Exit with status 0 by default, or use provided status
        int status = cmd->arg_count > 1 ? atoi(cmd->args[1]) : 0;
        do_exit(status);
    }
    return -1;
}

int execute_pipeline(Pipeline *pipeline) {
    if (!pipeline || pipeline_command_count(pipeline) == 0) {
        return -1;
    }

    // Handle single built-in command case
    if (pipeline_command_count(pipeline) == 1) {
        Command *cmd = pipeline_get_command(pipeline, 0);
        if (is_builtin(cmd->args[0])) {
            return execute_builtin(cmd);
        }
    }

    // Prepare for pipeline execution
    size_t command_count = pipeline_command_count(pipeline);
    int pipes[command_count - 1][2];
    pid_t pids[command_count];

    // Create pipes between commands
    for (size_t i = 0; i < command_count - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return -1;
        }
    }

    // Fork and execute each command
    for (size_t i = 0; i < command_count; i++) {
        Command *cmd = pipeline_get_command(pipeline, i);
        
        pids[i] = fork();

        if (pids[i] == -1) {
            perror("fork");
            return -1;
        }

        if (pids[i] == 0) {  // Child process
            // Handle input redirection
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 input");
                    exit(EXIT_FAILURE);
                }
            }

            // Handle output redirection
            if (i < command_count - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 output");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe fds
            for (size_t j = 0; j < command_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Check if it's a built-in command
            if (is_builtin(cmd->args[0])) {
                exit(execute_builtin(cmd));
            }

            // External command
            // Ensure args array is null-terminated for execvp
            char **exec_args = realloc(cmd->args, (cmd->arg_count + 1) * sizeof(char*));
            exec_args[cmd->arg_count] = NULL;

            execvp(exec_args[0], exec_args);
            
            // If execvp fails, print a more specific error
            if (errno == ENOENT) {
                fprintf(stderr, "%s: Command not found\n", exec_args[0]);
            } else {
                perror(exec_args[0]);
            }
            exit(EXIT_FAILURE);
        }
    }

    // Parent process: close all pipe file descriptors
    for (size_t i = 0; i < command_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes
    int status;
    int last_status = 0;
    for (size_t i = 0; i < command_count; i++) {
        waitpid(pids[i], &status, 0);
        if (i == command_count - 1) {
            last_status = WEXITSTATUS(status);
        }
    }

    return last_status;
}

int main() {
    // Setup readline
    rl_bind_key('\t', rl_complete);
    printf("Welcome to Plaid Shell!\n");
    while (1) {
        char *line = readline("#? ");
        
        // Exit on null (Ctrl-D)
        if (!line) {
            printf("\n");
            break;
        }
        // Skip empty lines
        if (strlen(line) == 0) {
            free(line);
            continue;
        }
        // Add to history
        add_history(line);
        // Tokenize
        char errmsg[256] = {0};
        CList tokens = TOK_tokenize_input(line, errmsg, sizeof(errmsg));
        
        if (!tokens) {
            if (strlen(errmsg) > 0) {
                fprintf(stderr, "Tokenization error: %s\n", errmsg);
            }
            free(line);
            continue;
        }
        // Parse
        Pipeline *pipeline = parse_tokens(tokens, errmsg, sizeof(errmsg));
        
        if (!pipeline) {
            if (strlen(errmsg) > 0) {
                fprintf(stderr, "Parsing error: %s\n", errmsg);
            }
            free_token_values(tokens);
            free(line);
            continue;
        }
        
        // Execute pipeline
        //int exit_status = execute_pipeline(pipeline);
        
        // Cleanup
        pipeline_free(pipeline);
        free_token_values(tokens);
        free(line);
    }
    return 0;
}