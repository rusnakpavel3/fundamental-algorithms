#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define TRANSFORM_OUTPUT_MAX 8

typedef struct {
    bool to_lower;
    int base;
} TokenRule;

Status token_rule(const int number, TokenRule *out_rule);

Status transform_char(const int c, const TokenRule *rule,
                      char *out, const size_t out_size, size_t *out_len);

#endif
