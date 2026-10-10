#ifndef PARSING_H
#define PARSING_H

#include "status.h"

typedef enum {
    FLAG_QUADRATIC,
    FLAG_MULTIPLE,
    FLAG_TRIANGLE
} Flag;

Status parse_flag(const char *str, Flag *out_flag);

Status required_argc(const Flag flag, int *out_argc);

Status parse_double(const char *str, double *out);

Status parse_long_long(const char *str, long long *out);

#endif
