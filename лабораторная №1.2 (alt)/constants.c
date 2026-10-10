#include "constants.h"
#include <math.h>
#include <stdlib.h>

typedef double (*MathFunc)(double x);

Status validate_eps(const double eps)
{
    if (isnan(eps) || isinf(eps) || eps < MIN_EPS || eps >= 1.0) {
        return STATUS_ERR_EPS_INVALID;
    }
    return STATUS_OK;
}

static Status check_args(const double eps, const double *out_value, const long *out_steps)
{
    if (out_value == NULL || out_steps == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    return validate_eps(eps);
}

static Status bisection(const MathFunc f, const double a, const double b, const double eps,
                        double *out_root, long *out_steps)
{
    const long max_steps = 200;

    if (f == NULL || out_root == NULL || out_steps == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (isnan(eps) || eps <= 0.0) {
        return STATUS_ERR_EPS_INVALID;
    }
    if (a >= b) {
        return STATUS_ERR_INVALID_INTERVAL;
    }

    double left = a;
    double right = b;
    double f_left = f(left);

    if (f_left * f(right) > 0.0) {
        return STATUS_ERR_INVALID_INTERVAL;
    }

    long steps = 0;
    while (right - left >= eps) {
        if (steps >= max_steps) {
            return STATUS_ERR_NOT_CONVERGED;
        }

        double mid = left + (right - left) / 2.0;
        double f_mid = f(mid);

        if (f_left * f_mid <= 0.0) {
            right = mid;
        } else {
            left = mid;
            f_left = f_mid;
        }
        steps++;
    }

    *out_root = left + (right - left) / 2.0;
    *out_steps = steps;
    return STATUS_OK;
}

Status e_limit(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 52;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double n = 1.0;
    double prev = 2.0;

    for (long k = 1; k <= max_steps; k++) {
        n = n * 2.0;
        double curr = exp(n * log1p(1.0 / n));

        if (2.0 * fabs(curr - prev) < eps) {
            *out_value = curr;
            *out_steps = k;
            return STATUS_OK;
        }
        prev = curr;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status e_series(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 100;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double sum = 1.0;
    double term = 1.0;

    for (long n = 1; n <= max_steps; n++) {
        term = term / (double) n;
        sum = sum + term;

        if (term < eps) {
            *out_value = sum;
            *out_steps = n;
            return STATUS_OK;
        }
    }
    return STATUS_ERR_NOT_CONVERGED;
}

static double e_equation_f(double x)
{
    return log(x) - 1.0;
}

Status e_equation(const double eps, double *out_value, long *out_steps)
{
    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }
    return bisection(e_equation_f, 1.0, 4.0, eps, out_value, out_steps);
}

Status pi_limit(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 20000000L;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double p = 4.0;

    for (long n = 1; n <= max_steps; n++) {
        double dn = (double) n;

        if (p / (2.0 * dn) < eps) {
            *out_value = p;
            *out_steps = n;
            return STATUS_OK;
        }
        p = p * (4.0 * dn * (dn + 1.0)) / ((2.0 * dn + 1.0) * (2.0 * dn + 1.0));
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status pi_series(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 20000000L;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double sum = 0.0;
    double sign = 1.0;

    for (long n = 1; n <= max_steps; n++) {
        double term = 4.0 / (2.0 * (double) n - 1.0);
        sum = sum + sign * term;

        if (term < eps) {
            *out_value = sum;
            *out_steps = n;
            return STATUS_OK;
        }
        sign = -sign;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

static double pi_equation_f(double x)
{
    return cos(x / 2.0);
}

Status pi_equation(const double eps, double *out_value, long *out_steps)
{
    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }
    return bisection(pi_equation_f, 2.0, 4.0, eps, out_value, out_steps);
}

Status ln2_limit(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 60;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double n = 1.0;
    double d = 1.0;
    double prev = 1.0;

    for (long k = 1; k <= max_steps; k++) {
        n = n * 2.0;
        d = d / (1.0 + sqrt(1.0 + d));
        double curr = n * d;

        if (2.0 * fabs(curr - prev) < eps) {
            *out_value = curr;
            *out_steps = k;
            return STATUS_OK;
        }
        prev = curr;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status ln2_series(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 10000000L;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double sum = 0.0;
    double sign = 1.0;

    for (long n = 1; n <= max_steps; n++) {
        double term = 1.0 / (double) n;
        sum = sum + sign * term;

        if (term < eps) {
            *out_value = sum;
            *out_steps = n;
            return STATUS_OK;
        }
        sign = -sign;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

static double ln2_equation_f(double x)
{
    return exp(x) - 2.0;
}

Status ln2_equation(const double eps, double *out_value, long *out_steps)
{
    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }
    return bisection(ln2_equation_f, 0.0, 1.0, eps, out_value, out_steps);
}

Status sqrt2_limit(const double eps, double *out_value, long *out_steps)
{
    const long max_steps = 1000;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double x = -0.5;

    for (long n = 1; n <= max_steps; n++) {
        double next = x - x * x / 2.0 + 1.0;

        if (n >= 3 && fabs(next - x) < eps) {
            *out_value = next;
            *out_steps = n;
            return STATUS_OK;
        }
        x = next;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status sqrt2_product(const double eps, double *out_value, long *out_steps)
{
    const long max_k = 100;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double product = 1.0;
    double exponent = 0.5;

    for (long k = 2; k <= max_k; k++) {
        exponent = exponent / 2.0;
        double next = product * pow(2.0, exponent);

        if (2.0 * fabs(next - product) < eps) {
            *out_value = next;
            *out_steps = k - 1;
            return STATUS_OK;
        }
        product = next;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

static double sqrt2_equation_f(double x)
{
    return x * x - 2.0;
}

Status sqrt2_equation(const double eps, double *out_value, long *out_steps)
{
    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }
    return bisection(sqrt2_equation_f, 1.0, 2.0, eps, out_value, out_steps);
}

Status gamma_limit(const double eps, double *out_value, long *out_steps)
{
    const long max_m = 36;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    double prev = 0.0;

    for (long m = 1; m <= max_m; m++) {
        double sum = 0.0;
        double binom = 1.0;
        double log_fact = 0.0;
        double sign = -1.0;

        for (long k = 1; k <= m; k++) {
            binom = binom * (double) (m - k + 1) / (double) k;
            log_fact = log_fact + log((double) k);
            sum = sum + sign * binom / (double) k * log_fact;
            sign = -sign;
        }

        if (m >= 2 && 2.0 * (double) m * fabs(sum - prev) < eps) {
            *out_value = sum;
            *out_steps = m;
            return STATUS_OK;
        }
        prev = sum;
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status gamma_series(const double eps, double *out_value, long *out_steps)
{
    const long max_j = 4000;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    const double pi = acos(-1.0);
    double sum = -(pi * pi) / 6.0;
    long steps = 0;

    for (long j = 1; j <= max_j; j++) {
        double inv_square = 1.0 / ((double) j * (double) j);
        long first = (j == 1) ? 2 : j * j;
        long last = (j + 1) * (j + 1) - 1;

        for (long k = first; k <= last; k++) {
            sum = sum + inv_square - 1.0 / (double) k;
            steps++;
        }

        if (2.0 / (double) j < eps) {
            *out_value = sum;
            *out_steps = steps;
            return STATUS_OK;
        }
    }
    return STATUS_ERR_NOT_CONVERGED;
}

Status gamma_equation(const double eps, double *out_value, long *out_steps)
{
    const long t_min = 512;
    const long t_max = 16777216L;

    Status status = check_args(eps, out_value, out_steps);
    if (status != STATUS_OK) {
        return status;
    }

    long t = t_min;
    double ln_t = log((double) t);

    while (1.0 / (ln_t * ln_t) >= eps) {
        if (t >= t_max) {
            return STATUS_ERR_NOT_CONVERGED;
        }
        t = t * 2;
        ln_t = log((double) t);
    }

    char *is_composite = calloc((size_t) t + 1, sizeof(char));
    if (is_composite == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    double product = 1.0;
    long primes = 0;

    for (long i = 2; i <= t; i++) {
        if (is_composite[i] == 0) {
            product = product * ((double) i - 1.0) / (double) i;
            primes++;

            if (i <= t / i) {
                for (long j = i * i; j <= t; j += i) {
                    is_composite[j] = 1;
                }
            }
        }
    }

    free(is_composite);

    *out_value = -log(ln_t * product);
    *out_steps = primes;
    return STATUS_OK;
}
