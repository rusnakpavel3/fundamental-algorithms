#include "digits.h"

#include <stddef.h>

static const char DIGIT_CHARS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool base_is_valid(const int base)
{
    return base >= BASE_MIN && base <= BASE_MAX;
}

Status digit_value(const char c, const int base, int *out_value)
{
    if (out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!base_is_valid(base)) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }

    int value = -1;

    if (c >= '0' && c <= '9') {
        value = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        value = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        value = c - 'a' + 10;
    }

    if (value < 0 || value >= base) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    *out_value = value;
    return STATUS_OK;
}

Status digit_char(const int value, char *out_char)
{
    if (out_char == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (value < 0 || value >= BASE_MAX) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }

    *out_char = DIGIT_CHARS[value];
    return STATUS_OK;
}
