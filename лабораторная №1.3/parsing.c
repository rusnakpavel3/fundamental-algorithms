#include "parsing.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

Status parse_flag(const char *str, Flag *out_flag)
{
    if (str == NULL || out_flag == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (str[0] != '-' && str[0] != '/') {
        return STATUS_ERR_FLAG_FORMAT;
    }
    if (str[1] == '\0' || str[2] != '\0') {
        return STATUS_ERR_FLAG_FORMAT;
    }

    switch (str[1]) {
        case 'q':
            *out_flag = FLAG_QUADRATIC;
            return STATUS_OK;
        case 'm':
            *out_flag = FLAG_MULTIPLE;
            return STATUS_OK;
        case 't':
            *out_flag = FLAG_TRIANGLE;
            return STATUS_OK;
        default:
            return STATUS_ERR_FLAG_UNKNOWN;
    }
}

Status required_argc(const Flag flag, int *out_argc)
{
    if (out_argc == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    switch (flag) {
        case FLAG_QUADRATIC:
            *out_argc = 6;
            return STATUS_OK;
        case FLAG_MULTIPLE:
            *out_argc = 4;
            return STATUS_OK;
        case FLAG_TRIANGLE:
            *out_argc = 6;
            return STATUS_OK;
    }

    return STATUS_ERR_VALUE_OUT_OF_RANGE;
}

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
        return STATUS_ERR_INTEGER_OUT_OF_RANGE;
    }

    *out = value;
    return STATUS_OK;
}
