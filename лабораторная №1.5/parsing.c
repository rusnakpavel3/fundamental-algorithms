#include "parsing.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

static const char *skip_sign(const char *str)
{
    while (isspace((unsigned char) *str)) {
        str++;
    }
    if (*str == '+' || *str == '-') {
        str++;
    }
    return str;
}

static bool is_hex_notation(const char *str)
{
    const char *p = skip_sign(str);
    return p[0] == '0' && (p[1] == 'x' || p[1] == 'X');
}

static int count_significant_digits(const char *str)
{
    int count = 0;
    int pending_zeros = 0;
    bool started = false;

    for (const char *p = skip_sign(str); *p != '\0' && *p != 'e' && *p != 'E'; p++) {
        if (*p == '.') {
            continue;
        }
        if (!isdigit((unsigned char) *p)) {
            break;
        }
        if (*p == '0') {
            if (started) {
                pending_zeros++;
            }
        } else {
            if (started) {
                count += pending_zeros;
            }
            pending_zeros = 0;
            started = true;
            count++;
        }
    }
    return count;
}

Status parse_double(const char *str, double *out)
{
    if (str == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (is_hex_notation(str)) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    errno = 0;
    char *endptr = NULL;
    double value = strtod(str, &endptr);

    if (endptr == str || *endptr != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    if (errno == ERANGE) {
        return STATUS_ERR_NUMBER_OUT_OF_RANGE;
    }
    if (!isfinite(value)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    if (count_significant_digits(str) > DBL_DIG) {
        return STATUS_ERR_NUMBER_TOO_PRECISE;
    }

    *out = value;
    return STATUS_OK;
}
