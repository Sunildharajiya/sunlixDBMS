#ifndef CRUD_H
#define CRUD_H

#include "cJSON.h"

/* Reader */
cJSON *reader(
    const char *filename
);

/* Writer */
int writer(
    const char *filename,
    const char *data
);

/* Update */
int updater(
    const char *filename,
    const char *key,
    const char *field,
    const char *value
);

/* Delete */
int deleter(
    const char *filename,
    const char *key
);

#endif