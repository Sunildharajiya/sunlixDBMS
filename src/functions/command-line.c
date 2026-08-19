#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./crud/writer.h"
#include "./crud/updater.h"

#define MAX_INPUT_SIZE 4096
#define MAX_ARG_SIZE 64

/* Built-in command declarations */
void cmd_help(void);

/* Main CLI engine */
void cli(void)
{
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARG_SIZE];
    int arg_count;

    printf("Welcome to SunlixDBMS CLI!\n");
    printf("Type 'help' for available commands.\n");

    /* Start the interactive CLI loop */
    while (1)
    {
        printf("my_cli> ");
        fflush(stdout);

        /* Read a complete command from the user */
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        /* Remove the newline added by fgets() */
        input[strcspn(input, "\n")] = '\0';

        /* Ignore empty input */
        if (input[0] == '\0')
        {
            continue;
        }

        /*
         * Special handling for the create command.
         *
         * JSON data can contain spaces, so the JSON
         * portion is handled separately.
         */
        if (strncmp(input, "create ", 7) == 0)
        {
            char *filename;
            char *data;

            filename = strtok(input + 7, " ");

            if (filename == NULL)
            {
                printf("Error: create requires a filename.\n");
                printf(
                    "Example: create users.json "
                    "{\"name\":\"DBMS\"}\n"
                );
                continue;
            }

            data = strtok(NULL, "");

            if (data == NULL || data[0] == '\0')
            {
                printf("Error: create requires JSON data.\n");
                printf(
                    "Example: create users.json "
                    "{\"name\":\"Sunil\"}\n"
                );
                continue;
            }

            /*
             * The current writer requires record length
             * and index values for key generation.
             */
            if (writer(filename, data, 2, 1) == 0)
            {
                printf("Data written successfully.\n");
            }
            else
            {
                printf("Failed to write data.\n");
            }

            continue;
        }

        /*
         * Special handling for the update command.
         *
         * Format:
         *
         * update <file> <key> <field> <value>
         */
        if (strncmp(input, "update ", 7) == 0)
        {
            char *filename;
            char *key;
            char *field;
            char *value;

            filename = strtok(input + 7, " ");

            if (filename == NULL)
            {
                printf("Error: update requires a filename.\n");
                printf(
                    "Example: update users.json "
                    "<key> name Sunil\n"
                );
                continue;
            }

            key = strtok(NULL, " ");

            if (key == NULL)
            {
                printf("Error: update requires a record key.\n");
                continue;
            }

            field = strtok(NULL, " ");

            if (field == NULL)
            {
                printf("Error: update requires a field name.\n");
                continue;
            }

            /*
             * Read the remaining input as the value.
             *
             * This allows values containing spaces.
             */
            value = strtok(NULL, "");

            if (value == NULL || value[0] == '\0')
            {
                printf("Error: update requires a value.\n");
                continue;
            }

            /*
             * Send the update request to updater().
             */
            if (
                updater(
                    filename,
                    key,
                    field,
                    value
                ) == 0
            )
            {
                printf("Data updated successfully.\n");
            }
            else
            {
                printf("Failed to update data.\n");
            }

            continue;
        }

        /*
         * Tokenize normal CLI commands.
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

        /* Execute built-in commands */
        if (strcmp(args[0], "help") == 0)
        {
            cmd_help();
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            printf("Exiting program. Goodbye!\n");
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

/*
 * Display all currently supported CLI commands.
 */
void cmd_help(void)
{
    printf("\nCommands:\n");

    printf(
        "  create <file> <json>  Create data\n"
    );

    printf(
        "  read <file>           Read data\n"
    );

    printf(
        "  update <file> <key> <field> <value>  "
        "Update data\n"
    );

    printf(
        "  delete <file>         Delete data\n"
    );

    printf(
        "  help                  Show commands\n"
    );

    printf(
        "  exit                  Exit DBMS\n\n"
    );
}