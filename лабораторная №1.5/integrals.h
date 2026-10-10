#ifndef INTEGRALS_H
#define INTEGRALS_H

#include "status.h"

Status integral_a(const double eps, const long max_segments, double *value, long *segments_used);

Status integral_b(const double eps, const long max_segments, double *value, long *segments_used);

Status integral_c(const double eps, const long max_segments, double *value, long *segments_used);

Status integral_d(const double eps, const long max_segments, double *value, long *segments_used);

#endif
