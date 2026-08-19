#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include <key_genrator.h>

#define DATA_PATH "data"

/*
 * Writes JSON data to a file and automatically
 * generates a unique key for the record.
 *
 * filename       : Name of the JSON file.
 * data           : JSON data provided by the CRUD/CLI layer.
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
     * Create the complete file path.
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
     * Parse the user-provided data.
     *
     * cJSON_Parse() converts the JSON text
     * into a cJSON object that can be modified.
     *
     * It returns NULL when the supplied
     * data is not valid JSON.
     */
    cJSON *json = cJSON_Parse(data);

    if (json == NULL)
    {
        fprintf(stderr, "writer: invalid JSON data\n");
        return -1;
    }

    /*
     * SunlixDBMS records are currently required
     * to be JSON objects.
     *
     * Arrays, strings, numbers, etc. are rejected.
     */
    if (!cJSON_IsObject(json))
    {
        fprintf(
            stderr,
            "writer: data must be a JSON object\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Generate the SunlixDBMS record key.
     *
     * Example:
     *
     * 26A0819T1917@0257
     *
     * The key contains the timestamp,
     * record length and index value.
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

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Add the generated key to the JSON object.
     */
    cJSON_AddStringToObject(
        json,
        "key",
        key
    );

    /*
     * The key was allocated by generate_key().
     * It is no longer needed after cJSON has
     * copied its value into the JSON object.
     */
    free(key);

    /*
     * Convert the modified cJSON object back
     * into formatted JSON text.
     */
    char *formatted_json = cJSON_Print(json);

    if (formatted_json == NULL)
    {
        fprintf(
            stderr,
            "writer: unable to format JSON\n"
        );

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Open the file in write mode.
     *
     * "w" creates the file if it does not exist
     * and replaces the existing contents if it does.
     */
    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        perror(
            "writer: unable to open file"
        );

        free(formatted_json);
        cJSON_Delete(json);

        return -1;
    }

    /*
     * Write the formatted JSON to the file.
     */
    fprintf(
        file,
        "%s\n",
        formatted_json
    );

    /*
     * Display the path of the file being written.
     */
    printf(
        "Writing file: %s\n",
        path
    );

    /*
     * Close the file after writing.
     */
    fclose(file);

    /*
     * Free the formatted JSON string.
     */
    free(formatted_json);

    /*
     * Free the cJSON object and all of
     * its internally allocated memory.
     */
    cJSON_Delete(json);

    return 0;
}