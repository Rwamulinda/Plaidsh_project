#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

// Include the header file for your tokenize function
#include "Tokenize.h"

int main()
{
    printf("Welcome to Plaid shell \n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        // Display the prompt with bold red color (optional)
        char *prompt = "\033[1;31m#? \033[0m"; // Bold red color for prompt
        char *input = readline(prompt);

        if (!input) // EOF (Ctrl+D) handling
        {
            printf("\nExiting. Goodbye!\n");
            break;
        }

        // Check for "exit" command
        if (strcmp(input, "exit") == 0)
        {
            free(input);
            break;
        }

        // If input is not empty, add it to history
        if (*input)
        {
            add_history(input);
        }

        // Call the tokenize function to process the input
        char **tokens = tokenize(input); // Assuming tokenize returns an array of strings
        if (tokens)
        {
            printf("Tokens:\n");
            for (int i = 0; tokens[i] != NULL; i++)
            {
                printf("  [%d]: %s\n", i, tokens[i]);
            }
            free_tokens(tokens); // Assuming you have a function to free the tokens
        }

        free(input); // Free memory allocated by readline
    }

    return 0;
}
