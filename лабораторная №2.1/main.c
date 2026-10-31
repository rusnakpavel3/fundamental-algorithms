#include <stdio.h>
#include <stdlib.h>

#include "parsing.h"
#include "status.h"
#include "string_ops.h"

#ifdef _WIN32
#include <windows.h>
#endif

static Status run_simple(const Flag flag, const char *text)
{
    size_t length = 0;
    char *result = NULL;
    Status status = STATUS_OK;

    switch (flag) {
        case FLAG_LENGTH:
            status = string_length(text, &length);
            if (status == STATUS_OK) {
                printf("%zu\n", length);
            }
            return status;
        case FLAG_REVERSE:
            status = string_reverse(text, &result);
            break;
        case FLAG_UPPER_ODD:
            status = string_upper_odd(text, &result);
            break;
        case FLAG_GROUP:
            status = string_group_by_class(text, &result);
            break;
        case FLAG_CONCAT:
            return STATUS_ERR_FLAG_UNKNOWN;
    }

    if (status == STATUS_OK) {
        printf("%s\n", result);
        free(result);
    }
    return status;
}

static Status run_concat(const int argc, char *argv[])
{
    unsigned int seed = 0;
    Status status = parse_unsigned_int(argv[3], &seed);
    if (status != STATUS_OK) {
        return status;
    }

    const size_t count = (size_t) (argc - 3);
    const char **strings = malloc(count * sizeof *strings);
    if (strings == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    strings[0] = argv[2];
    for (int i = 4; i < argc; i++) {
        strings[i - 3] = argv[i];
    }

    char *result = NULL;
    status = string_concat_random(strings, count, seed, &result);
    free(strings);

    if (status == STATUS_OK) {
        printf("%s\n", result);
        free(result);
    }
    return status;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ARGUMENT_COUNT));
        return 1;
    }

    Flag flag = FLAG_LENGTH;
    Status status = parse_flag(argv[1], &flag);
    if (status == STATUS_OK) {
        status = check_argc(flag, argc);
    }
    if (status == STATUS_OK) {
        if (flag == FLAG_CONCAT) {
            status = run_concat(argc, argv);
        } else {
            status = run_simple(flag, argv[2]);
        }
    }

    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 1;
    }
    return 0;
}
