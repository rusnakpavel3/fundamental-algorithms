#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define NUMBER_MAX_DIGITS 64

typedef struct {
    char digits[NUMBER_MAX_DIGITS + 1];
    size_t count;
    int max_digit;
    bool negative;
    bool has_digit;
    bool started;
} NumberParser;

typedef struct {
    char text[NUMBER_MAX_DIGITS + 2];
    int base;
    bool negative;
    unsigned long long value;
} NumberResult;

Status parser_reset(NumberParser *parser);

Status parser_feed(NumberParser *parser, const int c);

Status parser_finish(const NumberParser *parser, NumberResult *out_result);

#endif
