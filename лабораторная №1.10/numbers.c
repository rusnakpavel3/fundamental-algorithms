#include "numbers.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

static int digit_value(const char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    return -1;
}

static char digit_char(const int digit)
{
    if (digit < 10) {
        return (char) ('0' + digit);
    }
    return (char) ('A' + digit - 10);
}

static long long absolute(const long long value)
{
    if (value < 0) {
        return -value;
    }
    return value;
}

Status parse_base(const char *str, int *out_base)
{
    if (str == NULL || out_base == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    errno = 0;
    char *endptr = NULL;
    const long value = strtol(str, &endptr, 10);

    if (endptr == str || *endptr != '\0' || errno == ERANGE) {
        return STATUS_ERR_BASE_INVALID;
    }
    if (value < 2 || value > 36) {
        return STATUS_ERR_BASE_INVALID;
    }

    *out_base = (int) value;
    return STATUS_OK;
}

Status parse_in_base(const char *str, const int base, long long *out_value)
{
    if (str == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (base < 2 || base > 36) {
        return STATUS_ERR_BASE_INVALID;
    }

    size_t i = 0;
    int negative = 0;
    if (str[0] == '-') {
        negative = 1;
        i = 1;
    }
    if (str[i] == '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }

    long long magnitude = 0;
    for (; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            return STATUS_ERR_NUMBER_LOWERCASE;
        }

        const int digit = digit_value(str[i]);
        if (digit < 0) {
            return STATUS_ERR_NUMBER_INVALID;
        }
        if (digit >= base) {
            return STATUS_ERR_DIGIT_OUT_OF_BASE;
        }
        if (magnitude > (LLONG_MAX - digit) / base) {
            return STATUS_ERR_NUMBER_TOO_BIG;
        }
        magnitude = magnitude * base + digit;
    }

    if (negative) {
        *out_value = -magnitude;
    } else {
        *out_value = magnitude;
    }
    return STATUS_OK;
}

Status stats_init(Stats *stats)
{
    if (stats == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    stats->sum_high = 0;
    stats->sum_low = 0;
    stats->max_abs = 0;
    stats->count = 0;
    return STATUS_OK;
}

Status stats_add(Stats *stats, const long long value)
{
    if (stats == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (value < -LLONG_MAX) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }
    if (stats->sum_high > LLONG_MAX - 3 || stats->sum_high < -LLONG_MAX + 3) {
        return STATUS_ERR_SUM_OVERFLOW;
    }

    stats->sum_high += value / SUM_PART_BASE;
    stats->sum_low += value % SUM_PART_BASE;
    stats->sum_high += stats->sum_low / SUM_PART_BASE;
    stats->sum_low %= SUM_PART_BASE;

    if (stats->count == 0 || absolute(value) > absolute(stats->max_abs)) {
        stats->max_abs = value;
    }
    stats->count++;
    return STATUS_OK;
}

Status stats_sum(const Stats *stats, long long *out_sum)
{
    if (stats == NULL || out_sum == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const long long high = stats->sum_high;
    const long long low = stats->sum_low;

    if (high >= -1 && high <= 1) {
        *out_sum = high * SUM_PART_BASE + low;
        return STATUS_OK;
    }
    if (high == 2 && low < 0) {
        *out_sum = (SUM_PART_BASE + low) + SUM_PART_BASE;
        return STATUS_OK;
    }
    if (high == -2 && low > 0) {
        *out_sum = (low - SUM_PART_BASE) - SUM_PART_BASE;
        return STATUS_OK;
    }
    return STATUS_ERR_SUM_OVERFLOW;
}

Status to_base(const long long value, const int base, char *out, const size_t out_size)
{
    if (out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (base < 2 || base > 36) {
        return STATUS_ERR_BASE_INVALID;
    }
    if (out_size < BASE_TEXT_MAX || value < -LLONG_MAX) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    long long magnitude = absolute(value);
    char reversed[BASE_TEXT_MAX];
    size_t count = 0;

    if (magnitude == 0) {
        reversed[count] = '0';
        count++;
    }
    while (magnitude > 0) {
        reversed[count] = digit_char((int) (magnitude % base));
        count++;
        magnitude = magnitude / base;
    }

    size_t position = 0;
    if (value < 0) {
        out[position] = '-';
        position++;
    }
    while (count > 0) {
        count--;
        out[position] = reversed[count];
        position++;
    }
    out[position] = '\0';
    return STATUS_OK;
}
