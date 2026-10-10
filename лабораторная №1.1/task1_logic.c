#include "task1_logic.h"
#include <limits.h>
#include <stddef.h>


Status find_multiples_of_x(long long x, long long out_arr[MAX_MULTIPLES], int *out_count)
{
    if (out_arr == NULL || out_count == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }


    if (x == 0) {
        *out_count = 0;
        return STATUS_OK;
    }

    int count = 0;
    for (long long n = 1; n <= 100; n++) {
        if (n % x == 0) {
            out_arr[count] = n;
            count++;
        }
    }

    *out_count = count;
    return STATUS_OK;
}

Status classify_number(long long x, PrimeClass *out_class)
{
    if (out_class == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (x <= 1) {
        *out_class = NUMBER_NEITHER;
        return STATUS_OK;
    }

    for (long long i = 2; i <= x / i; i++) {
        if (x % i == 0) {
            *out_class = NUMBER_COMPOSITE;
            return STATUS_OK;
        }
    }

    *out_class = NUMBER_PRIME;
    return STATUS_OK;
}

Status to_hex_digits(long long x, int out_digits[MAX_HEX_DIGITS], int *out_count)
{
    if (out_digits == NULL || out_count == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (x < 0) {
        return STATUS_ERR_NUMBER_NEGATIVE;
    }

    if (x == 0) {
        out_digits[0] = 0;
        *out_count = 1;
        return STATUS_OK;
    }


    int temp[MAX_HEX_DIGITS];
    int count = 0;
    long long value = x;

    while (value > 0) {
        temp[count] = (int) (value % 16);
        count++;
        value = value / 16;
    }

    for (int i = 0; i < count; i++) {
        out_digits[i] = temp[count - 1 - i];
    }

    *out_count = count;
    return STATUS_OK;
}


Status compute_power_table(long long x,
                           long long out_table[POWER_TABLE_BASES][POWER_TABLE_MAX_EXP],
                           int *out_used_cols)
{
    if (out_table == NULL || out_used_cols == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (x < 1 || x > POWER_TABLE_MAX_EXP) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    for (int base_idx = 0; base_idx < POWER_TABLE_BASES; base_idx++) {
        long long base = base_idx + 1;
        long long value = 1;

        for (long long exp = 1; exp <= x; exp++) {
            value = value * base;
            out_table[base_idx][exp - 1] = value;
        }
    }

    *out_used_cols = (int) x;
    return STATUS_OK;
}


Status sum_1_to_x(long long x, long long *out_sum)
{
    if (out_sum == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (x < 0) {
        return STATUS_ERR_NUMBER_NEGATIVE;
    }

    if (x == 0) {
        *out_sum = 0;
        return STATUS_OK;
    }

    if (x == LLONG_MAX) {
        return STATUS_ERR_COMPUTE_OVERFLOW;
    }


    long long a = x;
    long long b = x + 1;

    if (a % 2 == 0) {
        a = a / 2;
    } else {
        b = b / 2;
    }


    if (b > LLONG_MAX / a) {
        return STATUS_ERR_COMPUTE_OVERFLOW;
    }

    *out_sum = a * b;
    return STATUS_OK;
}

Status factorial_of_x(long long x, long long *out_result)
{
    if (out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    if (x < 0) {
        return STATUS_ERR_NUMBER_NEGATIVE;
    }

    long long result = 1;

    for (long long i = 2; i <= x; i++) {
        if (result > LLONG_MAX / i) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }
        result = result * i;
    }

    *out_result = result;
    return STATUS_OK;
}
