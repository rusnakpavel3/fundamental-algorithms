#include "variadic.h"

#include <limits.h>
#include <math.h>
#include <stdarg.h>

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    int left_turns;
    int right_turns;
    double total_angle;
} TurnStats;

static Status read_point(va_list *args, Point *out)
{
    out->x = va_arg(*args, double);
    out->y = va_arg(*args, double);

    if (!isfinite(out->x) || !isfinite(out->y)) {
        return STATUS_ERR_NOT_FINITE;
    }
    return STATUS_OK;
}

static Status unit_direction(const Point *from, const Point *to, double *out_x, double *out_y)
{
    double dx = to->x - from->x;
    double dy = to->y - from->y;

    if (!isfinite(dx) || !isfinite(dy)) {
        dx = to->x / 2.0 - from->x / 2.0;
        dy = to->y / 2.0 - from->y / 2.0;
    }

    const double largest = fmax(fabs(dx), fabs(dy));
    if (largest <= 0.0) {
        return STATUS_ERR_POLYGON_DEGENERATE;
    }

    dx = dx / largest;
    dy = dy / largest;

    const double length = hypot(dx, dy);
    *out_x = dx / length;
    *out_y = dy / length;
    return STATUS_OK;
}

static Status add_turn(const Point *a, const Point *b, const Point *c,
                       const double eps, TurnStats *stats)
{
    double u1x = 0.0;
    double u1y = 0.0;
    double u2x = 0.0;
    double u2y = 0.0;

    Status status = unit_direction(a, b, &u1x, &u1y);
    if (status != STATUS_OK) {
        return status;
    }
    status = unit_direction(b, c, &u2x, &u2y);
    if (status != STATUS_OK) {
        return status;
    }

    const double sine = u1x * u2y - u1y * u2x;
    const double cosine = u1x * u2x + u1y * u2y;

    if (fabs(sine) <= eps) {
        if (cosine < 0.0) {
            return STATUS_ERR_POLYGON_DEGENERATE;
        }
    } else if (sine > 0.0) {
        stats->left_turns++;
    } else {
        stats->right_turns++;
    }

    stats->total_angle += atan2(sine, cosine);
    return STATUS_OK;
}

Status is_convex_polygon(bool *out_convex, const double eps, int vertex_count, ...)
{
    if (out_convex == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(eps) || eps <= 0.0 || eps >= 1.0) {
        return STATUS_ERR_EPS_INVALID;
    }
    if (vertex_count < 3) {
        return STATUS_ERR_COUNT_INVALID;
    }

    va_list args;
    va_start(args, vertex_count);

    Point first = {0.0, 0.0};
    Point second = {0.0, 0.0};
    Point current = {0.0, 0.0};
    TurnStats stats = {0, 0, 0.0};

    Status status = read_point(&args, &first);
    if (status == STATUS_OK) {
        status = read_point(&args, &second);
    }

    Point before = first;
    Point last = second;

    for (int i = 2; i < vertex_count && status == STATUS_OK; i++) {
        status = read_point(&args, &current);
        if (status == STATUS_OK) {
            status = add_turn(&before, &last, &current, eps, &stats);
        }
        before = last;
        last = current;
    }

    va_end(args);

    if (status != STATUS_OK) {
        return status;
    }

    status = add_turn(&before, &last, &first, eps, &stats);
    if (status != STATUS_OK) {
        return status;
    }
    status = add_turn(&last, &first, &second, eps, &stats);
    if (status != STATUS_OK) {
        return status;
    }

    if (stats.left_turns == 0 && stats.right_turns == 0) {
        return STATUS_ERR_POLYGON_DEGENERATE;
    }

    const double full_turn = 2.0 * acos(-1.0);
    const bool one_direction = (stats.left_turns == 0 || stats.right_turns == 0);
    const bool one_revolution = fabs(stats.total_angle) < 1.5 * full_turn;

    *out_convex = one_direction && one_revolution;
    return STATUS_OK;
}

Status polynomial_value(double *out_value, const double x, int degree, ...)
{
    if (out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!isfinite(x)) {
        return STATUS_ERR_NOT_FINITE;
    }
    if (degree < 0) {
        return STATUS_ERR_COUNT_INVALID;
    }

    va_list args;
    va_start(args, degree);

    double result = 0.0;
    Status status = STATUS_OK;

    for (long long i = 0; i <= (long long) degree; i++) {
        const double coefficient = va_arg(args, double);

        if (!isfinite(coefficient)) {
            status = STATUS_ERR_NOT_FINITE;
            break;
        }

        result = result * x + coefficient;

        if (!isfinite(result)) {
            status = STATUS_ERR_OVERFLOW;
            break;
        }
    }

    va_end(args);

    if (status != STATUS_OK) {
        return status;
    }

    *out_value = result;
    return STATUS_OK;
}

static int digit_value(const char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    return -1;
}

static Status parse_in_base(const char *str, const int base, unsigned long long *out_value)
{
    if (str == NULL || str[0] == '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }

    const unsigned long long b = (unsigned long long) base;
    unsigned long long value = 0;

    for (size_t i = 0; str[i] != '\0'; i++) {
        const int digit = digit_value(str[i]);

        if (digit < 0 || digit >= base) {
            return STATUS_ERR_NUMBER_INVALID;
        }

        const unsigned long long d = (unsigned long long) digit;
        if (value > (KAPREKAR_MAX_VALUE - d) / b) {
            return STATUS_ERR_NUMBER_TOO_BIG;
        }
        value = value * b + d;
    }

    *out_value = value;
    return STATUS_OK;
}

static bool is_kaprekar(const unsigned long long n, const int base)
{
    if (n == 0) {
        return false;
    }

    const unsigned long long b = (unsigned long long) base;
    const unsigned long long square = n * n;
    unsigned long long divisor = b;

    while (true) {
        const unsigned long long left = square / divisor;
        const unsigned long long right = square % divisor;

        if (right > 0 && left + right == n) {
            return true;
        }
        if (left == 0 || divisor > ULLONG_MAX / b) {
            return false;
        }
        divisor = divisor * b;
    }
}

Status find_kaprekar(const char *out_found[], const size_t capacity,
                     size_t *out_found_count, size_t *out_bad_index,
                     const int base, int count, ...)
{
    if (out_found == NULL || out_found_count == NULL || out_bad_index == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (base < 2 || base > 36) {
        return STATUS_ERR_BASE_INVALID;
    }
    if (count < 0) {
        return STATUS_ERR_COUNT_INVALID;
    }
    if ((size_t) count > capacity) {
        return STATUS_ERR_CAPACITY_TOO_SMALL;
    }

    va_list args;
    va_start(args, count);

    size_t found = 0;
    Status status = STATUS_OK;

    for (int i = 0; i < count; i++) {
        const char *text = va_arg(args, const char *);
        unsigned long long value = 0;

        status = parse_in_base(text, base, &value);
        if (status != STATUS_OK) {
            *out_bad_index = (size_t) i;
            break;
        }

        if (is_kaprekar(value, base)) {
            out_found[found] = text;
            found++;
        }
    }

    va_end(args);

    if (status != STATUS_OK) {
        return status;
    }

    *out_found_count = found;
    return STATUS_OK;
}

Status geometric_mean(double *out_value, int count, ...)
{
    if (out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (count <= 0) {
        return STATUS_ERR_COUNT_INVALID;
    }

    va_list args;
    va_start(args, count);

    double log_sum = 0.0;
    bool has_zero = false;
    Status status = STATUS_OK;

    for (int i = 0; i < count; i++) {
        const double value = va_arg(args, double);

        if (!isfinite(value)) {
            status = STATUS_ERR_NOT_FINITE;
            break;
        }
        if (value < 0.0) {
            status = STATUS_ERR_NEGATIVE_NUMBER;
            break;
        }

        if (value > 0.0) {
            log_sum += log(value);
        } else {
            has_zero = true;
        }
    }

    va_end(args);

    if (status != STATUS_OK) {
        return status;
    }

    if (has_zero) {
        *out_value = 0.0;
    } else {
        *out_value = exp(log_sum / (double) count);
    }
    return STATUS_OK;
}
