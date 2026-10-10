#include "integrals.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

typedef double (*Integrand)(const double x);

static double f_a(const double x)
{
    return log(1.0 + x) / x;
}

static double f_b(const double x)
{
    return exp(-(x * x) / 2.0);
}

static double f_c(const double x)
{
    return -log(1.0 - x);
}

static double f_d(const double x)
{
    return pow(x, x);
}

static double midpoint_sum(const Integrand f, const long n)
{
    const double h = 1.0 / (double) n;
    double sum = 0.0;

    for (long i = 0; i < n; i++) {
        const double x = ((double) i + 0.5) * h;
        sum = sum + f(x);
    }
    return sum * h;
}

static Status integrate(const Integrand f, const double eps, const long max_segments,
                        double *value, long *segments_used)
{
    if (f == NULL || value == NULL || segments_used == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(eps)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    if (eps <= 0.0) {
        return STATUS_ERR_EPS_NOT_POSITIVE;
    }
    if (eps >= 1.0) {
        return STATUS_ERR_EPS_TOO_LARGE;
    }
    if (max_segments < 2) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    double previous = midpoint_sum(f, 1);
    long n = 2;
    double current = midpoint_sum(f, n);
    if (!isfinite(previous) || !isfinite(current)) {
        return STATUS_ERR_COMPUTE_OVERFLOW;
    }

    while (fabs(current - previous) >= eps / 2.0) {
        if (n > max_segments / 2) {
            return STATUS_ERR_NOT_CONVERGED;
        }
        if (sqrt(2.0 * (double) n) * DBL_EPSILON * fabs(current) >= eps / 2.0) {
            return STATUS_ERR_PRECISION_UNREACHABLE;
        }

        previous = current;
        n = n * 2;
        current = midpoint_sum(f, n);
        if (!isfinite(current)) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }
    }

    *value = current;
    *segments_used = n;
    return STATUS_OK;
}

Status integral_a(const double eps, const long max_segments, double *value, long *segments_used)
{
    return integrate(f_a, eps, max_segments, value, segments_used);
}

Status integral_b(const double eps, const long max_segments, double *value, long *segments_used)
{
    return integrate(f_b, eps, max_segments, value, segments_used);
}

Status integral_c(const double eps, const long max_segments, double *value, long *segments_used)
{
    return integrate(f_c, eps, max_segments, value, segments_used);
}

Status integral_d(const double eps, const long max_segments, double *value, long *segments_used)
{
    return integrate(f_d, eps, max_segments, value, segments_used);
}
