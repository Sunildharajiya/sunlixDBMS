#include <stdio.h>
#include <string.h>

#include "../crud/crud.h"

void cmd_update(char *input)
{
    char *filename;
    char *key;
    char *field;
    char *value;

    /*
     * Skip the "update " portion.
     */
    filename = strtok(input + 7, " ");

    if (filename == NULL)
    {
        printf(
            "Error: update requires a filename.\n"
        );

        return;
    }

    /*
     * Get the record ID/key.
     */
    key = strtok(NULL, " ");

    if (key == NULL)
    {
        printf(
            "Error: update requires a record key.\n"
        );

        return;
    }

    /*
     * Get the field to update.
     */
    field = strtok(NULL, " ");

    if (field == NULL)
    {
        printf(
            "Error: update requires a field name.\n"
        );

        return;
    }

    /*
     * Everything remaining is treated as the value.
     *
     * This allows values containing spaces.
     */
    value = strtok(NULL, "");

    if (value == NULL || value[0] == '\0')
    {
        printf(
            "Error: update requires a value.\n"
        );

        return;
    }

    /*
     * Pass the parsed request to the CRUD layer.
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
        printf(
            "Data updated successfully.\n"
        );
    }
    else
    {
        printf(
            "Failed to update data.\n"
        );
    }
}