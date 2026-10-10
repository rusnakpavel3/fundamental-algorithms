#include "series.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

typedef double (*RatioFunction)(const double x, const long n);

static double ratio_a(const double x, const long n)
{
    const double k = (double) n;
    return x / (k + 1.0);
}

static double ratio_b(const double x, const long n)
{
    const double k = (double) n;
    return -(x * x) / ((2.0 * k + 1.0) * (2.0 * k + 2.0));
}

static double ratio_c(const double x, const long n)
{
    const double k = (double) n;
    return 9.0 * (k + 1.0) * (k + 1.0) * x * x / ((3.0 * k + 1.0) * (3.0 * k + 2.0));
}

static double ratio_d(const double x, const long n)
{
    const double k = (double) n;
    return -(x * x) * (2.0 * k + 1.0) / (2.0 * k + 2.0);
}

static Status check_common_args(const double x, const double eps, const long max_terms,
                                const double *value, const long *terms_used)
{
    if (value == NULL || terms_used == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(eps) || !isfinite(x)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    if (eps <= 0.0) {
        return STATUS_ERR_EPS_NOT_POSITIVE;
    }
    if (eps >= 1.0) {
        return STATUS_ERR_EPS_TOO_LARGE;
    }
    if (max_terms < 1) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }
    return STATUS_OK;
}

static Status sum_series(const double first_term, const long first_index,
                         const RatioFunction ratio, const double x,
                         const double eps, const long max_terms,
                         double *value, long *terms_used)
{
    double term = first_term;
    long n = first_index;
    double sum = 0.0;
    double abs_sum = 0.0;

    for (long count = 1; count <= max_terms; count++) {
        sum = sum + term;
        abs_sum = abs_sum + fabs(term);
        if (!isfinite(sum) || !isfinite(abs_sum)) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }

        term = term * ratio(x, n);
        n++;
        if (!isfinite(term)) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }

        const double next_q = ratio(x, n);
        if (fabs(next_q) < 1.0) {
            double tail = fabs(term);
            if (next_q > 0.0) {
                tail = fabs(term) / (1.0 - next_q);
            }

            if (tail < eps / 2.0) {
                if (sqrt((double) count) * DBL_EPSILON * abs_sum >= eps / 2.0) {
                    return STATUS_ERR_PRECISION_UNREACHABLE;
                }
                *value = sum;
                *terms_used = count;
                return STATUS_OK;
            }
        }
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status series_a(const double x, const double eps, const long max_terms,
                double *value, long *terms_used)
{
    const Status status = check_common_args(x, eps, max_terms, value, terms_used);
    if (status != STATUS_OK) {
        return status;
    }
    return sum_series(1.0, 0, ratio_a, x, eps, max_terms, value, terms_used);
}

Status series_b(const double x, const double eps, const long max_terms,
                double *value, long *terms_used)
{
    const Status status = check_common_args(x, eps, max_terms, value, terms_used);
    if (status != STATUS_OK) {
        return status;
    }
    return sum_series(1.0, 0, ratio_b, x, eps, max_terms, value, terms_used);
}

Status series_c(const double x, const double eps, const long max_terms,
                double *value, long *terms_used)
{
    const Status status = check_common_args(x, eps, max_terms, value, terms_used);
    if (status != STATUS_OK) {
        return status;
    }
    if (fabs(x) >= 1.0) {
        return STATUS_ERR_SERIES_DIVERGES;
    }
    return sum_series(1.0, 0, ratio_c, x, eps, max_terms, value, terms_used);
}

Status series_d(const double x, const double eps, const long max_terms,
                double *value, long *terms_used)
{
    const Status status = check_common_args(x, eps, max_terms, value, terms_used);
    if (status != STATUS_OK) {
        return status;
    }
    if (fabs(x) > 1.0) {
        return STATUS_ERR_SERIES_DIVERGES;
    }
    return sum_series(-(x * x) / 2.0, 1, ratio_d, x, eps, max_terms, value, terms_used);
}
