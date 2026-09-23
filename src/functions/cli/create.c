#include <stdio.h>
#include <string.h>

#include "../crud/crud.h"

void cmd_create(char *input)
{
    char *filename;
    char *data;

    int record_length;
    int index;

    /*
     * This variable is used to read the
     * existing JSON data and determine
     * the next record index.
     */
    cJSON *root;

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
     * Calculate the length of the entered data.
     */
    record_length = strlen(data);

    /*
     * Read the existing JSON data to
     * calculate the next index.
     */
    root = reader(filename);

    if (root == NULL)
    {
        index = 0;
    }
    else
    {
        index = cJSON_GetArraySize(root);

        cJSON_Delete(root);
    }

    /*
     * Pass the JSON record to writer().
     */
    if (writer(filename, data, record_length, index) == 0)
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