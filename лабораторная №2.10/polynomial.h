#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include "status.h"

Status shift_polynomial(const double epsilon, const double a, double **out_coefficients, int degree, ...);
Status polynomial_value(const double *coefficients, const int degree, const double x, double *out_value);

#endif
