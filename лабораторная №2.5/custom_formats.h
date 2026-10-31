#ifndef CUSTOM_FORMATS_H
#define CUSTOM_FORMATS_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define ROMAN_MIN 1
#define ROMAN_MAX 3999
#define ROMAN_BUFFER_SIZE 16
#define ZECKENDORF_BUFFER_SIZE 48
#define BASE_MIN 2
#define BASE_MAX 36
#define BASE_DEFAULT 10
#define BASE_BUFFER_SIZE (sizeof(int) * CHAR_BIT + 2)
#define DUMP_BUFFER_SIZE(bytes) ((bytes) * (CHAR_BIT + 1))

typedef enum {
    LETTERS_LOWER,
    LETTERS_UPPER
} LetterCase;

bool base_is_valid(const int base);
Status roman_from_int(const int value, char *out, const size_t out_size);
Status zeckendorf_from_uint(const unsigned int value, char *out, const size_t out_size);
Status int_to_base(const int value, const int base, const LetterCase letters,
                   char *out, const size_t out_size);
Status base_to_long_long(const char *text, const int base, const LetterCase letters,
                         long long *out_value);
Status memory_dump(const void *data, const size_t size, char *out, const size_t out_size);

#endif
