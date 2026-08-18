#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#define DATA_PATH "data"

/*
 * Writes user-defined JSON data to a file.
 *
 * filename : Name of the JSON file.
 * data     : JSON data provided by the CRUD/CLI layer.
 *
 * Returns:
 *   0  -> Success
 *  -1  -> Error
 */
int writer(const char *filename, const char *data)
{
    char path[512];

    /*
     * Create the complete path:
     * ../../data/<filename>
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
     * cJSON_Parse() returns NULL if the
     * supplied data is not valid JSON.
     */
    cJSON *json = cJSON_Parse(data);

    if (json == NULL)
    {
        fprintf(stderr, "writer: invalid JSON data\n");
        return -1;
    }

    /*
     * Convert the parsed JSON back into
     * formatted JSON text.
     *
     * This ensures that the data written
     * to the file is proper JSON.
     */
    char *formatted_json = cJSON_Print(json);

    if (formatted_json == NULL)
    {
        fprintf(stderr, "writer: unable to format JSON\n");

        cJSON_Delete(json);

        return -1;
    }

    /*
     * Open the file in write mode.
     *
     * "w" creates the file if it doesn't exist
     * and replaces its contents if it already exists.
     */
    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        perror("writer: unable to open file");

        free(formatted_json);
        cJSON_Delete(json);

        return -1;
    }

    /*
     * Write the formatted JSON to the file.
     */
    fprintf(file, "%s\n", formatted_json);
    printf("Writing file: %s\n", path);
    /*
     * Close the file after writing.
     */
    fclose(file);

    /*
     * Free memory allocated by cJSON.
     */
    free(formatted_json);
    cJSON_Delete(json);

    return 0;
}