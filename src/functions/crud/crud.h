#ifndef CRUD_H
#define CRUD_H

#include "cJSON.h"

/* Reader */
cJSON *reader(const char *filename);

/* Writer */
int writer(
    const char *filename,
    const char *data,
    int record_length,
    int index
);

/* Update */
int updater(
    const char *filename,
    const char *key,
    const char *field,
    const char *value
);

/* Delete */
int delete_record(
    const char *filename,
    int index
);

#endif