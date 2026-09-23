#ifndef METADATA_H
#define METADATA_H

#include <cJSON.h>

/*
 * Metadata paths
 */
#define METADATA_PATH "metadata"

/*
 * Get the metadata file path for a data lobby.
 *
 * Example:
 * users -> metadata/users.meta.json
 */
int metadata_get_path(
    const char *data_lobby,
    char *path,
    size_t path_size
);

/*
 * Get the deleted-record metadata path.
 *
 * Example:
 * users -> metadata/users.deleted.json
 */
int metadata_get_deleted_path(
    const char *data_lobby,
    char *path,
    size_t path_size
);

/*
 * Create the metadata directory if it does not exist.
 */
int metadata_init(void);

/*
 * Create initial metadata for a data lobby.
 */
int metadata_create(
    const char *data_lobby
);

/*
 * Load metadata JSON.
 *
 * Caller owns the returned cJSON object.
 */
cJSON *metadata_load(
    const char *data_lobby
);

/*
 * Save metadata JSON.
 */
int metadata_save(
    const char *data_lobby,
    const cJSON *metadata
);

/*
 * Get the next global record index.
 */
int metadata_get_next_index(
    const char *data_lobby
);

/*
 * Update the next global record index.
 */
int metadata_set_next_index(
    cJSON *metadata,
    int next_index
);

/*
 * Add a subfile and its record range.
 */
int metadata_add_subfile(
    const char *data_lobby,
    int file_id,
    int start_record,
    int end_record
);

/*
 * Find the subfile containing a record index.
 *
 * Returns:
 *   file ID >= 0 -> found
 *   -1            -> not found/error
 */
int metadata_find_subfile(
    const char *data_lobby,
    int record_index
);

/*
 * Load deleted-record metadata.
 *
 * Caller owns the returned cJSON object.
 */
cJSON *deleted_metadata_load(
    const char *data_lobby
);

/*
 * Save deleted-record metadata.
 */
int deleted_metadata_save(
    const char *data_lobby,
    const cJSON *metadata
);

/*
 * Add a deleted record ID.
 */
int deleted_metadata_add(
    const char *data_lobby,
    const char *record_id
);

/*
 * Check whether a record ID is marked as deleted.
 *
 * Returns:
 *   1 -> deleted
 *   0 -> not deleted
 *  -1 -> error
 */
int deleted_metadata_contains(
    const char *data_lobby,
    const char *record_id
);

int metadata_update_subfile_end(
    cJSON *metadata,
    int file_id,
    int end_index
);

#endif