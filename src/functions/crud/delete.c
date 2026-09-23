#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#define DATA_PATH "data"

/*
 * Deletes a JSON record file from the SunlixDBMS
 * data directory.
 *
 * filename : Name of the JSON file.
 *
 * Returns:
 *   0  -> Success
 *  -1  -> Error
 */
int deleter(const char *filename)
{
    char path[512];

    /*
     * Create the complete path to the JSON file.
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
     * Open the file in read mode first.
     *
     * This verifies that the record exists and
     * also allows us to validate its JSON data.
     */
    FILE *file = fopen(path, "r");

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
     * Move to the end of the file to determine
     * the number of bytes that need to be read.
     */
    fseek(file, 0, SEEK_END);

    long size = ftell(file);

    if (size < 0)
    {
        fclose(file);

        fprintf(
            stderr,
            "deleter: unable to determine file size\n"
        );

        return -1;
    }

    /*
     * Move back to the beginning of the file
     * before reading its contents.
     */
    rewind(file);

    /*
     * Allocate enough memory for the complete
     * JSON file plus the null terminator.
     */
    char *buffer = malloc((size_t)size + 1);

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
     * Read the complete JSON file into memory.
     */
    size_t read_size = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    fclose(file);

    /*
     * Add the null terminator so that cJSON_Parse()
     * can treat the buffer as a normal C string.
     */
    buffer[read_size] = '\0';

    /*
     * Parse the JSON data.
     *
     * This makes sure that the file contains valid
     * SunlixDBMS JSON before deleting it.
     */
    cJSON *json = cJSON_Parse(buffer);

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
     * The current SunlixDBMS writer stores every
     * record as a JSON object.
     */
    if (!cJSON_IsObject(json))
    {
        fprintf(
            stderr,
            "deleter: data must be a JSON object\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * The JSON object is no longer needed because
     * the actual delete operation removes the
     * corresponding record file.
     */
    cJSON_Delete(json);

    /*
     * Delete the JSON file from the data directory.
     */
    if (remove(path) != 0)
    {
        perror(
            "deleter: unable to delete file"
        );

        return -1;
    }

    /*
     * Inform the user that the record was deleted.
     */
    printf(
        "Deleted file: %s\n",
        path
    );

    return 0;
}
