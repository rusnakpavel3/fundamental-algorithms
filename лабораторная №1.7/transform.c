#include "transform.h"

#include <ctype.h>
#include <limits.h>

Status token_rule(const int number, TokenRule *out_rule)
{
    if (out_rule == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (number < 1 || number > 10) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    out_rule->to_lower = false;
    out_rule->base = 0;

    if (number == 10) {
        out_rule->to_lower = true;
        out_rule->base = 4;
    } else if (number % 2 == 0) {
        out_rule->to_lower = true;
    } else if (number == 5) {
        out_rule->base = 8;
    }
    return STATUS_OK;
}

Status transform_char(const int c, const TokenRule *rule,
                      char *out, const size_t out_size, size_t *out_len)
{
    if (rule == NULL || out == NULL || out_len == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (c < 0 || c > UCHAR_MAX || out_size < TRANSFORM_OUTPUT_MAX) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }
    if (rule->base != 0 && rule->base != 4 && rule->base != 8) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    int value = c;
    if (rule->to_lower) {
        value = tolower(value);
    }

    if (rule->base == 0) {
        out[0] = (char) value;
        *out_len = 1;
        return STATUS_OK;
    }

    int digits = 3;
    if (rule->base == 4) {
        digits = 4;
    }

    for (int i = digits - 1; i >= 0; i--) {
        out[i] = (char) ('0' + value % rule->base);
        value = value / rule->base;
    }
    *out_len = (size_t) digits;
    return STATUS_OK;
}
