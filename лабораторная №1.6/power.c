#include "power.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

static Status power_recursive(const double x, const long long exponent, double *out_value)
{
    if (exponent == 0) {
        *out_value = 1.0;
        return STATUS_OK;
    }

    double half = 0.0;
    const Status status = power_recursive(x, exponent / 2, &half);
    if (status != STATUS_OK) {
        return status;
    }

    double result = half * half;
    if (exponent % 2 != 0) {
        result = result * x;
    }

    if (!isfinite(result)) {
        return STATUS_ERR_OVERFLOW;
    }

    *out_value = result;
    return STATUS_OK;
}

Status fast_power(double *out_value, const double x, const int exponent)
{
    if (out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(x)) {
        return STATUS_ERR_NOT_FINITE;
    }
    if (exponent < 0 && x == 0.0) {
        return STATUS_ERR_DIVISION_BY_ZERO;
    }

    long long magnitude = (long long) exponent;
    if (magnitude < 0) {
        magnitude = -magnitude;
    }

    double result = 0.0;
    const Status status = power_recursive(x, magnitude, &result);

    if (status == STATUS_ERR_OVERFLOW && exponent < 0) {
        return STATUS_ERR_UNDERFLOW;
    }
    if (status != STATUS_OK) {
        return status;
    }

    if (exponent < 0) {
        result = 1.0 / result;
    }

    if (!isfinite(result)) {
        return STATUS_ERR_OVERFLOW;
    }
    if (x != 0.0 && fabs(result) < DBL_MIN) {
        return STATUS_ERR_UNDERFLOW;
    }

    *out_value = result;
    return STATUS_OK;
}
