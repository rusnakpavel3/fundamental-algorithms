#ifndef DIGITS_H
#define DIGITS_H

#include <stdbool.h>

#include "status.h"

#define BASE_MIN 2
#define BASE_MAX 36

bool base_is_valid(const int base);
Status digit_value(const char c, const int base, int *out_value);
Status digit_char(const int value, char *out_char);

#endif
