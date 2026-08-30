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
 * Parameters:
 *   filename -> JSON database filename
 *   key      -> unique key of the record
 *   field    -> field to update/add
 *   value    -> new string value
 *
 * Returns:
 *    0 -> Success
 *   -1 -> Error
 */
int updater(
    const char *filename,
    const char *key,
    const char *field,
    const char *value
)
{
    /*
     * File path.
     */
    char path[512];

    /*
     * File handle.
     */
    FILE *file = NULL;

    /*
     * Dynamically allocated memory.
     */
    char *buffer = NULL;
    char *formatted_json = NULL;

    /*
     * cJSON objects.
     */
    cJSON *json = NULL;
    cJSON *record = NULL;
    cJSON *stored_key = NULL;
    cJSON *old_value = NULL;

    /*
     * File information.
     */
    long size;
    size_t read_size;

    /*
     * Default result is failure.
     *
     * It will only become 0 when the
     * complete update succeeds.
     */
    int result = -1;


    /* -------------------------------------------------
     * 1. Validate arguments
     * ------------------------------------------------- */

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


    /* -------------------------------------------------
     * 2. Build database file path
     * ------------------------------------------------- */

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


    /* -------------------------------------------------
     * 3. Open database file for reading
     * ------------------------------------------------- */

    file = fopen(path, "r");

    if (file == NULL)
    {
        perror(
            "updater: unable to open file"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 4. Move to end of file
     * ------------------------------------------------- */

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(
            stderr,
            "updater: unable to seek file\n"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 5. Get file size
     * ------------------------------------------------- */

    size = ftell(file);

    if (size < 0)
    {
        fprintf(
            stderr,
            "updater: unable to determine file size\n"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 6. Return to beginning of file
     * ------------------------------------------------- */

    rewind(file);


    /* -------------------------------------------------
     * 7. Allocate memory for file contents
     * ------------------------------------------------- */

    buffer = malloc((size_t)size + 1);

    if (buffer == NULL)
    {
        fprintf(
            stderr,
            "updater: memory allocation failed\n"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 8. Read complete JSON file
     * ------------------------------------------------- */

    read_size = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    /*
     * Check whether fread() encountered
     * an actual file error.
     */
    if (ferror(file))
    {
        fprintf(
            stderr,
            "updater: unable to read file\n"
        );

        goto cleanup;
    }

    /*
     * Always terminate the string.
     */
    buffer[read_size] = '\0';


    /* -------------------------------------------------
     * 9. File is no longer required
     * ------------------------------------------------- */

    fclose(file);
    file = NULL;


    /* -------------------------------------------------
     * 10. Parse JSON
     * ------------------------------------------------- */

    json = cJSON_Parse(buffer);

    if (json == NULL)
    {
        fprintf(
            stderr,
            "updater: invalid JSON\n"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 11. Buffer is no longer required
     * ------------------------------------------------- */

    free(buffer);
    buffer = NULL;


    /* -------------------------------------------------
     * 12. Verify database structure
     * ------------------------------------------------- */

    if (!cJSON_IsArray(json))
    {
        fprintf(
            stderr,
            "updater: database is not a JSON array\n"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 13. Search record by unique key
     * ------------------------------------------------- */

    int record_count = cJSON_GetArraySize(json);

    for (int i = 0; i < record_count; i++)
    {
        cJSON *current_record =
            cJSON_GetArrayItem(json, i);

        /*
         * Ignore invalid array elements.
         */
        if (current_record == NULL ||
            !cJSON_IsObject(current_record))
        {
            continue;
        }

        /*
         * Get "key" from current record.
         */
        stored_key =
            cJSON_GetObjectItemCaseSensitive(
                current_record,
                "key"
            );

        /*
         * Check whether the key matches.
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


    /* -------------------------------------------------
     * 14. Record not found
     * ------------------------------------------------- */

    if (record == NULL)
    {
        fprintf(
            stderr,
            "updater: record key not found: %s\n",
            key
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 15. Find requested field
     * ------------------------------------------------- */

    old_value =
        cJSON_GetObjectItemCaseSensitive(
            record,
            field
        );


    /* -------------------------------------------------
     * 16. Update existing field
     * ------------------------------------------------- */

    if (old_value != NULL)
    {
        /*
         * This version of updater supports
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
         * Replace old string value.
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


    /* -------------------------------------------------
     * 17. Add field if it doesn't exist
     * ------------------------------------------------- */

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


    /* -------------------------------------------------
     * 18. Convert JSON tree back to text
     * ------------------------------------------------- */

    formatted_json = cJSON_Print(json);

    if (formatted_json == NULL)
    {
        fprintf(
            stderr,
            "updater: unable to format JSON\n"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 19. Open database for writing
     * ------------------------------------------------- */

    file = fopen(path, "w");

    if (file == NULL)
    {
        perror(
            "updater: unable to open file for writing"
        );

        goto cleanup;
    }


    /* -------------------------------------------------
     * 20. Write updated JSON
     * ------------------------------------------------- */

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


    /* -------------------------------------------------
     * 21. Check close operation
     * ------------------------------------------------- */

    if (fclose(file) != 0)
    {
        /*
         * fclose() may report errors such as
         * buffered write failures.
         */
        file = NULL;

        fprintf(
            stderr,
            "updater: unable to close database file\n"
        );

        goto cleanup;
    }

    file = NULL;


    /* -------------------------------------------------
     * 22. Operation successful
     * ------------------------------------------------- */

    result = 0;

    printf(
        "Updated file: %s\n",
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
     * Delete entire cJSON tree.
     */
    if (json != NULL)
    {
        cJSON_Delete(json);
        json = NULL;
    }

    /*
     * Return final result.
     */
    return result;
}