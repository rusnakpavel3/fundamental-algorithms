#include "parsing.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

Status parse_int(const char *str, int *out)
{
    if (str == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    errno = 0;
    char *endptr = NULL;
    const long long value = strtoll(str, &endptr, 10);

    if (endptr == str || *endptr != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    if (errno == ERANGE || value < INT_MIN || value > INT_MAX) {
        return STATUS_ERR_NUMBER_OUT_OF_RANGE;
    }

    *out = (int) value;
    return STATUS_OK;
}
