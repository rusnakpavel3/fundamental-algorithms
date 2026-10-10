#include "parsing.h"

#include <ctype.h>
#include <float.h>
#include <limits.h>
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

static bool is_numeric_notation(const char *str)
{
    const char *p = skip_sign(str);
    return isdigit((unsigned char) *p) || *p == '.';
}

static bool out_of_double_range(const char *str, const double value)
{
    if (is_numeric_notation(str) && !isfinite(value)) {
        return true;
    }
    if (value == 0.0) {
        return count_significant_digits(str) > 0;
    }
    return fabs(value) < DBL_MIN;
}

Status parse_double(const char *str, double *out)
{
    if (str == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (is_hex_notation(str)) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    char *endptr = NULL;
    const double value = strtod(str, &endptr);

    if (endptr == str || *endptr != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    if (out_of_double_range(str, value)) {
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

    const char *p = str;
    while (isspace((unsigned char) *p)) {
        p++;
    }

    bool negative = false;
    if (*p == '+' || *p == '-') {
        negative = (*p == '-');
        p++;
    }
    if (!isdigit((unsigned char) *p)) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    const unsigned long long limit = negative ? (unsigned long long) LLONG_MAX + 1ULL
                                              : (unsigned long long) LLONG_MAX;
    unsigned long long magnitude = 0;
    bool too_big = false;

    while (isdigit((unsigned char) *p)) {
        const unsigned long long digit = (unsigned long long) (*p - '0');

        if (!too_big) {
            if (magnitude > (limit - digit) / 10ULL) {
                too_big = true;
            } else {
                magnitude = magnitude * 10ULL + digit;
            }
        }
        p++;
    }

    if (*p != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    if (too_big) {
        return STATUS_ERR_INTEGER_OUT_OF_RANGE;
    }

    if (negative && magnitude == limit) {
        *out = LLONG_MIN;
    } else if (negative) {
        *out = -(long long) magnitude;
    } else {
        *out = (long long) magnitude;
    }
    return STATUS_OK;
}
