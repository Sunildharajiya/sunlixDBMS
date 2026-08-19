#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include "updater.h"

#define DATA_PATH "data"


/*
 * Updates a field inside an existing JSON record.
 *
 * filename : Name of the JSON file.
 * key      : Unique key of the record.
 * field    : JSON field that needs to be updated.
 * value    : New value of the field.
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

    /*
     * Create the complete file path:
     *
     * data/<filename>
     */
    snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    /*
     * Open the existing JSON file
     * in read mode.
     */
    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        perror("updater: unable to open file");
        return -1;
    }

    /*
     * Find the size of the file.
     */
    fseek(file, 0, SEEK_END);

    long size = ftell(file);

    rewind(file);

    if (size < 0)
    {
        fclose(file);
        return -1;
    }

    /*
     * Allocate memory to store
     * the complete JSON file.
     *
     * One extra byte is reserved
     * for the null terminator.
     */
    char *buffer = malloc(size + 1);

    if (buffer == NULL)
    {
        fclose(file);
        return -1;
    }

    /*
     * Read the complete file into memory.
     */
    size_t read_size = fread(
        buffer,
        1,
        size,
        file
    );

    /*
     * Add the string terminator.
     */
    buffer[read_size] = '\0';

    fclose(file);

    /*
     * Parse the file contents as JSON.
     */
    cJSON *json = cJSON_Parse(buffer);

    /*
     * The file buffer is no longer needed
     * after parsing.
     */
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
     * Get the "key" field from the JSON record.
     *
     * The key is used to make sure that
     * we are updating the correct record.
     */
    cJSON *stored_key = cJSON_GetObjectItem(
        json,
        "key"
    );

    if (stored_key == NULL ||
        !cJSON_IsString(stored_key))
    {
        fprintf(
            stderr,
            "updater: key not found\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Compare the supplied key with
     * the key stored in the JSON record.
     */
    if (strcmp(
            stored_key->valuestring,
            key
        ) != 0)
    {
        fprintf(
            stderr,
            "updater: record key does not match\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Find the field that needs to be updated.
     */
    cJSON *old_value = cJSON_GetObjectItem(
        json,
        field
    );

    /*
     * If the field already exists,
     * replace its value.
     */
    if (old_value != NULL)
    {
        cJSON_SetValuestring(
            old_value,
            value
        );
    }
    /*
     * If the field does not exist,
     * create a new string field.
     */
    else
    {
        cJSON_AddStringToObject(
            json,
            field,
            value
        );
    }

    /*
     * Convert the modified JSON object
     * back into formatted JSON text.
     */
    char *formatted_json = cJSON_Print(json);

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
     * Open the file in write mode.
     *
     * This replaces the old contents
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
     * Write the updated JSON to the file.
     */
    fprintf(
        file,
        "%s\n",
        formatted_json
    );

    /*
     * Close the file after writing.
     */
    fclose(file);

    printf(
        "Updated file: %s\n",
        path
    );

    /*
     * Free the memory allocated by cJSON_Print().
     */
    free(formatted_json);

    /*
     * Delete the parsed JSON tree.
     */
    cJSON_Delete(json);

    return 0;
}