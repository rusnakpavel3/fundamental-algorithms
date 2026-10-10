#ifndef PARSING_H
#define PARSING_H

#include "status.h"

typedef enum {
    FLAG_H,
    FLAG_P,
    FLAG_S,
    FLAG_E,
    FLAG_A,
    FLAG_F
} FlagType;

Status parse_flag(const char *arg, FlagType *out_flag);

Status parse_long_long(const char *str, long long *out);

#endif
