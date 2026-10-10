#include "logic.h"

#include <float.h>
#include <math.h>
#include <stdio.h>

static bool is_zero(const double x, const double eps)
{
    return fabs(x) <= eps;
}

static double max_abs3(const double a, const double b, const double c)
{
    double m = fabs(a);
    if (fabs(b) > m) {
        m = fabs(b);
    }
    if (fabs(c) > m) {
        m = fabs(c);
    }
    return m;
}

static Status check_eps(const double eps)
{
    if (!isfinite(eps)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    if (eps <= 0.0) {
        return STATUS_ERR_EPS_NOT_POSITIVE;
    }
    if (eps >= 1.0) {
        return STATUS_ERR_EPS_TOO_LARGE;
    }
    return STATUS_OK;
}

static bool is_close(const double x, const double y, const double eps)
{
    return fabs(x - y) <= eps;
}

static bool coefficients_equal(const Coefficients *p, const Coefficients *q,
                               const double eps)
{
    return is_close(p->a, q->a, eps)
        && is_close(p->b, q->b, eps)
        && is_close(p->c, q->c, eps);
}

Status build_unique_permutations(const double a, const double b, const double c,
                                 const double eps,
                                 Coefficients out[], const size_t out_capacity,
                                 size_t *out_count)
{
    const double values[3] = {a, b, c};
    const int order[MAX_PERMUTATIONS][3] = {
        {0, 1, 2}, {0, 2, 1},
        {1, 0, 2}, {1, 2, 0},
        {2, 0, 1}, {2, 1, 0}
    };

    if (out == NULL || out_count == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(a) || !isfinite(b) || !isfinite(c)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    const Status eps_status = check_eps(eps);
    if (eps_status != STATUS_OK) {
        return eps_status;
    }
    if (out_capacity < MAX_PERMUTATIONS) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    size_t count = 0;

    for (size_t i = 0; i < MAX_PERMUTATIONS; i++) {
        Coefficients candidate;
        candidate.a = values[order[i][0]];
        candidate.b = values[order[i][1]];
        candidate.c = values[order[i][2]];

        bool duplicate = false;
        for (size_t j = 0; j < count; j++) {
            if (coefficients_equal(&candidate, &out[j], eps)) {
                duplicate = true;
                break;
            }
        }

        if (!duplicate) {
            out[count] = candidate;
            count++;
        }
    }

    *out_count = count;
    return STATUS_OK;
}

Status solve_quadratic(const Coefficients *coefs, const double eps,
                       QuadraticSolution *out)
{
    if (coefs == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(coefs->a) || !isfinite(coefs->b) || !isfinite(coefs->c)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    const Status eps_status = check_eps(eps);
    if (eps_status != STATUS_OK) {
        return eps_status;
    }

    out->x1 = 0.0;
    out->x2 = 0.0;

    if (is_zero(coefs->a, eps) && is_zero(coefs->b, eps)) {
        if (is_zero(coefs->c, eps)) {
            out->kind = QUAD_INFINITE_SOLUTIONS;
        } else {
            out->kind = QUAD_NO_SOLUTIONS;
        }
        return STATUS_OK;
    }

    const double scale = max_abs3(coefs->a, coefs->b, coefs->c);
    const double a = coefs->a / scale;
    const double b = coefs->b / scale;
    const double c = coefs->c / scale;

    if (is_zero(coefs->a, eps)) {
        const double x = -c / b;
        if (!isfinite(x)) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }
        out->kind = QUAD_LINEAR_ONE_ROOT;
        out->x1 = x;
        return STATUS_OK;
    }

    const double b_squared = b * b;
    const double four_ac = 4.0 * a * c;
    double discriminant = b_squared - four_ac;
    const double rounding_error = 8.0 * DBL_EPSILON * (b_squared + fabs(four_ac));

    if (fabs(discriminant) <= rounding_error) {
        discriminant = 0.0;
    }
    if (discriminant < 0.0) {
        out->kind = QUAD_NO_REAL_ROOTS;
        return STATUS_OK;
    }

    const double sqrt_d = sqrt(discriminant);

    if (sqrt_d / fabs(a) <= eps) {
        const double x = -b / (2.0 * a);
        if (!isfinite(x)) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }
        out->kind = QUAD_ONE_DOUBLE_ROOT;
        out->x1 = x;
        return STATUS_OK;
    }

    double q;
    if (b >= 0.0) {
        q = -(b + sqrt_d) / 2.0;
    } else {
        q = -(b - sqrt_d) / 2.0;
    }

    const double x1 = q / a;
    const double x2 = c / q;
    if (!isfinite(x1) || !isfinite(x2)) {
        return STATUS_ERR_COMPUTE_OVERFLOW;
    }

    out->kind = QUAD_TWO_ROOTS;
    out->x1 = x1;
    out->x2 = x2;
    return STATUS_OK;
}

Status check_multiple(const long long first, const long long second,
                      bool *out_is_multiple)
{
    if (out_is_multiple == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (first == 0 || second == 0) {
        return STATUS_ERR_ZERO_NOT_ALLOWED;
    }

    if (second == 1 || second == -1) {
        *out_is_multiple = true;
        return STATUS_OK;
    }

    *out_is_multiple = (first % second == 0);
    return STATUS_OK;
}

Status check_right_triangle(const double a, const double b, const double c,
                            const double eps,
                            TriangleVerdict *out_verdict,
                            int *out_side_index)
{
    if (out_verdict == NULL || out_side_index == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(a) || !isfinite(b) || !isfinite(c)) {
        return STATUS_ERR_NUMBER_NOT_FINITE;
    }
    const Status eps_status = check_eps(eps);
    if (eps_status != STATUS_OK) {
        return eps_status;
    }

    const double sides[3] = {a, b, c};
    for (int i = 0; i < 3; i++) {
        if (sides[i] <= eps) {
            *out_verdict = TRIANGLE_BAD_SIDE;
            *out_side_index = i;
            return STATUS_OK;
        }
    }

    const double scale = max_abs3(a, b, c);
    double tolerance = eps / scale;
    if (tolerance < 4.0 * DBL_EPSILON) {
        tolerance = 4.0 * DBL_EPSILON;
    }

    for (int h = 0; h < 3; h++) {
        const double hypotenuse = sides[h] / scale;
        const double leg1 = sides[(h + 1) % 3] / scale;
        const double leg2 = sides[(h + 2) % 3] / scale;

        if (is_zero(hypot(leg1, leg2) - hypotenuse, tolerance)) {
            *out_verdict = TRIANGLE_RIGHT;
            *out_side_index = h;
            return STATUS_OK;
        }
    }

    *out_verdict = TRIANGLE_NOT_RIGHT;
    *out_side_index = -1;
    return STATUS_OK;
}
