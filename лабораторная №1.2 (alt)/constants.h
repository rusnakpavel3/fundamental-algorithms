#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "status.h"

#define MIN_EPS 1e-14

Status validate_eps(const double eps);

Status e_limit(const double eps, double *out_value, long *out_steps);
Status e_series(const double eps, double *out_value, long *out_steps);
Status e_equation(const double eps, double *out_value, long *out_steps);

Status pi_limit(const double eps, double *out_value, long *out_steps);
Status pi_series(const double eps, double *out_value, long *out_steps);
Status pi_equation(const double eps, double *out_value, long *out_steps);

Status ln2_limit(const double eps, double *out_value, long *out_steps);
Status ln2_series(const double eps, double *out_value, long *out_steps);
Status ln2_equation(const double eps, double *out_value, long *out_steps);

Status sqrt2_limit(const double eps, double *out_value, long *out_steps);
Status sqrt2_product(const double eps, double *out_value, long *out_steps);
Status sqrt2_equation(const double eps, double *out_value, long *out_steps);

Status gamma_limit(const double eps, double *out_value, long *out_steps);
Status gamma_series(const double eps, double *out_value, long *out_steps);
Status gamma_equation(const double eps, double *out_value, long *out_steps);

#endif
