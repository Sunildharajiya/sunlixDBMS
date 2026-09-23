#include <stdio.h>
#include <string.h>

#include "../crud/crud.h"

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
            "Example: create users_001.json "
            "{\"name\":\"Sunil\"}\n"
        );

        return;
    }

    /*
     * Get the remaining input as JSON.
     *
     * JSON may contain spaces, so it is not
     * tokenized further.
     */
    data = strtok(NULL, "");

    if (data == NULL || data[0] == '\0')
    {
        printf(
            "Error: create requires JSON data.\n"
        );

        printf(
            "Example: create users_001.json "
            "{\"name\":\"Sunil\"}\n"
        );

        return;
    }

    /*
     * Writer now handles:
     *
     * - Global record index
     * - Subfile ID
     * - Record key generation
     * - Delete flag
     * - Subfile creation/update
     * - Metadata updates
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