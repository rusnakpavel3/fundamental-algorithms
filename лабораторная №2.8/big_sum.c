#include "big_sum.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "digits.h"

static Status make_zero(char **out_zero)
{
    char *zero = malloc(2);
    if (zero == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    zero[0] = '0';
    zero[1] = '\0';
    *out_zero = zero;
    return STATUS_OK;
}

static void remove_leading_zeros(char *digits, const size_t last_index)
{
    size_t start = 0;

    while (start < last_index && digits[start] == '0') {
        start++;
    }

    size_t i = 0;
    while (digits[start + i] != '\0') {
        digits[i] = digits[start + i];
        i++;
    }
    digits[i] = '\0';
}

Status add_in_column(const char *a, const char *b, const int base, char **out_sum)
{
    if (a == NULL || b == NULL || out_sum == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!base_is_valid(base)) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }

    const size_t length_a = strlen(a);
    const size_t length_b = strlen(b);

    if (length_a == 0 || length_b == 0) {
        return STATUS_ERR_NUMBER_EMPTY;
    }

    const size_t max_length = length_a > length_b ? length_a : length_b;

    if (max_length > SIZE_MAX - 2) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    char *result = malloc(max_length + 2);
    if (result == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    result[max_length + 1] = '\0';

    int carry = 0;

    for (size_t i = 0; i < max_length; i++) {
        int digit_a = 0;
        int digit_b = 0;
        Status status = STATUS_OK;

        if (i < length_a) {
            status = digit_value(a[length_a - 1 - i], base, &digit_a);
        }
        if (status == STATUS_OK && i < length_b) {
            status = digit_value(b[length_b - 1 - i], base, &digit_b);
        }

        const int column_sum = digit_a + digit_b + carry;

        if (status == STATUS_OK) {
            status = digit_char(column_sum % base, &result[max_length - i]);
        }
        if (status != STATUS_OK) {
            free(result);
            return status;
        }

        carry = column_sum / base;
    }

    const Status status = digit_char(carry, &result[0]);
    if (status != STATUS_OK) {
        free(result);
        return status;
    }

    remove_leading_zeros(result, max_length);
    *out_sum = result;
    return STATUS_OK;
}

Status sum_in_base(char **out_result, int base, int count, ...)
{
    if (out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_result = NULL;

    if (!base_is_valid(base)) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }
    if (count < 0) {
        return STATUS_ERR_COUNT_NEGATIVE;
    }

    char *accumulator = NULL;
    Status status = make_zero(&accumulator);
    if (status != STATUS_OK) {
        return status;
    }

    va_list args;
    va_start(args, count);

    for (int i = 0; i < count && status == STATUS_OK; i++) {
        const char *number = va_arg(args, const char *);
        char *next = NULL;

        status = add_in_column(accumulator, number, base, &next);
        if (status == STATUS_OK) {
            free(accumulator);
            accumulator = next;
        }
    }

    va_end(args);

    if (status != STATUS_OK) {
        free(accumulator);
        return status;
    }

    *out_result = accumulator;
    return STATUS_OK;
}
