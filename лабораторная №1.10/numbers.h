#ifndef NUMBERS_H
#define NUMBERS_H

#include <stddef.h>

#include "status.h"

#define BASE_TEXT_MAX 66
#define SUM_PART_BASE (1LL << 62)

typedef struct {
    long long sum_high;
    long long sum_low;
    long long max_abs;
    size_t count;
} Stats;

Status parse_base(const char *str, int *out_base);

Status parse_in_base(const char *str, const int base, long long *out_value);

Status stats_init(Stats *stats);

Status stats_add(Stats *stats, const long long value);

Status stats_sum(const Stats *stats, long long *out_sum);

Status to_base(const long long value, const int base, char *out, const size_t out_size);

#endif
