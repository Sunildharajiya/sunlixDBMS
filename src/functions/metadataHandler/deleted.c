#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cJSON.h>

#include "metadata.h"

cJSON *deleted_metadata_load(
    const char *data_lobby
)
{
    char path[512];

    if (metadata_get_deleted_path(
            data_lobby,
            path,
            sizeof(path)) != 0)
    {
        return NULL;
    }

    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        /*
         * The deleted metadata file does not exist yet.
         * Treat it as an empty deleted-record list.
         */
        cJSON *metadata = cJSON_CreateObject();

        if (metadata == NULL)
            return NULL;

        cJSON_AddStringToObject(
            metadata,
            "dataLoby",
            data_lobby
        );

        cJSON_AddArrayToObject(
            metadata,
            "deleted"
        );

        return metadata;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return NULL;
    }

    long size = ftell(file);

    if (size < 0)
    {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *buffer = malloc(
        (size_t)size + 1
    );

    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    fclose(file);

    buffer[read_size] = '\0';

    cJSON *metadata = cJSON_Parse(buffer);

    free(buffer);

    if (metadata == NULL)
    {
        fprintf(
            stderr,
            "deleted metadata: invalid JSON\n"
        );

        return NULL;
    }

    return metadata;
}

int deleted_metadata_save(
    const char *data_lobby,
    const cJSON *metadata
)
{
    if (metadata_init() != 0)
        return -1;

    char path[512];

    if (metadata_get_deleted_path(
            data_lobby,
            path,
            sizeof(path)) != 0)
    {
        return -1;
    }

    char *json_string = cJSON_Print(metadata);

    if (json_string == NULL)
        return -1;

    FILE *file = fopen(path, "wb");

    if (file == NULL)
    {
        free(json_string);
        return -1;
    }

    size_t length = strlen(json_string);

    size_t written = fwrite(
        json_string,
        1,
        length,
        file
    );

    fclose(file);

    free(json_string);

    return written == length ? 0 : -1;
}

int deleted_metadata_add(
    const char *data_lobby,
    const char *record_id
)
{
    if (data_lobby == NULL ||
        record_id == NULL)
    {
        return -1;
    }

    cJSON *metadata =
        deleted_metadata_load(data_lobby);

    if (metadata == NULL)
        return -1;

    cJSON *deleted =
        cJSON_GetObjectItemCaseSensitive(
            metadata,
            "deleted"
        );

    if (!cJSON_IsArray(deleted))
    {
        deleted = cJSON_AddArrayToObject(
            metadata,
            "deleted"
        );
    }

    /*
     * Prevent duplicate IDs.
     */
    cJSON *item = NULL;

    cJSON_ArrayForEach(item, deleted)
    {
        if (cJSON_IsString(item) &&
            strcmp(
                item->valuestring,
                record_id
            ) == 0)
        {
            cJSON_Delete(metadata);
            return 0;
        }
    }

    cJSON_AddItemToArray(
        deleted,
        cJSON_CreateString(record_id)
    );

    int result = deleted_metadata_save(
        data_lobby,
        metadata
    );

    cJSON_Delete(metadata);

    return result;
}

int deleted_metadata_contains(
    const char *data_lobby,
    const char *record_id
)
{
    if (data_lobby == NULL ||
        record_id == NULL)
    {
        return -1;
    }

    cJSON *metadata =
        deleted_metadata_load(data_lobby);

    if (metadata == NULL)
        return -1;

    cJSON *deleted =
        cJSON_GetObjectItemCaseSensitive(
            metadata,
            "deleted"
        );

    if (!cJSON_IsArray(deleted))
    {
        cJSON_Delete(metadata);
        return 0;
    }

    cJSON *item = NULL;

    cJSON_ArrayForEach(item, deleted)
    {
        if (cJSON_IsString(item) &&
            strcmp(
                item->valuestring,
                record_id
            ) == 0)
        {
            cJSON_Delete(metadata);
            return 1;
        }
    }

    cJSON_Delete(metadata);

    return 0;
}