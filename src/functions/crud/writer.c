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
    cJSON *record = cJSON_Parse(data);

    if (record == NULL)
    {
        fprintf(
            stderr,
            "writer: invalid JSON data\n"
        );

        return -1;
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

        cJSON_Delete(record);

        return -1;
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

        cJSON_Delete(record);

        return -1;
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
    FILE *file = fopen(path, "r");

    cJSON *database = NULL;

    if (file != NULL)
    {
        /*
         * Find the size of the existing file.
         */
        fseek(file, 0, SEEK_END);

        long size = ftell(file);

        if (size < 0)
        {
            fclose(file);
            cJSON_Delete(record);

            fprintf(
                stderr,
                "writer: unable to determine file size\n"
            );

            return -1;
        }

        rewind(file);

        /*
         * Allocate memory for the existing JSON.
         */
        char *buffer = malloc(
            (size_t)size + 1
        );

        if (buffer == NULL)
        {
            fclose(file);
            cJSON_Delete(record);

            fprintf(
                stderr,
                "writer: memory allocation failed\n"
            );

            return -1;
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

        buffer[read_size] = '\0';

        /*
         * Parse the existing database.
         */
        database = cJSON_Parse(buffer);

        free(buffer);

        if (database == NULL)
        {
            cJSON_Delete(record);

            fprintf(
                stderr,
                "writer: existing file contains invalid JSON\n"
            );

            return -1;
        }

        /*
         * The database must be a JSON array.
         */
        if (!cJSON_IsArray(database))
        {
            cJSON_Delete(database);
            cJSON_Delete(record);

            fprintf(
                stderr,
                "writer: existing data must be a JSON array\n"
            );

            return -1;
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
            cJSON_Delete(record);

            fprintf(
                stderr,
                "writer: unable to create JSON array\n"
            );

            return -1;
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
    char *formatted_json = cJSON_Print(database);

    if (formatted_json == NULL)
    {
        fprintf(
            stderr,
            "writer: unable to format JSON\n"
        );

        cJSON_Delete(database);

        return -1;
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

        free(formatted_json);
        cJSON_Delete(database);

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
        "Writing file: %s\n",
        path
    );

    /*
     * Free allocated memory.
     */
    free(formatted_json);
    cJSON_Delete(database);

    return 0;
}