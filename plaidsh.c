#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

#include "Tokenize.h"
#include "pipeline.h"
#include "parse.h"

// Builtin command implementations
int do_cd(int argc, char **argv) {
    if (argc == 1) {
        // Change to home directory if no argument
        const char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "cd: Could not determine home directory\n");
            return 1;
        }
        return chdir(home);
    } else {
        return chdir(argv[1]);
    }
}

int do_pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("pwd");
        return 1;
    }
}

void do_author() {
    printf("Uwase Pauline\n");
}

int execute_pipeline(Pipeline *pipeline) {
    int pipes[pipeline->num_commands - 1][2];
    pid_t child_pids[pipeline->num_commands];

    // Setup pipes between commands
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }

    for (int i = 0; i < pipeline->num_commands; i++) {
        child_pids[i] = fork();

        if (child_pids[i] == -1) {
            perror("fork");
            return 1;
        }

        if (child_pids[i] == 0) {
            // Child process
            if (i > 0) {
                // Not first command, redirect stdin from previous pipe
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(1);
                }
            } else if (pipeline->input_file != stdin) {
                // Redirect input from specified file
                if (dup2(fileno(pipeline->input_file), STDIN_FILENO) == -1) {
                    perror("dup2 input file");
                    exit(1);
                }
            }

            if (i < pipeline->num_commands - 1) {
                // Not last command, redirect stdout to next pipe
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    exit(1);
                }
            } else if (pipeline->output_file != stdout) {
                // Redirect output to specified file
                if (dup2(fileno(pipeline->output_file), STDOUT_FILENO) == -1) {
                    perror("dup2 output file");
                    exit(1);
                }
            }

            // Close all pipe file descriptors in child
            for (int j = 0; j < pipeline->num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command
            execvp(pipeline->commands[i].argv[0], pipeline->commands[i].argv);
            
            // If execvp fails, print a more specific error
            if (errno == ENOENT) {
                fprintf(stderr, "%s: Command not found\n", pipeline->commands[i].argv[0]);
            } else {
                perror(pipeline->commands[i].argv[0]);
            }
            exit(127);  // Standard exit code for command not found
        }
    }

    // Close pipes in parent
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Close input/output files
    if (pipeline->input_file != stdin) {
        fclose(pipeline->input_file);
    }
    if (pipeline->output_file != stdout) {
        fclose(pipeline->output_file);
    }

    // Wait for children
    int status;
    int exit_status = 0;
    for (int i = 0; i < pipeline->num_commands; i++) {
        waitpid(child_pids[i], &status, 0);

        if (WIFEXITED(status)) {
            int child_exit_status = WEXITSTATUS(status);
            if (child_exit_status != 0) {
                fprintf(stderr, "Child %d exited with status %d\n", 
                        child_pids[i], child_exit_status);
                exit_status = 2;
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "Child %d killed by signal %d\n", 
                    child_pids[i], WTERMSIG(status));
            exit_status = 2;
        }
    }

    return exit_status;
}

int main() {
    char *line;
    
    // Configure readline
    // rl_bind_textvar("editing-mode", "vi");
    // using_history();

    printf("Welcome to Plaid Shell!\n");

    while ((line = readline("#? ")) != NULL) {
        // Skip empty lines
        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        // Add to history
        add_history(line);

        // Tokenize input
        char errmsg[256] = {0};
        CList tokens = TOK_tokenize_input(line, errmsg, sizeof(errmsg));

        if (tokens == NULL) {
            if (strlen(errmsg) > 0) {
                fprintf(stderr, "Tokenization error: %s\n", errmsg);
            }
            free(line);
            continue;
        }

        // Special built-in commands first
        Token first_token = TOK_next(tokens);
        int  should_exit = 0;
        if (strcmp(first_token.value, "exit") == 0 || 
            strcmp(first_token.value, "quit") == 0) {
            should_exit = 1;
        }

        if (strcmp(first_token.value, "cd") == 0) {
            TOK_consume(tokens);
            Token dir_token = TOK_next(tokens);
            char *argv[2] = {"cd", dir_token.value};
            do_cd(dir_token.value ? 2 : 1, argv);
            free_token_values(tokens);
            free(line);
            continue;
        }

        if (strcmp(first_token.value, "pwd") == 0) {
            do_pwd();
            free_token_values(tokens);
            free(line);
            continue;
        }

        if (strcmp(first_token.value, "author") == 0) {
            do_author();
            free_token_values(tokens);
            free(line);
            continue;
        }

        // Parse and execute pipeline
        if (!should_exit) {
            Pipeline *pipeline = parse_tokens(tokens);
            if (pipeline) {
                execute_pipeline(pipeline);
                pipeline_free(pipeline);
        }
        }

        // Cleanup
        free_token_values(tokens);
        free(line);

        if (should_exit) {
            break;
        }
    }

    return 0;
}