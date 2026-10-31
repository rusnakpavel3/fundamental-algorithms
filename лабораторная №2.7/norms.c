#include "norms.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static double max_abs(const Vector *vector)
{
    double max = 0.0;

    for (size_t j = 0; j < vector->dimension; j++) {
        max = fmax(max, fabs(vector->coordinates[j]));
    }
    return max;
}

Status check_vector(const Vector *vector)
{
    if (vector == NULL || vector->coordinates == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (vector->dimension == 0) {
        return STATUS_ERR_DIMENSION_INVALID;
    }

    for (size_t j = 0; j < vector->dimension; j++) {
        if (!isfinite(vector->coordinates[j])) {
            return STATUS_ERR_COORDINATE_NOT_FINITE;
        }
    }
    return STATUS_OK;
}

Status norm_infinity(const Vector *vector, const void *parameters, double *out_norm)
{
    (void) parameters;

    if (out_norm == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const Status status = check_vector(vector);
    if (status != STATUS_OK) {
        return status;
    }

    *out_norm = max_abs(vector);
    return STATUS_OK;
}

Status norm_p(const Vector *vector, const void *parameters, double *out_norm)
{
    if (parameters == NULL || out_norm == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const double p = *(const double *) parameters;
    if (!isfinite(p) || p < 1.0) {
        return STATUS_ERR_P_INVALID;
    }

    const Status status = check_vector(vector);
    if (status != STATUS_OK) {
        return status;
    }

    const double max = max_abs(vector);
    if (max == 0.0) {
        *out_norm = 0.0;
        return STATUS_OK;
    }

    double sum = 0.0;
    for (size_t j = 0; j < vector->dimension; j++) {
        sum += pow(fabs(vector->coordinates[j]) / max, p);
    }

    const double norm = max * pow(sum, 1.0 / p);
    if (!isfinite(norm)) {
        return STATUS_ERR_NORM_OVERFLOW;
    }

    *out_norm = norm;
    return STATUS_OK;
}

Status norm_matrix(const Vector *vector, const void *parameters, double *out_norm)
{
    if (parameters == NULL || out_norm == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const MatrixNorm *matrix = (const MatrixNorm *) parameters;
    if (!matrix->positive_definite) {
        return STATUS_ERR_MATRIX_NOT_POSITIVE_DEFINITE;
    }
    if (matrix->values == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const Status status = check_vector(vector);
    if (status != STATUS_OK) {
        return status;
    }
    if (matrix->size != vector->dimension) {
        return STATUS_ERR_DIMENSION_MISMATCH;
    }

    const size_t n = vector->dimension;
    const double max = max_abs(vector);
    if (max == 0.0) {
        *out_norm = 0.0;
        return STATUS_OK;
    }

    double product = 0.0;
    for (size_t i = 0; i < n; i++) {
        const double x_i = vector->coordinates[i] / max;
        for (size_t j = 0; j < n; j++) {
            const double x_j = vector->coordinates[j] / max;
            product += matrix->values[i * n + j] * x_i * x_j;
        }
    }

    if (!isfinite(product)) {
        return STATUS_ERR_NORM_OVERFLOW;
    }
    if (product < 0.0) {
        return STATUS_ERR_MATRIX_NOT_POSITIVE_DEFINITE;
    }

    const double norm = max * sqrt(product);
    if (!isfinite(norm)) {
        return STATUS_ERR_NORM_OVERFLOW;
    }

    *out_norm = norm;
    return STATUS_OK;
}

static double symmetric_part(const double *values, const size_t n, const size_t i, const size_t j)
{
    return values[i * n + j] / 2.0 + values[j * n + i] / 2.0;
}

static Status cholesky_check(const double *values, const size_t n, double *lower)
{
    for (size_t j = 0; j < n; j++) {
        double diagonal = symmetric_part(values, n, j, j);
        for (size_t k = 0; k < j; k++) {
            diagonal -= lower[j * n + k] * lower[j * n + k];
        }

        if (!isfinite(diagonal)) {
            return STATUS_ERR_MATRIX_INVALID;
        }
        if (diagonal <= 0.0) {
            return STATUS_ERR_MATRIX_NOT_POSITIVE_DEFINITE;
        }
        lower[j * n + j] = sqrt(diagonal);

        for (size_t i = j + 1; i < n; i++) {
            double value = symmetric_part(values, n, i, j);
            for (size_t k = 0; k < j; k++) {
                value -= lower[i * n + k] * lower[j * n + k];
            }

            lower[i * n + j] = value / lower[j * n + j];
            if (!isfinite(lower[i * n + j])) {
                return STATUS_ERR_MATRIX_INVALID;
            }
        }
    }
    return STATUS_OK;
}

Status matrix_norm_init(MatrixNorm *out_matrix, const double *values, const size_t size)
{
    if (out_matrix == NULL || values == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    out_matrix->values = NULL;
    out_matrix->size = 0;
    out_matrix->positive_definite = false;

    if (size == 0) {
        return STATUS_ERR_DIMENSION_INVALID;
    }
    if (size > SIZE_MAX / size || size * size > SIZE_MAX / sizeof(double)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    for (size_t i = 0; i < size * size; i++) {
        if (!isfinite(values[i])) {
            return STATUS_ERR_MATRIX_INVALID;
        }
    }

    double *lower = (double *) malloc(size * size * sizeof(double));
    if (lower == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    const Status status = cholesky_check(values, size, lower);
    free(lower);
    if (status != STATUS_OK) {
        return status;
    }

    out_matrix->values = values;
    out_matrix->size = size;
    out_matrix->positive_definite = true;
    return STATUS_OK;
}
