#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 64

// Function declarations for built-in commands
void cmd_help(void);
void cmd_greet(char *name);
void cmd_add(char *num1_str, char *num2_str);

// Reusable CLI engine function
void cli(void) {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];
    int arg_count;

    printf("Welcome to the Custom C CLI! Type 'help' for commands.\n");

    // Start the infinite interactive loop
    while (1) {
        printf("my_cli> ");
        fflush(stdout);

        // Read user input safely
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }

        // Remove trailing newline character (\n)
        input[strcspn(input, "\n")] = '\0';

        // Tokenize the input string by spaces
        arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARG_SIZE) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }

        // Skip execution if user just hits enter
        if (arg_count == 0) {
            continue;
        }

        // Evaluate the commands
        if (strcmp(args[0], "exit") == 0) {
            printf("Exiting program. Goodbye!\n");
            break;
        } 
        else if (strcmp(args[0], "help") == 0) {
            cmd_help();
        } 
        else if (strcmp(args[0], "") == 0) {
            if (arg_count < 2) {
                printf("Error: 'greet' requires a name parameter. Example: greet Alice\n");
            } else {
                cmd_greet(args[1]);
            }
        } 
        else if (strcmp(args[0], "add") == 0) {
            if (arg_count < 3) {
                printf("Error: 'add' requires two numbers. Example: add 5 10\n");
            } else {
                cmd_add(args[1], args[2]);
            }
        } 
        else {
            printf("Unknown command: '%s'. Type 'help' for options.\n", args[0]);
        }
    }
}

// Built-in command logic implementations
void cmd_help(void) {
    printf("\nAvailable Commands:\n");
    printf("  help                - Display this menu\n");
    printf("  greet [name]        - Greet the user by name\n");
    printf("  add [num1] [num2]   - Sum two integers together\n");
    printf("  exit                - Safely quit the application\n\n");
}

void cmd_greet(char *name) {
    printf("Hello, %s! Welcome to this custom execution environment.\n", name);
}

void cmd_add(char *num1_str, char *num2_str) {
    int val1 = atoi(num1_str);
    int val2 = atoi(num2_str);
    printf("Result: %d\n", val1 + val2);
}
