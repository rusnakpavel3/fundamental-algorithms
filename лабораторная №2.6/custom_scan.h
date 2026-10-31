#ifndef CUSTOM_SCAN_H
#define CUSTOM_SCAN_H

#include <stdbool.h>

#include "source.h"
#include "status.h"

#define ROMAN_MIN 1
#define ROMAN_MAX 3999
#define ROMAN_MAX_LENGTH 15
#define BASE_MIN 2
#define BASE_MAX 36
#define BASE_DEFAULT 10

typedef enum {
    LETTERS_LOWER,
    LETTERS_UPPER
} LetterCase;

bool base_is_valid(const int base);
Status scan_roman(Source *source, int *out_value);
Status scan_zeckendorf(Source *source, unsigned int *out_value);
Status scan_base(Source *source, const int base, const LetterCase letters, int *out_value);

#endif
