#include <stdio.h>
#include <string.h>

#include "../crud/crud.h"

void cmd_delete(char *input)
{
    char *filename;
    char *key;

    /*
     * Skip the "delete " portion.
     */
    filename = strtok(input + 7, " ");

    if (filename == NULL)
    {
        printf(
            "Error: delete requires a filename.\n"
        );

        printf(
            "Example: delete users_001.json "
            "<record-key>\n"
        );

        return;
    }

    /*
     * Get the record key.
     */
    key = strtok(NULL, " ");

    if (key == NULL)
    {
        printf(
            "Error: delete requires a record key.\n"
        );

        printf(
            "Example: delete users_001.json "
            "<record-key>\n"
        );

        return;
    }

    /*
     * Pass the request to the CRUD layer.
     *
     * deleter() performs a soft delete by
     * setting the record's delete field to true.
     */
    if (
        deleter(
            filename,
            key
        ) == 0
    )
    {
        printf(
            "Record deleted successfully.\n"
        );
    }
    else
    {
        printf(
            "Failed to delete record.\n"
        );
    }
}