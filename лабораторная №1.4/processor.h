#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define PROCESSOR_OUTPUT_MAX 32

typedef enum {
    ACTION_DROP_DIGITS,
    ACTION_COUNT_LATIN,
    ACTION_COUNT_OTHER,
    ACTION_HEX_CODES
} Action;

typedef struct {
    Action action;
    unsigned long long counter;
    bool line_started;
    bool need_separator;
} Processor;

Status processor_init(Processor *processor, const Action action);

Status processor_feed(Processor *processor, const int c,
                      char *out, const size_t out_size, size_t *out_len);

Status processor_finish(Processor *processor,
                        char *out, const size_t out_size, size_t *out_len);

#endif
