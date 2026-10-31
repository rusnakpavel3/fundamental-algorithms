#include "polynomial.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static bool epsilon_is_valid(const double epsilon)
{
    return isfinite(epsilon) && epsilon > 0.0 && epsilon < 1.0;
}

static Status horner_shift(double *coefficients, const size_t degree, const double a)
{
    for (size_t k = 0; k < degree; k++) {
        for (size_t i = degree; i > k; i--) {
            coefficients[i - 1] += a * coefficients[i];
            if (!isfinite(coefficients[i - 1])) {
                return STATUS_ERR_RESULT_OVERFLOW;
            }
        }
    }
    return STATUS_OK;
}

static Status make_scales(const double *coefficients, const size_t length, const double a, double **out_scales)
{
    double *scales = malloc(length * sizeof(double));
    if (scales == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    for (size_t i = 0; i < length; i++) {
        scales[i] = fabs(coefficients[i]);
    }

    if (horner_shift(scales, length - 1, fabs(a)) != STATUS_OK) {
        free(scales);
        scales = NULL;
    }

    *out_scales = scales;
    return STATUS_OK;
}

static void clean_noise(double *coefficients, const double *scales, const size_t length, const double epsilon)
{
    for (size_t i = 0; i < length; i++) {
        if (fabs(coefficients[i]) <= epsilon * scales[i]) {
            coefficients[i] = 0.0;
        }
    }
}

Status shift_polynomial(const double epsilon, const double a, double **out_coefficients, int degree, ...)
{
    if (out_coefficients == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_coefficients = NULL;

    if (!epsilon_is_valid(epsilon)) {
        return STATUS_ERR_EPSILON_OUT_OF_RANGE;
    }
    if (!isfinite(a)) {
        return STATUS_ERR_POINT_NOT_FINITE;
    }
    if (degree < 0) {
        return STATUS_ERR_DEGREE_NEGATIVE;
    }

    const size_t length = (size_t) degree + 1;
    if (length > SIZE_MAX / sizeof(double)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    double *coefficients = malloc(length * sizeof(double));
    if (coefficients == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    Status status = STATUS_OK;
    va_list args;
    va_start(args, degree);

    for (size_t i = 0; i < length && status == STATUS_OK; i++) {
        const double value = va_arg(args, double);
        if (isfinite(value)) {
            coefficients[i] = value;
        } else {
            status = STATUS_ERR_COEFFICIENT_NOT_FINITE;
        }
    }

    va_end(args);

    double *scales = NULL;
    if (status == STATUS_OK) {
        status = make_scales(coefficients, length, a, &scales);
    }
    if (status == STATUS_OK) {
        status = horner_shift(coefficients, (size_t) degree, a);
    }
    if (status == STATUS_OK && scales != NULL) {
        clean_noise(coefficients, scales, length, epsilon);
    }

    free(scales);

    if (status != STATUS_OK) {
        free(coefficients);
        return status;
    }

    *out_coefficients = coefficients;
    return STATUS_OK;
}

Status polynomial_value(const double *coefficients, const int degree, const double x, double *out_value)
{
    if (coefficients == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (degree < 0) {
        return STATUS_ERR_DEGREE_NEGATIVE;
    }
    if (!isfinite(x)) {
        return STATUS_ERR_POINT_NOT_FINITE;
    }

    double value = 0.0;
    for (int i = degree; i >= 0; i--) {
        if (!isfinite(coefficients[i])) {
            return STATUS_ERR_COEFFICIENT_NOT_FINITE;
        }
        value = value * x + coefficients[i];
        if (!isfinite(value)) {
            return STATUS_ERR_RESULT_OVERFLOW;
        }
    }

    *out_value = value;
    return STATUS_OK;
}
