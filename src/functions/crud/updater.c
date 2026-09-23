#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include "crud.h"

#define DATA_PATH "data"

int updater(
    const char *filename,
    const char *key,
    const char *field,
    const char *value
)
{
    char path[512];

    FILE *file = NULL;

    char *buffer = NULL;
    char *formatted_json = NULL;

    cJSON *json = NULL;
    cJSON *record = NULL;
    cJSON *stored_key = NULL;
    cJSON *old_value = NULL;
    cJSON *deleted = NULL;

    long size;
    size_t read_size;

    int result = -1;

    /*
     * Validate arguments.
     */
    if (filename == NULL ||
        key == NULL ||
        field == NULL ||
        value == NULL)
    {
        fprintf(
            stderr,
            "updater: invalid argument\n"
        );

        goto cleanup;
    }

    /*
     * Do not allow updater to modify
     * the generated key.
     */
    if (strcmp(field, "key") == 0)
    {
        fprintf(
            stderr,
            "updater: key field cannot be modified\n"
        );

        goto cleanup;
    }

    /*
     * Do not allow updater to modify
     * the deletion flag.
     *
     * Deletion must be handled by deleter().
     */
    if (strcmp(field, "delete") == 0)
    {
        fprintf(
            stderr,
            "updater: delete field cannot be modified\n"
        );

        goto cleanup;
    }

    /*
     * Build data file path.
     *
     * Example:
     *
     * data/users_001.json
     */
    int path_result = snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    if (path_result < 0 ||
        (size_t)path_result >= sizeof(path))
    {
        fprintf(
            stderr,
            "updater: file path is too long\n"
        );

        goto cleanup;
    }

    /*
     * Open data file.
     */
    file = fopen(
        path,
        "r"
    );

    if (file == NULL)
    {
        perror(
            "updater: unable to open file"
        );

        goto cleanup;
    }

    /*
     * Move to end of file.
     */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(
            stderr,
            "updater: unable to seek file\n"
        );

        goto cleanup;
    }

    /*
     * Get file size.
     */
    size = ftell(file);

    if (size < 0)
    {
        fprintf(
            stderr,
            "updater: unable to determine file size\n"
        );

        goto cleanup;
    }

    /*
     * Return to beginning.
     */
    rewind(file);

    /*
     * Allocate memory.
     */
    buffer = malloc(
        (size_t)size + 1
    );

    if (buffer == NULL)
    {
        fprintf(
            stderr,
            "updater: memory allocation failed\n"
        );

        goto cleanup;
    }

    /*
     * Read complete JSON file.
     */
    read_size = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    if (ferror(file))
    {
        fprintf(
            stderr,
            "updater: unable to read file\n"
        );

        goto cleanup;
    }

    buffer[read_size] = '\0';

    /*
     * File is no longer needed.
     */
    fclose(file);
    file = NULL;

    /*
     * Parse JSON.
     */
    json = cJSON_Parse(buffer);

    if (json == NULL)
    {
        fprintf(
            stderr,
            "updater: invalid JSON\n"
        );

        goto cleanup;
    }

    /*
     * Buffer is no longer needed.
     */
    free(buffer);
    buffer = NULL;

    /*
     * Database must be an array.
     */
    if (!cJSON_IsArray(json))
    {
        fprintf(
            stderr,
            "updater: database is not a JSON array\n"
        );

        goto cleanup;
    }

    /*
     * Search for record by key.
     */
    int record_count = cJSON_GetArraySize(json);

    for (int i = 0; i < record_count; i++)
    {
        cJSON *current_record =
            cJSON_GetArrayItem(
                json,
                i
            );

        /*
         * Ignore invalid array elements.
         */
        if (current_record == NULL ||
            !cJSON_IsObject(current_record))
        {
            continue;
        }

        /*
         * Get record key.
         */
        stored_key =
            cJSON_GetObjectItemCaseSensitive(
                current_record,
                "key"
            );

        /*
         * Compare keys.
         */
        if (cJSON_IsString(stored_key) &&
            stored_key->valuestring != NULL &&
            strcmp(
                stored_key->valuestring,
                key
            ) == 0)
        {
            record = current_record;
            break;
        }
    }

    /*
     * Record does not exist.
     */
    if (record == NULL)
    {
        fprintf(
            stderr,
            "updater: record key not found: %s\n",
            key
        );

        goto cleanup;
    }

    /*
     * Check soft-delete status.
     *
     * Deleted records must not be updated.
     */
    deleted =
        cJSON_GetObjectItemCaseSensitive(
            record,
            "delete"
        );

    if (cJSON_IsTrue(deleted))
    {
        fprintf(
            stderr,
            "updater: record is deleted: %s\n",
            key
        );

        goto cleanup;
    }

    /*
     * Find requested field.
     */
    old_value =
        cJSON_GetObjectItemCaseSensitive(
            record,
            field
        );

    /*
     * Update existing field.
     */
    if (old_value != NULL)
    {
        /*
         * Current updater supports
         * string values only.
         */
        if (!cJSON_IsString(old_value))
        {
            fprintf(
                stderr,
                "updater: existing field '%s' "
                "is not a string\n",
                field
            );

            goto cleanup;
        }

        /*
         * Replace old value.
         */
        if (cJSON_SetValuestring(
                old_value,
                value
            ) == NULL)
        {
            fprintf(
                stderr,
                "updater: unable to update field '%s'\n",
                field
            );

            goto cleanup;
        }
    }

    /*
     * Add new field.
     */
    else
    {
        if (cJSON_AddStringToObject(
                record,
                field,
                value
            ) == NULL)
        {
            fprintf(
                stderr,
                "updater: unable to add field '%s'\n",
                field
            );

            goto cleanup;
        }
    }

    /*
     * Convert JSON tree back to text.
     */
    formatted_json = cJSON_Print(json);

    if (formatted_json == NULL)
    {
        fprintf(
            stderr,
            "updater: unable to format JSON\n"
        );

        goto cleanup;
    }

    /*
     * Open database for writing.
     */
    file = fopen(
        path,
        "w"
    );

    if (file == NULL)
    {
        perror(
            "updater: unable to open file for writing"
        );

        goto cleanup;
    }

    /*
     * Write updated JSON.
     */
    if (fprintf(
            file,
            "%s\n",
            formatted_json
        ) < 0)
    {
        fprintf(
            stderr,
            "updater: unable to write database\n"
        );

        goto cleanup;
    }

    /*
     * Close database.
     */
    if (fclose(file) != 0)
    {
        file = NULL;

        fprintf(
            stderr,
            "updater: unable to close database file\n"
        );

        goto cleanup;
    }

    file = NULL;

    /*
     * Update successful.
     */
    result = 0;

    printf(
        "Updated record: %s\n",
        key
    );

    printf(
        "Updated file: %s\n",
        path
    );

cleanup:

    if (file != NULL)
    {
        fclose(file);
        file = NULL;
    }

    free(buffer);
    buffer = NULL;

    free(formatted_json);
    formatted_json = NULL;

    if (json != NULL)
    {
        cJSON_Delete(json);
        json = NULL;
    }

    return result;
}