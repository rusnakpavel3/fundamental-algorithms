#include "args.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_PREFIX "out_"
#define OUTPUT_PREFIX_LENGTH 4

Status parse_flag(const char *str, Flag *out_flag)
{
    if (str == NULL || out_flag == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (str[0] != '-' && str[0] != '/') {
        return STATUS_ERR_FLAG_FORMAT;
    }

    const size_t length = strlen(str);
    bool has_output_path = false;
    char letter;

    if (length == 2) {
        letter = str[1];
    } else if (length == 3) {
        if (str[1] != 'n') {
            return STATUS_ERR_FLAG_FORMAT;
        }
        has_output_path = true;
        letter = str[2];
    } else {
        return STATUS_ERR_FLAG_FORMAT;
    }

    Action action;
    switch (letter) {
        case 'd':
            action = ACTION_DROP_DIGITS;
            break;
        case 'i':
            action = ACTION_COUNT_LATIN;
            break;
        case 's':
            action = ACTION_COUNT_OTHER;
            break;
        case 'a':
            action = ACTION_HEX_CODES;
            break;
        default:
            return STATUS_ERR_FLAG_UNKNOWN;
    }

    out_flag->action = action;
    out_flag->has_output_path = has_output_path;
    return STATUS_OK;
}

Status required_argc(const Flag *flag, int *out_argc)
{
    if (flag == NULL || out_argc == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (flag->has_output_path) {
        *out_argc = 4;
    } else {
        *out_argc = 3;
    }
    return STATUS_OK;
}

static bool is_dir_separator(const char c)
{
#ifdef _WIN32
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
}

static bool is_name_boundary(const char c)
{
#ifdef _WIN32
    if (c == ':') {
        return true;
    }
#endif
    return is_dir_separator(c);
}

static size_t find_name_start(const char *path)
{
    size_t name_start = 0;

    for (size_t i = 0; path[i] != '\0'; i++) {
        if (is_name_boundary(path[i])) {
            name_start = i + 1;
        }
    }
    return name_start;
}

Status validate_path(const char *path)
{
    if (path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (path[0] == '\0') {
        return STATUS_ERR_PATH_EMPTY;
    }

    const char *name = path + find_name_start(path);

    if (name[0] == '\0' || strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return STATUS_ERR_PATH_NO_FILENAME;
    }
    return STATUS_OK;
}

Status build_output_path(const char *input_path, char **out_path)
{
    if (input_path == NULL || out_path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_path = NULL;

    const Status status = validate_path(input_path);
    if (status != STATUS_OK) {
        return status;
    }

    const size_t length = strlen(input_path);
    const size_t name_start = find_name_start(input_path);

    char *result = malloc(length + OUTPUT_PREFIX_LENGTH + 1);
    if (result == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    memcpy(result, input_path, name_start);
    memcpy(result + name_start, OUTPUT_PREFIX, OUTPUT_PREFIX_LENGTH);
    memcpy(result + name_start + OUTPUT_PREFIX_LENGTH,
           input_path + name_start,
           length - name_start + 1);

    *out_path = result;
    return STATUS_OK;
}
