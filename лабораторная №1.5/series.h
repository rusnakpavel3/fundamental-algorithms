#ifndef SERIES_H
#define SERIES_H

#include "status.h"

Status series_a(const double x, const double eps, const long max_terms,
                double *value, long *terms_used);

Status series_b(const double x, const double eps, const long max_terms,
                double *value, long *terms_used);

Status series_c(const double x, const double eps, const long max_terms,
                double *value, long *terms_used);

Status series_d(const double x, const double eps, const long max_terms,
                double *value, long *terms_used);

#endif
