#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "Tokenize.h" // Include the tokenize header

int main() {
    printf(" Welocme to Plaid shell\n");
    //printf("Type 'exit' to quit.\n\n");

    while (1) {
        // Display the prompt with bold red color
        char *prompt = "\033[1;31m#? \033[0m";
        char *input = readline(prompt);

        if (!input) { // EOF (Ctrl+D) handling
            printf("\nExiting. Goodbye!\n");
            break;
        }

        // Check for "exit" command
        if (strcmp(input, "exit") == 0) {
            free(input);
            break;
        }

        // If input is not empty, add it to history
        if (*input) {
            add_history(input);
        }

        // Tokenize the input
        char errmsg[256];
        CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));

        if (!tokens) {
            // Handle tokenization error
            fprintf(stderr, "Tokenization error: %s\n", errmsg);
        } else {
            // Print the tokens for debugging
            printf("Tokens:\n");
            TOK_print(tokens);

            // Free the tokens
            CL_free(tokens);
        }

        free(input); // Free memory allocated by readline
    }

    return 0;
}
