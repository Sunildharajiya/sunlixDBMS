#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include <cJSON.h>

#include "metadata.h"

#define METADATA_PATH "metadata"

static int build_path(
    const char *data_lobby,
    const char *suffix,
    char *path,
    size_t path_size
)
{
    if (data_lobby == NULL ||
        suffix == NULL ||
        path == NULL)
    {
        return -1;
    }

    int written = snprintf(
        path,
        path_size,
        "%s/%s%s",
        METADATA_PATH,
        data_lobby,
        suffix
    );

    if (written < 0 || (size_t)written >= path_size)
        return -1;

    return 0;
}

int metadata_get_path(
    const char *data_lobby,
    char *path,
    size_t path_size
)
{
    return build_path(
        data_lobby,
        ".meta.json",
        path,
        path_size
    );
}

int metadata_get_deleted_path(
    const char *data_lobby,
    char *path,
    size_t path_size
)
{
    return build_path(
        data_lobby,
        ".deleted.json",
        path,
        path_size
    );
}

int metadata_init(void)
{
    if (mkdir(METADATA_PATH, 0755) != 0)
    {
        if (errno == EEXIST)
            return 0;

        perror("metadata: unable to create metadata directory");
        return -1;
    }

    return 0;
}

int metadata_create(
    const char *data_lobby
)
{
    if (data_lobby == NULL)
        return -1;

    if (metadata_init() != 0)
        return -1;

    char path[512];

    if (metadata_get_path(
            data_lobby,
            path,
            sizeof(path)) != 0)
    {
        fprintf(
            stderr,
            "metadata: path is too long\n"
        );

        return -1;
    }

    /*
     * Do not overwrite existing metadata.
     */
    FILE *existing = fopen(path, "r");

    if (existing != NULL)
    {
        fclose(existing);
        return 0;
    }

    cJSON *metadata = cJSON_CreateObject();

    if (metadata == NULL)
        return -1;

    cJSON_AddStringToObject(
        metadata,
        "dataLoby",
        data_lobby
    );

    cJSON_AddNumberToObject(
        metadata,
        "nextIndex",
        1
    );

    cJSON_AddObjectToObject(
        metadata,
        "subfiles"
    );

    int result = metadata_save(
        data_lobby,
        metadata
    );

    cJSON_Delete(metadata);

    return result;
}

cJSON *metadata_load(
    const char *data_lobby
)
{
    char path[512];

    if (metadata_get_path(
            data_lobby,
            path,
            sizeof(path)) != 0)
    {
        fprintf(
            stderr,
            "metadata: path is too long\n"
        );

        return NULL;
    }

    FILE *file = fopen(path, "rb");

    if (file == NULL)
        return NULL;

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

    char *buffer = malloc((size_t)size + 1);

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
            "metadata: invalid JSON\n"
        );

        return NULL;
    }

    return metadata;
}

int metadata_save(
    const char *data_lobby,
    const cJSON *metadata
)
{
    if (data_lobby == NULL ||
        metadata == NULL)
    {
        return -1;
    }

    if (metadata_init() != 0)
        return -1;

    char path[512];

    if (metadata_get_path(
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

        perror(
            "metadata: unable to open metadata file"
        );

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

    if (written != length)
    {
        fprintf(
            stderr,
            "metadata: failed to write metadata\n"
        );

        return -1;
    }

    return 0;
}

int metadata_get_next_index(
    const char *data_lobby
)
{
    cJSON *metadata = metadata_load(data_lobby);

    if (metadata == NULL)
        return -1;

    cJSON *next_index = cJSON_GetObjectItemCaseSensitive(
        metadata,
        "nextIndex"
    );

    if (!cJSON_IsNumber(next_index))
    {
        cJSON_Delete(metadata);
        return -1;
    }

    int index = next_index->valueint;

    cJSON_Delete(metadata);

    return index;
}

int metadata_set_next_index(
    const char *data_lobby,
    int next_index
)
{
    if (next_index < 1)
        return -1;

    cJSON *metadata = metadata_load(data_lobby);

    if (metadata == NULL)
        return -1;

    cJSON *value = cJSON_GetObjectItemCaseSensitive(
        metadata,
        "nextIndex"
    );

    if (value == NULL)
    {
        cJSON_AddNumberToObject(
            metadata,
            "nextIndex",
            next_index
        );
    }
    else
    {
        cJSON_SetNumberValue(
            value,
            next_index
        );
    }

    int result = metadata_save(
        data_lobby,
        metadata
    );

    cJSON_Delete(metadata);

    return result;
}

int metadata_add_subfile(
    const char *data_lobby,
    int file_id,
    int start_record,
    int end_record
)
{
    if (file_id < 0 ||
        start_record < 1 ||
        end_record < start_record)
    {
        return -1;
    }

    cJSON *metadata = metadata_load(data_lobby);

    if (metadata == NULL)
        return -1;

    cJSON *subfiles = cJSON_GetObjectItemCaseSensitive(
        metadata,
        "subfiles"
    );

    if (!cJSON_IsObject(subfiles))
    {
        subfiles = cJSON_AddObjectToObject(
            metadata,
            "subfiles"
        );
    }

    char file_key[32];

    snprintf(
        file_key,
        sizeof(file_key),
        "%03d",
        file_id
    );

    /*
     * Do not silently overwrite an existing
     * subfile definition.
     */
    if (cJSON_HasObjectItem(
            subfiles,
            file_key))
    {
        cJSON_Delete(metadata);
        return -1;
    }

    cJSON *subfile = cJSON_CreateObject();

    if (subfile == NULL)
    {
        cJSON_Delete(metadata);
        return -1;
    }

    cJSON_AddNumberToObject(
        subfile,
        "dataStartsWithRecord",
        start_record
    );

    cJSON_AddNumberToObject(
        subfile,
        "dataEndsWithRecord",
        end_record
    );

    cJSON_AddItemToObject(
        subfiles,
        file_key,
        subfile
    );

    int result = metadata_save(
        data_lobby,
        metadata
    );

    cJSON_Delete(metadata);

    return result;
}

int metadata_find_subfile(
    const char *data_lobby,
    int record_index
)
{
    if (record_index < 1)
        return -1;

    cJSON *metadata = metadata_load(data_lobby);

    if (metadata == NULL)
        return -1;

    cJSON *subfiles = cJSON_GetObjectItemCaseSensitive(
        metadata,
        "subfiles"
    );

    if (!cJSON_IsObject(subfiles))
    {
        cJSON_Delete(metadata);
        return -1;
    }

    cJSON *subfile = NULL;

    cJSON_ArrayForEach(subfile, subfiles)
    {
        cJSON *start =
            cJSON_GetObjectItemCaseSensitive(
                subfile,
                "dataStartsWithRecord"
            );

        cJSON *end =
            cJSON_GetObjectItemCaseSensitive(
                subfile,
                "dataEndsWithRecord"
            );

        if (!cJSON_IsNumber(start) ||
            !cJSON_IsNumber(end))
        {
            continue;
        }

        if (record_index >= start->valueint &&
            record_index <= end->valueint)
        {
            int file_id = atoi(subfile->string);

            cJSON_Delete(metadata);

            return file_id;
        }
    }

    cJSON_Delete(metadata);

    return -1;
}

int metadata_update_subfile_end(
    cJSON *metadata,
    int file_id,
    int end_index
)
{
    if (metadata == NULL ||
        end_index < 1)
    {
        return -1;
    }

    cJSON *subfiles = cJSON_GetObjectItem(
        metadata,
        "subfiles"
    );

    if (!cJSON_IsObject(subfiles))
    {
        return -1;
    }

    char file_key[16];

    snprintf(
        file_key,
        sizeof(file_key),
        "%03d",
        file_id
    );

    cJSON *subfile = cJSON_GetObjectItem(
        subfiles,
        file_key
    );

    if (!cJSON_IsObject(subfile))
    {
        return -1;
    }

    cJSON *end = cJSON_GetObjectItem(
        subfile,
        "dataEndsWithRecord"
    );

    if (end == NULL)
    {
        cJSON_AddNumberToObject(
            subfile,
            "dataEndsWithRecord",
            end_index
        );
    }
    else
    {
        cJSON_SetNumberValue(
            end,
            end_index
        );
    }

    return 0;
}