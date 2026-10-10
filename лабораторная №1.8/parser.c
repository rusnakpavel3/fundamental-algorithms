#include "parser.h"

#include <limits.h>

static int digit_value(const int c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    return -1;
}

Status parser_reset(NumberParser *parser)
{
    if (parser == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    parser->digits[0] = '\0';
    parser->count = 0;
    parser->max_digit = 0;
    parser->negative = false;
    parser->has_digit = false;
    parser->started = false;
    return STATUS_OK;
}

Status parser_feed(NumberParser *parser, const int c)
{
    if (parser == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (!parser->started && c == '-') {
        parser->started = true;
        parser->negative = true;
        return STATUS_OK;
    }
    parser->started = true;

    const int digit = digit_value(c);
    if (digit < 0) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    parser->has_digit = true;
    if (digit > parser->max_digit) {
        parser->max_digit = digit;
    }

    if (parser->count == 0 && digit == 0) {
        return STATUS_OK;
    }
    if (parser->count == NUMBER_MAX_DIGITS) {
        return STATUS_ERR_NUMBER_TOO_BIG;
    }

    parser->digits[parser->count] = (char) c;
    parser->count++;
    return STATUS_OK;
}

Status parser_finish(const NumberParser *parser, NumberResult *out_result)
{
    if (parser == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!parser->has_digit) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    int base = parser->max_digit + 1;
    if (base < 2) {
        base = 2;
    }

    unsigned long long value = 0;
    for (size_t i = 0; i < parser->count; i++) {
        const unsigned long long digit = (unsigned long long) digit_value(parser->digits[i]);

        if (value > (ULLONG_MAX - digit) / (unsigned long long) base) {
            return STATUS_ERR_NUMBER_TOO_BIG;
        }
        value = value * (unsigned long long) base + digit;
    }

    size_t position = 0;
    out_result->negative = parser->negative && parser->count > 0;

    if (out_result->negative) {
        out_result->text[position] = '-';
        position++;
    }
    if (parser->count == 0) {
        out_result->text[position] = '0';
        position++;
    }
    for (size_t i = 0; i < parser->count; i++) {
        out_result->text[position] = parser->digits[i];
        position++;
    }
    out_result->text[position] = '\0';

    out_result->base = base;
    out_result->value = value;
    return STATUS_OK;
}
