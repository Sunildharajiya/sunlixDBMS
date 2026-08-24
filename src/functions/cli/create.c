#include <stdio.h>
#include <string.h>

#include "../crud/writer.h"

void cmd_create(char *input)
{
    char *filename;
    char *data;

    /*
     * Skip the "create " portion of the command.
     */
    filename = strtok(input + 7, " ");

    if (filename == NULL)
    {
        printf(
            "Error: create requires a filename.\n"
        );

        printf(
            "Example: create users.json "
            "{\"name\":\"Sunil\"}\n"
        );

        return;
    }

    /*
     * Get the remaining input as JSON.
     *
     * JSON may contain spaces, so we don't
     * tokenize it further.
     */
    data = strtok(NULL, "");

    if (data == NULL || data[0] == '\0')
    {
        printf(
            "Error: create requires JSON data.\n"
        );

        return;
    }

    /*
     * Pass the JSON record to writer().
     */
    if (writer(filename, data) == 0)
    {
        printf(
            "Data written successfully.\n"
        );
    }
    else
    {
        printf(
            "Failed to write data.\n"
        );
    }
}