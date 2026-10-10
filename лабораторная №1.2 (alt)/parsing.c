#include "parsing.h"

#include <ctype.h>
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
        return STATUS_ERR_NUMBER_OVERFLOW;
    }

    if (count_significant_digits(str) > DBL_DIG) {
        return STATUS_ERR_NUMBER_TOO_PRECISE;
    }

    *out = value;
    return STATUS_OK;
}
