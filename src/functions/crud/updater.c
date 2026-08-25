#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include "crud.h"

#define DATA_PATH "data"

/*
 * Update an existing field or add a new field
 * inside a record identified by its unique key.
 *
 * Returns:
 *   0  -> Success
 *  -1  -> Error
 */
int updater(
    const char *filename,
    const char *key,
    const char *field,
    const char *value
)
{
    char path[512];
    char *buffer;
    char *formatted_json;
    long size;
    size_t read_size;

    /*
     * Build the complete file path.
     */
    snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    /*
     * Open the database file.
     */
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("updater: unable to open file");
        return -1;
    }

    /*
     * Get the file size.
     */
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    if (size < 0)
    {
        fclose(file);
        return -1;
    }

    /*
     * Allocate memory for the file contents.
     */
    buffer = malloc((size_t)size + 1);

    if (buffer == NULL)
    {
        fclose(file);
        return -1;
    }

    /*
     * Read the complete JSON file.
     */
    read_size = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    fclose(file);

    buffer[read_size] = '\0';

    /*
     * Parse the JSON data.
     */
    cJSON *json = cJSON_Parse(buffer);

    free(buffer);

    if (json == NULL)
    {
        fprintf(
            stderr,
            "updater: invalid JSON\n"
        );

        return -1;
    }

    /*
     * The database must contain an array
     * of JSON records.
     */
    if (!cJSON_IsArray(json))
    {
        fprintf(
            stderr,
            "updater: database is not a JSON array\n"
        );

        cJSON_Delete(json);
        return -1;
    }

   /*
 * Search every record for the requested key.
 */
cJSON *record = NULL;
cJSON *stored_key = NULL;

int record_count = cJSON_GetArraySize(json);

for (int i = 0; i < record_count; i++)
{
    record = cJSON_GetArrayItem(json, i);

    if (record == NULL)
        continue;

    stored_key = cJSON_GetObjectItemCaseSensitive(
        record,
        "key"
    );

    if (cJSON_IsString(stored_key) &&
        strcmp(stored_key->valuestring, key) == 0)
    {
        break;
    }

    record = NULL;
}

/*
 * Record was not found.
 */
if (record == NULL)
{
    fprintf(
        stderr,
        "updater: record key not found\n"
    );

    cJSON_Delete(json);
    return -1;
}

    /*
     * Find the requested field inside
     * the matching record.
     */
    cJSON *old_value = cJSON_GetObjectItemCaseSensitive(
        record,
        field
    );

    /*
     * Update the existing field.
     */
    if (old_value != NULL)
    {
        /*
         * Currently the updater accepts
         * string values.
         */
        if (!cJSON_IsString(old_value))
        {
            fprintf(
                stderr,
                "updater: existing field is not a string\n"
            );

            cJSON_Delete(json);
            return -1;
        }

        if (!cJSON_SetValuestring(
                old_value,
                value
            ))
        {
            fprintf(
                stderr,
                "updater: unable to update field\n"
            );

            cJSON_Delete(json);
            return -1;
        }
    }
    /*
     * Add the field if it doesn't exist.
     */
    else
    {
        if (!cJSON_AddStringToObject(
                record,
                field,
                value
            ))
        {
            fprintf(
                stderr,
                "updater: unable to add field\n"
            );

            cJSON_Delete(json);
            return -1;
        }
    }

    /*
     * Convert the modified JSON tree
     * back into formatted JSON.
     */
    formatted_json = cJSON_Print(json);

    if (formatted_json == NULL)
    {
        fprintf(
            stderr,
            "updater: unable to format JSON\n"
        );

        cJSON_Delete(json);
        return -1;
    }

    /*
     * Replace the old database file
     * with the updated JSON.
     */
    file = fopen(path, "w");

    if (file == NULL)
    {
        perror(
            "updater: unable to open file for writing"
        );

        free(formatted_json);
        cJSON_Delete(json);

        return -1;
    }

    /*
     * Write the updated database.
     */
    fprintf(
        file,
        "%s\n",
        formatted_json
    );

    fclose(file);

    printf(
        "Updated file: %s\n",
        path
    );

    /*
     * Release allocated memory.
     */
    free(formatted_json);
    cJSON_Delete(json);

    return 0;
}