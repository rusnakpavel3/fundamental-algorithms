#include "custom_formats.h"

static char digit_char(const unsigned int value, const LetterCase letters)
{
    static const char LOWER[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    static const char UPPER[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    return letters == LETTERS_UPPER ? UPPER[value] : LOWER[value];
}

static int digit_value(const char c, const LetterCase letters)
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

Status roman_from_int(const int value, char *out, const size_t out_size)
{
    static const int VALUES[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    static const char *const SYMBOLS[] = {"M", "CM", "D", "CD", "C", "XC", "L",
                                          "XL", "X", "IX", "V", "IV", "I"};

    if (out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (value < ROMAN_MIN || value > ROMAN_MAX) {
        return STATUS_ERR_ROMAN_OUT_OF_RANGE;
    }
    if (out_size < ROMAN_BUFFER_SIZE) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

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
    return STATUS_OK;
}

Status zeckendorf_from_uint(const unsigned int value, char *out, const size_t out_size)
{
    if (out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (out_size < ZECKENDORF_BUFFER_SIZE) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    unsigned int fibonacci[ZECKENDORF_BUFFER_SIZE];
    size_t count = 1;
    unsigned long long smaller = 1;
    unsigned long long larger = 2;

    fibonacci[0] = 1;
    while (larger <= value && count < ZECKENDORF_BUFFER_SIZE - 2) {
        fibonacci[count] = (unsigned int) larger;
        count++;

        const unsigned long long next = smaller + larger;
        smaller = larger;
        larger = next;
    }

    unsigned int rest = value;

    for (size_t i = count; i > 0; i--) {
        if (fibonacci[i - 1] <= rest) {
            out[i - 1] = '1';
            rest -= fibonacci[i - 1];
        } else {
            out[i - 1] = '0';
        }
    }

    if (rest != 0) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    out[count] = '1';
    out[count + 1] = '\0';
    return STATUS_OK;
}

Status int_to_base(const int value, const int base, const LetterCase letters,
                   char *out, const size_t out_size)
{
    if (out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!base_is_valid(base)) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }
    if (out_size < BASE_BUFFER_SIZE) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    const unsigned int unsigned_base = (unsigned int) base;
    unsigned int magnitude = value < 0 ? 0u - (unsigned int) value : (unsigned int) value;
    char reversed[BASE_BUFFER_SIZE];
    size_t count = 0;

    do {
        reversed[count] = digit_char(magnitude % unsigned_base, letters);
        count++;
        magnitude /= unsigned_base;
    } while (magnitude != 0);

    size_t position = 0;
    if (value < 0) {
        out[position] = '-';
        position++;
    }
    while (count > 0) {
        count--;
        out[position] = reversed[count];
        position++;
    }

    out[position] = '\0';
    return STATUS_OK;
}

Status base_to_long_long(const char *text, const int base, const LetterCase letters,
                         long long *out_value)
{
    if (text == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!base_is_valid(base)) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }

    const char *p = text;
    bool negative = false;

    if (*p == '-') {
        negative = true;
        p++;
    }
    if (*p == '\0') {
        return STATUS_ERR_BASE_NUMBER_EMPTY;
    }

    const unsigned long long limit = negative ? (unsigned long long) LLONG_MAX + 1ULL
                                              : (unsigned long long) LLONG_MAX;
    const unsigned long long unsigned_base = (unsigned long long) base;
    unsigned long long magnitude = 0;
    bool too_big = false;

    for (; *p != '\0'; p++) {
        const int digit = digit_value(*p, letters);
        if (digit < 0 || digit >= base) {
            return STATUS_ERR_BASE_NUMBER_INVALID;
        }

        if (!too_big) {
            if (magnitude > (limit - (unsigned long long) digit) / unsigned_base) {
                too_big = true;
            } else {
                magnitude = magnitude * unsigned_base + (unsigned long long) digit;
            }
        }
    }

    if (too_big) {
        return STATUS_ERR_BASE_NUMBER_OVERFLOW;
    }

    if (negative && magnitude == limit) {
        *out_value = LLONG_MIN;
    } else if (negative) {
        *out_value = -(long long) magnitude;
    } else {
        *out_value = (long long) magnitude;
    }
    return STATUS_OK;
}

Status memory_dump(const void *data, const size_t size, char *out, const size_t out_size)
{
    if (data == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (out_size == 0 || size > out_size / (CHAR_BIT + 1)) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    const unsigned char *bytes = (const unsigned char *) data;
    size_t position = 0;

    for (size_t i = 0; i < size; i++) {
        if (i > 0) {
            out[position] = ' ';
            position++;
        }
        for (int bit = CHAR_BIT - 1; bit >= 0; bit--) {
            out[position] = ((bytes[i] >> bit) & 1u) ? '1' : '0';
            position++;
        }
    }

    out[position] = '\0';
    return STATUS_OK;
}
