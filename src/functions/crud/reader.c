#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "crud.h"

#define DATA_PATH "data"

cJSON *reader(const char *filename)
{
    char path[512];

    FILE *file = NULL;

    long file_size;
    size_t bytes_read;

    char *buffer = NULL;
    cJSON *json = NULL;

    /*
     * Validate filename.
     */
    if (filename == NULL)
    {
        fprintf(
            stderr,
            "reader: invalid filename\n"
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
            "reader: file path is too long\n"
        );

        goto cleanup;
    }

    /*
     * Open the data file.
     */
    file = fopen(
        path,
        "r"
    );

    if (file == NULL)
    {
        perror(
            "reader: unable to open file"
        );

        goto cleanup;
    }

    /*
     * Move to the end of the file.
     */
    if (fseek(
            file,
            0,
            SEEK_END
        ) != 0)
    {
        fprintf(
            stderr,
            "reader: unable to seek file\n"
        );

        goto cleanup;
    }

    /*
     * Get file size.
     */
    file_size = ftell(file);

    if (file_size < 0)
    {
        fprintf(
            stderr,
            "reader: unable to determine file size\n"
        );

        goto cleanup;
    }

    /*
     * Return to beginning.
     */
    rewind(file);

    /*
     * Allocate memory for file contents.
     */
    buffer = malloc(
        (size_t)file_size + 1
    );

    if (buffer == NULL)
    {
        fprintf(
            stderr,
            "reader: memory allocation failed\n"
        );

        goto cleanup;
    }

    /*
     * Read the complete file.
     */
    bytes_read = fread(
        buffer,
        1,
        (size_t)file_size,
        file
    );

    /*
     * Check for actual read error.
     */
    if (ferror(file))
    {
        fprintf(
            stderr,
            "reader: unable to read file\n"
        );

        goto cleanup;
    }

    /*
     * Null terminate the buffer.
     */
    buffer[bytes_read] = '\0';

    /*
     * Close file.
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
            "reader: invalid JSON\n"
        );

        goto cleanup;
    }

    /*
     * The database subfile must contain
     * a JSON array.
     */
    if (!cJSON_IsArray(json))
    {
        fprintf(
            stderr,
            "reader: database is not a JSON array\n"
        );

        cJSON_Delete(json);
        json = NULL;

        goto cleanup;
    }

cleanup:

    if (file != NULL)
    {
        fclose(file);
    }

    free(buffer);

    return json;
}