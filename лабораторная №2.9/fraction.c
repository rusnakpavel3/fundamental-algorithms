#include "fraction.h"

#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define DECIMAL_SCALE 1000000000LL
#define DENOMINATOR_LIMIT 1000000000000000LL
#define BASE_MIN 2
#define EPSILON_MIN 1e-15
#define EPSILON_MAX 1e-12


static long long gcd(long long a, long long b)
{
    while (b != 0) {
        const long long remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

static bool epsilon_is_valid(const double epsilon)
{
    return isfinite(epsilon) && epsilon >= EPSILON_MIN && epsilon <= EPSILON_MAX;
}

static bool decimal_denominator(const double x, const double epsilon, long long *out_denominator)
{
    const double nearest = round(x * (double) DECIMAL_SCALE);
    const long long numerator = (long long) nearest;

    if (numerator == 0 || numerator == DECIMAL_SCALE) {
        return false;
    }
    if (fabs(x - nearest / (double) DECIMAL_SCALE) >= epsilon) {
        return false;
    }

    *out_denominator = DECIMAL_SCALE / gcd(numerator, DECIMAL_SCALE);
    return true;
}

static Status continued_fraction_denominator(const double x, const double epsilon, long long *out_denominator)
{
    long long p_prev = 1;
    long long q_prev = 0;
    long long p = 0;
    long long q = 1;
    double rest = x;

    while (fabs(x - (double) p / (double) q) >= epsilon) {
        if (rest < DBL_MIN) {
            return STATUS_ERR_NOT_CONVERGED;
        }

        const double inverse = 1.0 / rest;
        const double whole = floor(inverse);
        rest = inverse - whole;

        if (whole > (double) (DENOMINATOR_LIMIT - q_prev) / (double) q) {
            return STATUS_ERR_NOT_CONVERGED;
        }

        const long long a = (long long) whole;
        const long long p_next = a * p + p_prev;
        const long long q_next = a * q + q_prev;

        p_prev = p;
        q_prev = q;
        p = p_next;
        q = q_next;
    }

    *out_denominator = q;
    return STATUS_OK;
}

Status fraction_kind(const double x, const int base, const double epsilon, FractionKind *out_kind)
{
    if (out_kind == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (base < BASE_MIN) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }
    if (!epsilon_is_valid(epsilon)) {
        return STATUS_ERR_EPSILON_OUT_OF_RANGE;
    }
    if (!isfinite(x)) {
        return STATUS_ERR_FRACTION_NOT_FINITE;
    }
    if (x <= epsilon || x >= 1.0 - epsilon) {
        return STATUS_ERR_FRACTION_OUT_OF_RANGE;
    }

    long long denominator = 0;
    if (!decimal_denominator(x, epsilon, &denominator)) {
        const Status status = continued_fraction_denominator(x, epsilon, &denominator);
        if (status != STATUS_OK) {
            return status;
        }
    }

    long long common = gcd(denominator, base);
    while (common > 1) {
        denominator /= common;
        common = gcd(denominator, base);
    }

    *out_kind = denominator == 1 ? FRACTION_FINITE : FRACTION_INFINITE;
    return STATUS_OK;
}

Status classify_fractions(FractionKind **out_results, const double epsilon, const int base, int count, ...)
{
    if (out_results == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_results = NULL;

    if (base < BASE_MIN) {
        return STATUS_ERR_BASE_OUT_OF_RANGE;
    }
    if (!epsilon_is_valid(epsilon)) {
        return STATUS_ERR_EPSILON_OUT_OF_RANGE;
    }
    if (count < 0) {
        return STATUS_ERR_COUNT_NEGATIVE;
    }
    if (count == 0) {
        return STATUS_OK;
    }
    if ((size_t) count > SIZE_MAX / sizeof(FractionKind)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    FractionKind *results = (FractionKind *) malloc((size_t) count * sizeof(FractionKind));
    if (results == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    Status status = STATUS_OK;
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count && status == STATUS_OK; i++) {
        const double x = va_arg(args, double);
        status = fraction_kind(x, base, epsilon, &results[i]);
    }

    va_end(args);

    if (status != STATUS_OK) {
        free(results);
        return status;
    }

    *out_results = results;
    return STATUS_OK;
}
