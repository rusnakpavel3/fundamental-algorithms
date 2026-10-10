#ifndef VARIADIC_H
#define VARIADIC_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define KAPREKAR_MAX_VALUE 4294967295ULL

Status is_convex_polygon(bool *out_convex, const double eps, int vertex_count, ...);

Status polynomial_value(double *out_value, const double x, int degree, ...);

Status find_kaprekar(const char *out_found[], const size_t capacity,
                     size_t *out_found_count, size_t *out_bad_index,
                     const int base, int count, ...);

Status geometric_mean(double *out_value, int count, ...);

#endif
