#ifndef TASK1_LOGIC_H
#define TASK1_LOGIC_H

#include "status.h"

#define MAX_MULTIPLES 100

#define MAX_HEX_DIGITS 16

#define POWER_TABLE_BASES 10
#define POWER_TABLE_MAX_EXP 10

typedef enum {
    NUMBER_PRIME,
    NUMBER_COMPOSITE,
    NUMBER_NEITHER
} PrimeClass;

Status find_multiples_of_x(long long x, long long out_arr[MAX_MULTIPLES], int *out_count);

Status classify_number(long long x, PrimeClass *out_class);

Status to_hex_digits(long long x, int out_digits[MAX_HEX_DIGITS], int *out_count);

Status compute_power_table(long long x,
                           long long out_table[POWER_TABLE_BASES][POWER_TABLE_MAX_EXP],
                           int *out_used_cols);

Status sum_1_to_x(long long x, long long *out_sum);

Status factorial_of_x(long long x, long long *out_result);

#endif
