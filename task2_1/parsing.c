#include "parsing.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

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
        case 'l':
            *out_flag = FLAG_LENGTH;
            return STATUS_OK;
        case 'r':
            *out_flag = FLAG_REVERSE;
            return STATUS_OK;
        case 'u':
            *out_flag = FLAG_UPPER_ODD;
            return STATUS_OK;
        case 'n':
            *out_flag = FLAG_GROUP;
            return STATUS_OK;
        case 'c':
            *out_flag = FLAG_CONCAT;
            return STATUS_OK;
        default:
            return STATUS_ERR_FLAG_UNKNOWN;
    }
}

Status check_argc(const Flag flag, const int argc)
{
    switch (flag) {
        case FLAG_LENGTH:
        case FLAG_REVERSE:
        case FLAG_UPPER_ODD:
        case FLAG_GROUP:
            return argc == 3 ? STATUS_OK : STATUS_ERR_ARGUMENT_COUNT;
        case FLAG_CONCAT:
            return argc >= 4 ? STATUS_OK : STATUS_ERR_ARGUMENT_COUNT;
    }

    return STATUS_ERR_FLAG_UNKNOWN;
}

Status parse_unsigned_int(const char *str, unsigned int *out)
{
    if (str == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const char *p = str;
    if (*p == '+') {
        p++;
    }
    if (!isdigit((unsigned char) *p)) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    unsigned int value = 0;
    bool too_big = false;

    while (isdigit((unsigned char) *p)) {
        const unsigned int digit = (unsigned int) (*p - '0');

        if (!too_big) {
            if (value > (UINT_MAX - digit) / 10U) {
                too_big = true;
            } else {
                value = value * 10U + digit;
            }
        }
        p++;
    }

    if (*p != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    if (too_big) {
        return STATUS_ERR_NUMBER_OUT_OF_RANGE;
    }

    *out = value;
    return STATUS_OK;
}
