#ifndef PARSING_H
#define PARSING_H

#include "status.h"

typedef enum {
    FLAG_LENGTH,
    FLAG_REVERSE,
    FLAG_UPPER_ODD,
    FLAG_GROUP,
    FLAG_CONCAT
} Flag;

Status parse_flag(const char *str, Flag *out_flag);
Status check_argc(const Flag flag, const int argc);
Status parse_unsigned_int(const char *str, unsigned int *out);

#endif
