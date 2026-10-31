#ifndef NORMS_H
#define NORMS_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

typedef struct {
    const double *coordinates;
    size_t dimension;
} Vector;

typedef Status (*NormFunction)(const Vector *vector, const void *parameters, double *out_norm);

typedef struct {
    const double *values;
    size_t size;
    bool positive_definite;
} MatrixNorm;

Status check_vector(const Vector *vector);
Status norm_infinity(const Vector *vector, const void *parameters, double *out_norm);
Status norm_p(const Vector *vector, const void *parameters, double *out_norm);
Status norm_matrix(const Vector *vector, const void *parameters, double *out_norm);
Status matrix_norm_init(MatrixNorm *out_matrix, const double *values, const size_t size);

#endif
