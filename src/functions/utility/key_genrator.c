#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char *generate_key(int file_id, int index)
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
        "%02d%c%02dT%02d%02d@%03d%04d",
        year,
        month,
        local->tm_mon + 1,
        local->tm_mday,
        local->tm_hour,
        file_id,
        index
    );

    return key;
}