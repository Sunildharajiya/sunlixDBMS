#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "crud.h"

cJSON *reader(const char *filename)
{
    FILE *file;
    long file_size;
    char *buffer;
    size_t bytes_read;
    cJSON *json;

    file = fopen(filename, "r");

    if (file == NULL)
    {
        return NULL;
    }

    /*
     * Find the size of the file.
     */
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    if (file_size < 0)
    {
        fclose(file);
        return NULL;
    }

    /*
     * Allocate memory for file contents.
     */
    buffer = malloc((size_t)file_size + 1);

    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    /*
     * Read the entire file.
     */
    bytes_read = fread(
        buffer,
        1,
        (size_t)file_size,
        file
    );

    fclose(file);

    buffer[bytes_read] = '\0';

    /*
     * Parse JSON.
     */
    json = cJSON_Parse(buffer);

    free(buffer);

    return json;
}