#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include <utility.h>
#include <metadataHandler/metadata.h>

#define DATA_PATH "data"

static int parse_filename(
    const char *filename,
    char *data_loby,
    size_t data_loby_size,
    int *file_id
)
{
    char name[512];

    if (filename == NULL ||
        data_loby == NULL ||
        file_id == NULL)
    {
        return -1;
    }

    if (snprintf(
            name,
            sizeof(name),
            "%s",
            filename
        ) >= (int)sizeof(name))
    {
        return -1;
    }

    char *extension = strrchr(name, '.');

    if (extension == NULL ||
        strcmp(extension, ".json") != 0)
    {
        return -1;
    }

    *extension = '\0';

    char *separator = strrchr(name, '_');

    if (separator == NULL ||
        separator == name ||
        *(separator + 1) == '\0')
    {
        return -1;
    }

    *separator = '\0';

    char *end = NULL;

    long parsed_id = strtol(
        separator + 1,
        &end,
        10
    );

    if (*end != '\0' ||
        parsed_id < 1 ||
        parsed_id > 999)
    {
        return -1;
    }

    if (strlen(name) >= data_loby_size)
    {
        return -1;
    }

    strcpy(data_loby, name);

    *file_id = (int)parsed_id;

    return 0;
}

int writer(
    const char *filename,
    const char *data
)
{
    char path[512];
    char data_loby[256];

    FILE *file = NULL;

    char *buffer = NULL;
    char *formatted_json = NULL;
    char *key = NULL;

    cJSON *record = NULL;
    cJSON *database = NULL;
    cJSON *metadata = NULL;

    int file_id;
    int index;

    int result = -1;

    /*
     * Check arguments.
     */
    if (filename == NULL ||
        data == NULL)
    {
        fprintf(
            stderr,
            "writer: invalid argument\n"
        );

        goto cleanup;
    }

    /*
     * Extract data lobby and subfile ID.
     *
     * Example:
     *
     * users_001.json
     *
     * data_loby = users
     * file_id   = 1
     */
    if (parse_filename(
            filename,
            data_loby,
            sizeof(data_loby),
            &file_id
        ) != 0)
    {
        fprintf(
            stderr,
            "writer: invalid subfile name: %s\n",
            filename
        );

        goto cleanup;
    }

    /*
     * Load metadata.
     *
     * Example:
     *
     * metadata/users.meta.json
     */
    metadata = metadata_load(data_loby);

    if (metadata == NULL)
    {
        fprintf(
            stderr,
            "writer: unable to load metadata\n"
        );

        goto cleanup;
    }

    /*
     * Get the next global record index.
     */
    index = metadata_get_next_index(
      data_loby
    );

    if (index < 1)
    {
        fprintf(
            stderr,
            "writer: invalid nextIndex\n"
        );

        goto cleanup;
    }

    /*
     * Create data file path.
     *
     * Example:
     *
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
            "writer: path too long\n"
        );

        goto cleanup;
    }

    /*
     * Parse the new record.
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
     * Every record must be an object.
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
     * Generate the unique record key.
     *
     * file_id = physical subfile ID
     * index   = global record index
     */
    key = generate_key(
        file_id,
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
     * Add generated key to record.
     */
    cJSON_AddStringToObject(
        record,
        "key",
        key
    );

    free(key);
    key = NULL;

    /*
     * New records are not deleted.
     */
    cJSON_AddBoolToObject(
        record,
        "delete",
        0
    );

    /*
     * Try to open existing subfile.
     */
    file = fopen(
        path,
        "r"
    );

    if (file != NULL)
    {
        /*
         * Find file size.
         */
        if (fseek(file, 0, SEEK_END) != 0)
        {
            fprintf(
                stderr,
                "writer: unable to seek file\n"
            );

            goto cleanup;
        }

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
         * Allocate buffer.
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
         * Read existing JSON.
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
         * Parse existing database.
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
         * Existing database must be an array.
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
         * Subfile does not exist.
         *
         * Create a new JSON array.
         *
         * IMPORTANT:
         * We do NOT touch or delete metadata here.
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
     * Add the new record to the database.
     *
     * database now owns record.
     */
    cJSON_AddItemToArray(
        database,
        record
    );

    record = NULL;

    /*
     * Convert database to formatted JSON.
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
     * Open data file for writing.
     */
    file = fopen(
        path,
        "w"
    );

    if (file == NULL)
    {
        perror(
            "writer: unable to open data file"
        );

        goto cleanup;
    }

    /*
     * Write updated database.
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
     * Close data file.
     */
    if (fclose(file) != 0)
    {
        file = NULL;

        fprintf(
            stderr,
            "writer: unable to close data file\n"
        );

        goto cleanup;
    }

    file = NULL;

    /*
     * ==================================================
     * DATA FILE SUCCESSFULLY WRITTEN
     *
     * NOW UPDATE METADATA
     * ==================================================
     */

    /*
     * Check whether this subfile already exists
     * in metadata.
     */
    if (metadata_find_subfile(
    data_loby,
    index
) == -1)
    {
        /*
         * First record in this subfile.
         *
         * Example:
         *
         * file_id = 1
         * start   = 1
         * end     = 1
         */
        if (metadata_add_subfile(
    data_loby,
    file_id,
    index,
    index
) != 0)
        {
            fprintf(
                stderr,
                "writer: unable to add subfile metadata\n"
            );

            goto cleanup;
        }
    }
    else
    {
        /*
         * Subfile already exists.
         *
         * Extend its record range.
         */
        if (metadata_update_subfile_end(
                metadata,
                file_id,
                index
            ) != 0)
        {
            fprintf(
                stderr,
                "writer: unable to update subfile metadata\n"
            );

            goto cleanup;
        }
    }

    /*
     * Increment global nextIndex.
     */
    if (metadata_set_next_index(
        metadata,
        index + 1
    ) != 0)
{
    fprintf(
        stderr,
        "writer: unable to update nextIndex\n"
    );

    goto cleanup;
}

    /*
     * Save metadata.
     *
     * This updates the metadata file.
     *
     * It does NOT delete the metadata file.
     */
    if (metadata_save(
            data_loby,
            metadata
        ) != 0)
    {
        fprintf(
            stderr,
            "writer: unable to save metadata\n"
        );

        goto cleanup;
    }

    printf(
        "Writing file: %s\n",
        path
    );

    printf(
        "Record index: %d\n",
        index
    );

    printf(
        "Subfile ID: %03d\n",
        file_id
    );

    /*
     * Success.
     */
    result = 0;

cleanup:

    if (file != NULL)
    {
        fclose(file);
    }

    free(buffer);
    free(formatted_json);
    free(key);

    if (record != NULL)
    {
        cJSON_Delete(record);
    }

    if (database != NULL)
    {
        cJSON_Delete(database);
    }

    if (metadata != NULL)
    {
        cJSON_Delete(metadata);
    }

    return result;
}