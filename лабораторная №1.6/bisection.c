#include "bisection.h"

#include <math.h>
#include <stddef.h>

Status bisection(double *out_root, const RealFunction f,
                 const double a, const double b, const double eps)
{
    if (out_root == NULL || f == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(a) || !isfinite(b)) {
        return STATUS_ERR_NOT_FINITE;
    }
    if (!isfinite(eps) || eps <= 0.0) {
        return STATUS_ERR_EPS_INVALID;
    }
    if (a >= b) {
        return STATUS_ERR_INVALID_INTERVAL;
    }

    double left = a;
    double right = b;
    double f_left = f(left);
    const double f_right = f(right);

    if (!isfinite(f_left) || !isfinite(f_right)) {
        return STATUS_ERR_FUNCTION_NOT_FINITE;
    }
    if (f_left == 0.0) {
        *out_root = left;
        return STATUS_OK;
    }
    if (f_right == 0.0) {
        *out_root = right;
        return STATUS_OK;
    }
    if ((f_left > 0.0) == (f_right > 0.0)) {
        return STATUS_ERR_NO_SIGN_CHANGE;
    }

    while (right / 2.0 - left / 2.0 > eps) {
        const double middle = left / 2.0 + right / 2.0;

        if (middle <= left || middle >= right) {
            return STATUS_ERR_PRECISION_UNREACHABLE;
        }

        const double f_middle = f(middle);

        if (!isfinite(f_middle)) {
            return STATUS_ERR_FUNCTION_NOT_FINITE;
        }
        if (f_middle == 0.0) {
            *out_root = middle;
            return STATUS_OK;
        }

        if ((f_middle > 0.0) == (f_left > 0.0)) {
            left = middle;
            f_left = f_middle;
        } else {
            right = middle;
        }
    }

    *out_root = left / 2.0 + right / 2.0;
    return STATUS_OK;
}
