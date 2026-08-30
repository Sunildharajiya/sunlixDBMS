#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include <utility.h>

#define DATA_PATH "data"

/*
 * Writes a new JSON record to a JSON array file.
 *
 * filename       : Name of the JSON file.
 * data           : JSON object provided by the CRUD/CLI layer.
 * record_length  : Length of the record.
 * index          : Index value used by the key generator.
 *
 * Returns:
 *   0  -> Success
 *  -1  -> Error
 */
int writer(
    const char *filename,
    const char *data,
    int record_length,
    int index
)
{
    char path[512];
    FILE *file = NULL;
    char *buffer = NULL;
    char *formatted_json = NULL;
    cJSON *record = NULL;
    cJSON *database = NULL;
    int result = -1;

    /*
     * Create the complete path.
     *
     * Example:
     * data/users.json
     */
    snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    /*
     * Parse the new record supplied by the caller.
     */
    record = cJSON_Parse(data);

    if (record == NULL)
    {
        fprintf(
            stderr,
            "writer: invalid JSON data\n"
        );

        goto cleanup;
    }

    /*
     * Every record must be a JSON object.
     */
    if (!cJSON_IsObject(record))
    {
        fprintf(
            stderr,
            "writer: data must be a JSON object\n"
        );

        goto cleanup;
    }

    /*
     * Generate a unique SunlixDBMS key for
     * the new record.
     */
    char *key = generate_key(
        record_length,
        index
    );

    if (key == NULL)
    {
        fprintf(
            stderr,
            "writer: unable to generate key\n"
        );

        goto cleanup;
    }

    /*
     * Add the generated key to the record.
     */
    cJSON_AddStringToObject(
        record,
        "key",
        key
    );

    free(key);

    /*
     * Try to open the existing JSON file.
     */
    file = fopen(path, "r");

    if (file != NULL)
    {
        /*
         * Find the size of the existing file.
         */
        fseek(file, 0, SEEK_END);

        long size = ftell(file);

        if (size < 0)
        {
            fprintf(
                stderr,
                "writer: unable to determine file size\n"
            );

            goto cleanup;
        }

        rewind(file);

        /*
         * Allocate memory for the existing JSON.
         */
        buffer = malloc(
            (size_t)size + 1
        );

        if (buffer == NULL)
        {
            fprintf(
                stderr,
                "writer: memory allocation failed\n"
            );

            goto cleanup;
        }

        /*
         * Read the existing JSON file.
         */
        size_t read_size = fread(
            buffer,
            1,
            (size_t)size,
            file
        );

        fclose(file);
        file = NULL;

        buffer[read_size] = '\0';

        /*
         * Parse the existing database.
         */
        database = cJSON_Parse(buffer);

        if (database == NULL)
        {
            fprintf(
                stderr,
                "writer: existing file contains invalid JSON\n"
            );

            goto cleanup;
        }

        /*
         * The database must be a JSON array.
         */
        if (!cJSON_IsArray(database))
        {
            fprintf(
                stderr,
                "writer: existing data must be a JSON array\n"
            );

            goto cleanup;
        }
    }
    else
    {
        /*
         * The file does not exist.
         *
         * Create a new empty JSON array.
         */
        database = cJSON_CreateArray();

        if (database == NULL)
        {
            fprintf(
                stderr,
                "writer: unable to create JSON array\n"
            );

            goto cleanup;
        }
    }

    /*
     * Add the new record to the database array.
     *
     * cJSON_AddItemToArray() takes ownership
     * of the record object.
     */
    cJSON_AddItemToArray(
        database,
        record
    );

    /*
     * Convert the complete database array
     * back into formatted JSON.
     */
    formatted_json = cJSON_Print(database);

    if (formatted_json == NULL)
    {
        fprintf(
            stderr,
            "writer: unable to format JSON\n"
        );

        goto cleanup;
    }

    /*
     * Open the file in write mode.
     *
     * The complete updated array is written back.
     */
    file = fopen(path, "w");

    if (file == NULL)
    {
        perror(
            "writer: unable to open file"
        );

        goto cleanup;
    }

    /*
     * Write the updated database.
     */
    if (fprintf(
            file,
            "%s\n",
            formatted_json
        ) < 0)
    {
        fprintf(
            stderr,
            "writer: unable to write database\n"
        );

        goto cleanup;
    }

    /*
     * Check close operation.
     */
    if (fclose(file) != 0)
    {
        file = NULL;

        fprintf(
            stderr,
            "writer: unable to close file\n"
        );

        goto cleanup;
    }

    file = NULL;

    result = 0;

    printf(
        "Writing file: %s\n",
        path
    );

    /* =====================================================
     * CENTRALIZED CLEANUP
     * ===================================================== */

cleanup:

    /*
     * Close file if it is still open.
     */
    if (file != NULL)
    {
        fclose(file);
        file = NULL;
    }

    /*
     * Free file buffer.
     */
    if (buffer != NULL)
    {
        free(buffer);
        buffer = NULL;
    }

    /*
     * Free formatted JSON.
     */
    if (formatted_json != NULL)
    {
        free(formatted_json);
        formatted_json = NULL;
    }

    /*
     * Delete entire cJSON tree for record.
     * Note: record is owned by database after cJSON_AddItemToArray,
     * so only delete if we haven't added it yet (error path).
     */
    if (record != NULL && database == NULL)
    {
        cJSON_Delete(record);
        record = NULL;
    }

    /*
     * Delete entire cJSON tree for database.
     */
    if (database != NULL)
    {
        cJSON_Delete(database);
        database = NULL;
    }

    /*
     * Return final result.
     */
    return result;
}
