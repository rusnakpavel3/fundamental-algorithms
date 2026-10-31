#include "custom_scan.h"

#include <limits.h>
#include <string.h>

static int roman_digit(const int c)
{
    switch (c) {
        case 'I':
            return 1;
        case 'V':
            return 5;
        case 'X':
            return 10;
        case 'L':
            return 50;
        case 'C':
            return 100;
        case 'D':
            return 500;
        case 'M':
            return 1000;
        default:
            return 0;
    }
}

static void roman_from_int(const int value, char *out)
{
    static const int VALUES[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    static const char *const SYMBOLS[] = {"M", "CM", "D", "CD", "C", "XC", "L",
                                          "XL", "X", "IX", "V", "IV", "I"};

    int rest = value;
    size_t position = 0;

    for (size_t i = 0; i < sizeof VALUES / sizeof VALUES[0]; i++) {
        while (rest >= VALUES[i]) {
            for (const char *symbol = SYMBOLS[i]; *symbol != '\0'; symbol++) {
                out[position] = *symbol;
                position++;
            }
            rest -= VALUES[i];
        }
    }
    out[position] = '\0';
}

static int digit_value(const int c, const LetterCase letters)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (letters == LETTERS_LOWER && c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    if (letters == LETTERS_UPPER && c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    return -1;
}

bool base_is_valid(const int base)
{
    return base >= BASE_MIN && base <= BASE_MAX;
}

Status scan_roman(Source *source, int *out_value)
{
    if (source == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (source_skip_spaces(source) == EOF) {
        return source_end_status(source);
    }

    char letters[ROMAN_MAX_LENGTH + 2];
    size_t length = 0;
    int c = source_get(source);

    while (c != EOF && roman_digit(c) != 0 && length <= ROMAN_MAX_LENGTH) {
        letters[length] = (char) c;
        length++;
        c = source_get(source);
    }
    source_unget(source, c);

    if (source_has_error(source)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (length == 0 || length > ROMAN_MAX_LENGTH) {
        return STATUS_MATCH_FAILURE;
    }
    letters[length] = '\0';

    int value = 0;
    for (size_t i = 0; i < length; i++) {
        const int current = roman_digit(letters[i]);
        const int next = i + 1 < length ? roman_digit(letters[i + 1]) : 0;
        value += current < next ? -current : current;
    }

    if (value < ROMAN_MIN || value > ROMAN_MAX) {
        return STATUS_MATCH_FAILURE;
    }

    char canonical[ROMAN_MAX_LENGTH + 1];
    roman_from_int(value, canonical);
    if (strcmp(canonical, letters) != 0) {
        return STATUS_MATCH_FAILURE;
    }

    *out_value = value;
    return STATUS_OK;
}

Status scan_zeckendorf(Source *source, unsigned int *out_value)
{
    if (source == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (source_skip_spaces(source) == EOF) {
        return source_end_status(source);
    }

    const unsigned long long limit = (unsigned long long) UINT_MAX + 1ULL;
    unsigned long long term = 1;
    unsigned long long next_term = 2;
    unsigned long long value = 0;
    size_t count = 0;
    int first = 0;
    int previous = 0;
    bool finished = false;
    int c = source_get(source);

    while (!finished && (c == '0' || c == '1')) {
        if (c == '1' && previous == '1') {
            finished = true;
        } else {
            if (c == '1' && value < limit) {
                value += term;
            }
            if (count == 0) {
                first = c;
            }
            count++;
            previous = c;

            const unsigned long long sum = term + next_term;
            term = next_term;
            next_term = sum > limit ? limit : sum;

            c = source_get(source);
        }
    }

    if (!finished) {
        source_unget(source, c);
        if (source_has_error(source)) {
            return STATUS_ERR_READ_FAILED;
        }
        if (count == 2 && first == '0' && previous == '1') {
            *out_value = 0;
            return STATUS_OK;
        }
        return STATUS_MATCH_FAILURE;
    }

    if (value >= limit) {
        return STATUS_MATCH_FAILURE;
    }

    *out_value = (unsigned int) value;
    return STATUS_OK;
}

Status scan_base(Source *source, const int base, const LetterCase letters, int *out_value)
{
    if (source == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!base_is_valid(base)) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }
    if (source_skip_spaces(source) == EOF) {
        return source_end_status(source);
    }

    int c = source_get(source);
    bool negative = false;

    if (c == '+' || c == '-') {
        negative = c == '-';
        c = source_get(source);
    }

    const unsigned int limit = negative ? (unsigned int) INT_MAX + 1u : (unsigned int) INT_MAX;
    const unsigned int unsigned_base = (unsigned int) base;
    unsigned int magnitude = 0;
    size_t digits = 0;
    bool too_big = false;
    int digit = digit_value(c, letters);

    while (digit >= 0 && digit < base) {
        if (!too_big) {
            if (magnitude > (limit - (unsigned int) digit) / unsigned_base) {
                too_big = true;
            } else {
                magnitude = magnitude * unsigned_base + (unsigned int) digit;
            }
        }
        digits++;
        c = source_get(source);
        digit = digit_value(c, letters);
    }
    source_unget(source, c);

    if (source_has_error(source)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (digits == 0 || too_big) {
        return STATUS_MATCH_FAILURE;
    }

    if (negative && magnitude == limit) {
        *out_value = INT_MIN;
    } else if (negative) {
        *out_value = -(int) magnitude;
    } else {
        *out_value = (int) magnitude;
    }
    return STATUS_OK;
}
