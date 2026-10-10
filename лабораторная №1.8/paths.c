#include "paths.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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
