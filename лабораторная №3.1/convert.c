#include "convert.h"

#include <stdbool.h>



static unsigned int bit_add(unsigned int a, unsigned int b)
{
    while (b != 0u) {
        const unsigned int carry = a & b;
        a ^= b;
        b = carry << 1;
    }
    return a;
}



static unsigned int bit_subtract(unsigned int a, unsigned int b)
{
    while (b != 0u) {
        const unsigned int borrow = ~a & b;
        a ^= b;
        b = borrow << 1;
    }
    return a;
}



Status to_power_of_two_base(const int number, const int power, char *out, const size_t capacity)
{
    const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

    if (out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (power < 1 || power > 5) {
        return STATUS_ERR_POWER_OUT_OF_RANGE;
    }


    const bool negative = number < 0;
    unsigned int magnitude = (unsigned int) number;
    if (negative) {
        magnitude = bit_add(~magnitude, 1u);
    }

    const unsigned int mask = ~(~0u << power);

    unsigned int digit_count = 0u;
    unsigned int rest = magnitude;
    do {
        digit_count = bit_add(digit_count, 1u);
        rest >>= power;
    } while (rest != 0u);

    unsigned int required = bit_add(digit_count, 1u);
    if (negative) {
        required = bit_add(required, 1u);
    }
    if ((size_t) required > capacity) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    unsigned int position = bit_subtract(required, 1u);
    out[position] = '\0';
    do {
        position = bit_subtract(position, 1u);
        out[position] = digits[magnitude & mask];
        magnitude >>= power;
    } while (magnitude != 0u);

    if (negative) {
        out[0] = '-';
    }
    return STATUS_OK;
}
