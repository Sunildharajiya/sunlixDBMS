#include <stdio.h>
#include <string.h>

#include "cli.h"
#include "commands.h"
#include "create.h"
#include "update.h"

#define MAX_INPUT_SIZE 4096
#define MAX_ARG_SIZE 64

void cli(void)
{
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];
    int arg_count;

    printf(
        "Welcome to SunlixDBMS CLI!\n"
    );

    printf(
        "Type 'help' for available commands.\n"
    );

    while (1)
    {
        printf("my_cli> ");
        fflush(stdout);

        /*
         * Read one complete command.
         */
        if (
            fgets(
                input,
                sizeof(input),
                stdin
            ) == NULL
        )
        {
            break;
        }

        /*
         * Remove the newline.
         */
        input[
            strcspn(
                input,
                "\n"
            )
        ] = '\0';

        /*
         * Ignore empty commands.
         */
        if (input[0] == '\0')
        {
            continue;
        }

        /*
         * Handle create separately because
         * JSON can contain spaces.
         */
        if (strncmp(input, "create ", 7) == 0)
        {
            cmd_create(input);
            continue;
        }

        /*
         * Handle update separately because
         * its value may contain spaces.
         */
        if (strncmp(input, "update ", 7) == 0)
        {
            cmd_update(input);
            continue;
        }

        /*
         * Tokenize normal commands.
         */
        arg_count = 0;

        char *token = strtok(input, " ");

        while (
            token != NULL &&
            arg_count < MAX_ARG_SIZE
        )
        {
            args[arg_count++] = token;

            token = strtok(NULL, " ");
        }

        /*
         * Make sure at least one argument exists.
         */
        if (arg_count == 0)
        {
            continue;
        }

        /*
         * Built-in commands.
         */
        if (strcmp(args[0], "help") == 0)
        {
            cmd_help();
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            printf(
                "Exiting program. Goodbye!\n"
            );

            break;
        }
        else
        {
            printf(
                "Unknown command: '%s'. "
                "Type 'help' for available commands.\n",
                args[0]
            );
        }
    }
}