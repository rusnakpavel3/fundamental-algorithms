#ifndef BISECTION_H
#define BISECTION_H

#include "status.h"

typedef double (*RealFunction)(double x);

Status bisection(double *out_root, const RealFunction f,
                 const double a, const double b, const double eps);

#endif
