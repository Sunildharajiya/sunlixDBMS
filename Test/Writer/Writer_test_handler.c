#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <cJSON.h>
#include <crud.h>

#define TEST_CONFIG "Test/Writer/writertests.json"
#define DATA_PATH "data"

/* ---------------------------------------------------------
 * Read complete file into memory
 * --------------------------------------------------------- */

static char *read_file(const char *filename)
{
    FILE *file;
    char *buffer;
    long size;
    size_t read_size;

    file = fopen(filename, "r");

    if (file == NULL)
        return NULL;

    fseek(file, 0, SEEK_END);

    size = ftell(file);

    if (size < 0)
    {
        fclose(file);
        return NULL;
    }

    rewind(file);

    buffer = malloc((size_t)size + 1);

    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    read_size = fread(
        buffer,
        1,
        (size_t)size,
        file
    );

    fclose(file);

    buffer[read_size] = '\0';

    return buffer;
}

/* ---------------------------------------------------------
 * Remove previous test database
 * --------------------------------------------------------- */

static void remove_test_file(const char *filename)
{
    char path[512];

    snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    remove(path);
}

/* ---------------------------------------------------------
 * Validate database record count
 * --------------------------------------------------------- */

static int check_record_count(
    const char *filename,
    int expected
)
{
    char path[512];
    char *buffer;
    cJSON *database;
    int count;

    snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    buffer = read_file(path);

    if (buffer == NULL)
        return expected == 0;

    database = cJSON_Parse(buffer);

    free(buffer);

    if (database == NULL)
        return 0;

    if (!cJSON_IsArray(database))
    {
        cJSON_Delete(database);
        return 0;
    }

    count = cJSON_GetArraySize(database);

    cJSON_Delete(database);

    return count == expected;
}

/* ---------------------------------------------------------
 * Validate generated key
 * --------------------------------------------------------- */

static int check_keys(const char *filename)
{
    char path[512];
    char *buffer;
    cJSON *database;
    cJSON *record;
    cJSON *key;
    int i;
    int count;

    snprintf(
        path,
        sizeof(path),
        "%s/%s",
        DATA_PATH,
        filename
    );

    buffer = read_file(path);

    if (buffer == NULL)
        return 0;

    database = cJSON_Parse(buffer);

    free(buffer);

    if (database == NULL)
        return 0;

    if (!cJSON_IsArray(database))
    {
        cJSON_Delete(database);
        return 0;
    }

    count = cJSON_GetArraySize(database);

    for (i = 0; i < count; i++)
    {
        record = cJSON_GetArrayItem(database, i);

        if (!cJSON_IsObject(record))
        {
            cJSON_Delete(database);
            return 0;
        }

        key = cJSON_GetObjectItem(
            record,
            "key"
        );

        if (!cJSON_IsString(key) ||
            key->valuestring == NULL ||
            strlen(key->valuestring) == 0)
        {
            cJSON_Delete(database);
            return 0;
        }
    }

    cJSON_Delete(database);

    return 1;
}

/* ---------------------------------------------------------
 * Run one test
 * --------------------------------------------------------- */

static int run_test(cJSON *test)
{
    const char *name;
    const char *filename;
    const char *data;

    int record_length;
    int index;
    int expected_return;
    int expected_records;

    int result;
    int records_ok;

    name = cJSON_GetObjectItem(
        test,
        "name"
    )->valuestring;

    filename = cJSON_GetObjectItem(
        test,
        "filename"
    )->valuestring;

    data = cJSON_GetObjectItem(
        test,
        "data"
    )->valuestring;

    record_length = cJSON_GetObjectItem(
        test,
        "record_length"
    )->valueint;

    index = cJSON_GetObjectItem(
        test,
        "index"
    )->valueint;

    expected_return = cJSON_GetObjectItem(
        test,
        "expected_return"
    )->valueint;

    expected_records = cJSON_GetObjectItem(
        test,
        "expected_records"
    )->valueint;

    /*
     * Invalid JSON / independent tests should
     * start from a clean file.
     */
    if (expected_records <= 1)
        remove_test_file(filename);

    printf(
        "\n[TEST] %s\n",
        name
    );

    result = writer(
        filename,
        data,
        record_length,
        index
    );

    if (result != expected_return)
    {
        printf(
            "[FAIL] Expected return %d, got %d\n",
            expected_return,
            result
        );

        return 0;
    }

    records_ok = check_record_count(
        filename,
        expected_records
    );

    if (!records_ok)
    {
        printf(
            "[FAIL] Record count mismatch\n"
        );

        return 0;
    }

    /*
     * Successful writer calls must generate keys.
     */
    if (expected_return == 0)
    {
        if (!check_keys(filename))
        {
            printf(
                "[FAIL] Generated key validation failed\n"
            );

            return 0;
        }
    }

    printf("[PASS]\n");

    return 1;
}

/* ---------------------------------------------------------
 * Main test entry
 * --------------------------------------------------------- */

int main(void)
{
    char *buffer;
    cJSON *root;
    cJSON *tests;
    cJSON *test;

    int total = 0;
    int passed = 0;
    int failed = 0;

    printf(
        "\n========================================\n"
        "      SunlixDBMS Writer Test Suite\n"
        "========================================\n"
    );

    buffer = read_file(TEST_CONFIG);

    if (buffer == NULL)
    {
        fprintf(
            stderr,
            "Unable to read test configuration: %s\n",
            TEST_CONFIG
        );

        return 1;
    }

    root = cJSON_Parse(buffer);

    free(buffer);

    if (root == NULL)
    {
        fprintf(
            stderr,
            "Invalid test configuration JSON\n"
        );

        return 1;
    }

    tests = cJSON_GetObjectItem(
        root,
        "tests"
    );

    if (!cJSON_IsArray(tests))
    {
        fprintf(
            stderr,
            "Test configuration does not contain a tests array\n"
        );

        cJSON_Delete(root);

        return 1;
    }

    cJSON_ArrayForEach(test, tests)
    {
        total++;

        if (run_test(test))
            passed++;
        else
            failed++;
    }

    cJSON_Delete(root);

    printf(
        "\n========================================\n"
        "Results\n"
        "========================================\n"
        "Total  : %d\n"
        "Passed : %d\n"
        "Failed : %d\n"
        "========================================\n",
        total,
        passed,
        failed
    );

    return failed == 0 ? 0 : 1;
}