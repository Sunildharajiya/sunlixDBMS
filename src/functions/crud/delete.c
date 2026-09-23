#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include "crud.h"

#define DATA_PATH "data"

/*
 * Soft-deletes a JSON record from a SunlixDBMS subfile.
 *
 * filename : Physical subfile name.
 * key      : Unique record key.
 *
 * Returns:
 *   0  -> Success
 *  -1  -> Error
 */
int deleter(
    const char *filename,
    const char *key
)
{
    char path[512];
    FILE *file;
    long file_size;
    char *buffer;
    size_t bytes_read;
    cJSON *json;
    cJSON *record;
    cJSON *record_key;
    cJSON *delete_field;

    if (filename == NULL || key == NULL)
    {
        fprintf(
            stderr,
            "deleter: invalid argument\n"
        );

        return -1;
    }

    /*
     * Build the physical subfile path.
     *
     * Example:
     * data/users_001.json
     */
    if (snprintf(
            path,
            sizeof(path),
            "%s/%s",
            DATA_PATH,
            filename
        ) >= (int)sizeof(path))
    {
        fprintf(
            stderr,
            "deleter: path too long\n"
        );

        return -1;
    }

    /*
     * Open the physical subfile.
     */
    file = fopen(path, "r");

    if (file == NULL)
    {
        fprintf(
            stderr,
            "deleter: file not found: %s\n",
            path
        );

        return -1;
    }

    /*
     * Determine the file size.
     */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);

        fprintf(
            stderr,
            "deleter: unable to seek file\n"
        );

        return -1;
    }

    file_size = ftell(file);

    if (file_size < 0)
    {
        fclose(file);

        fprintf(
            stderr,
            "deleter: unable to determine file size\n"
        );

        return -1;
    }

    rewind(file);

    /*
     * Allocate memory for the complete JSON file.
     */
    buffer = malloc((size_t)file_size + 1);

    if (buffer == NULL)
    {
        fclose(file);

        fprintf(
            stderr,
            "deleter: memory allocation failed\n"
        );

        return -1;
    }

    /*
     * Read the complete subfile.
     */
    bytes_read = fread(
        buffer,
        1,
        (size_t)file_size,
        file
    );

    if (ferror(file))
    {
        free(buffer);
        fclose(file);

        fprintf(
            stderr,
            "deleter: failed to read file\n"
        );

        return -1;
    }

    fclose(file);

    buffer[bytes_read] = '\0';

    /*
     * Parse the JSON.
     */
    json = cJSON_Parse(buffer);

    free(buffer);

    if (json == NULL)
    {
        fprintf(
            stderr,
            "deleter: invalid JSON data\n"
        );

        return -1;
    }

    /*
     * Every physical SunlixDBMS data file contains
     * a JSON array of records.
     */
    if (!cJSON_IsArray(json))
    {
        fprintf(
            stderr,
            "deleter: data must be a JSON array\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Search for the record using its unique key.
     */
    cJSON_ArrayForEach(record, json)
    {
        if (!cJSON_IsObject(record))
        {
            continue;
        }

        record_key = cJSON_GetObjectItemCaseSensitive(
            record,
            "key"
        );

        if (!cJSON_IsString(record_key))
        {
            continue;
        }

        if (strcmp(record_key->valuestring, key) == 0)
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
            "deleter: record not found: %s\n",
            key
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Check whether the record is already deleted.
     */
    delete_field = cJSON_GetObjectItemCaseSensitive(
        record,
        "delete"
    );

    if (cJSON_IsTrue(delete_field))
    {
        fprintf(
            stderr,
            "deleter: record already deleted: %s\n",
            key
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Mark the record as deleted.
     *
     * The record itself remains inside the database.
     */
    if (!cJSON_ReplaceItemInObject(
            record,
            "delete",
            cJSON_CreateTrue()
        ))
    {
        /*
         * If the delete field does not already exist,
         * add it instead.
         */
        if (!cJSON_AddBoolToObject(
                record,
                "delete",
                1
            ))
        {
            fprintf(
                stderr,
                "deleter: unable to mark record as deleted\n"
            );

            cJSON_Delete(json);

            return -1;
        }
    }

    /*
     * Convert the updated database back to JSON.
     */
    char *output = cJSON_Print(json);

    if (output == NULL)
    {
        fprintf(
            stderr,
            "deleter: unable to serialize JSON\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Reopen the same physical subfile for writing.
     */
    file = fopen(path, "w");

    if (file == NULL)
    {
        fprintf(
            stderr,
            "deleter: unable to open file for writing: %s\n",
            path
        );

        free(output);
        cJSON_Delete(json);

        return -1;
    }

    /*
     * Write the updated JSON.
     */
    if (fputs(output, file) == EOF)
    {
        fprintf(
            stderr,
            "deleter: failed to write updated data\n"
        );

        fclose(file);
        free(output);
        cJSON_Delete(json);

        return -1;
    }

    if (fclose(file) != 0)
    {
        fprintf(
            stderr,
            "deleter: failed to close file\n"
        );

        free(output);
        cJSON_Delete(json);

        return -1;
    }

    free(output);
    cJSON_Delete(json);

    /*
     * The physical record has been soft-deleted.
     *
     * Deleted metadata will be handled separately
     * by the metadataHandler.
     */
    printf(
        "Deleted record: %s\n",
        key
    );

    return 0;
}