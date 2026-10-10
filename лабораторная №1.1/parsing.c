#include "parsing.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>

Status parse_flag(const char *arg, FlagType *out_flag)
{
    if (arg == NULL || out_flag == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }


    if (strlen(arg) != 2) {
        return STATUS_ERR_FLAG_FORMAT;
    }
    if (arg[0] != '-' && arg[0] != '/') {
        return STATUS_ERR_FLAG_FORMAT;
    }

    switch (arg[1]) {
        case 'h': *out_flag = FLAG_H; break;
        case 'p': *out_flag = FLAG_P; break;
        case 's': *out_flag = FLAG_S; break;
        case 'e': *out_flag = FLAG_E; break;
        case 'a': *out_flag = FLAG_A; break;
        case 'f': *out_flag = FLAG_F; break;
        default:
            return STATUS_ERR_FLAG_UNKNOWN;
    }

    return STATUS_OK;
}

Status parse_long_long(const char *str, long long *out)
{
    if (str == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    errno = 0;
    char *endptr = NULL;
    long long value = strtoll(str, &endptr, 10);


    if (endptr == str || *endptr != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }

    if (errno == ERANGE) {
        return STATUS_ERR_NUMBER_OVERFLOW;
    }

    *out = value;
    return STATUS_OK;
}
