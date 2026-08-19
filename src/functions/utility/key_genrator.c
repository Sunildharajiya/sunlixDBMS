#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <key_genrator.h>

char *generate_key(int record_length, int index)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local == NULL)
        return NULL;

    const char month_codes[] = {
        'J', 'F', 'M', 'P',
        'Y', 'J', 'L', 'A',
        'S', 'O', 'N', 'D'
    };

    char *key = malloc(32);

    if (key == NULL)
        return NULL;

    int year = (local->tm_year + 1900) % 100;
    char month = month_codes[local->tm_mon];

    snprintf(
        key,
        32,
        "%02d%c%02dT%02d%02d@%02d%02d",
        year,
        month,
        local->tm_mon + 1,
        local->tm_mday,
        local->tm_hour,
        local->tm_min,
        record_length,
        index
    );

    return key;
}