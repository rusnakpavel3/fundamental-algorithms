#ifndef LOGIC_H
#define LOGIC_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define MAX_PERMUTATIONS 6

typedef struct {
    double a;
    double b;
    double c;
} Coefficients;

typedef enum {
    QUAD_TWO_ROOTS,
    QUAD_ONE_DOUBLE_ROOT,
    QUAD_NO_REAL_ROOTS,
    QUAD_LINEAR_ONE_ROOT,
    QUAD_NO_SOLUTIONS,
    QUAD_INFINITE_SOLUTIONS
} QuadraticKind;

typedef struct {
    QuadraticKind kind;
    double x1;
    double x2;
} QuadraticSolution;

typedef enum {
    TRIANGLE_RIGHT,
    TRIANGLE_NOT_RIGHT,
    TRIANGLE_BAD_SIDE
} TriangleVerdict;

Status build_unique_permutations(const double a, const double b, const double c,
                                 const double eps,
                                 Coefficients out[], const size_t out_capacity,
                                 size_t *out_count);

Status solve_quadratic(const Coefficients *coefs, const double eps,
                       QuadraticSolution *out);

Status check_multiple(const long long first, const long long second,
                      bool *out_is_multiple);

Status check_right_triangle(const double a, const double b, const double c,
                            const double eps,
                            TriangleVerdict *out_verdict,
                            int *out_side_index);

#endif
