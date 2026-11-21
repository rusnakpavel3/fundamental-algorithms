#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <stdbool.h>

#include "status.h"

#define MAX_DEGREE 10000

typedef struct Term {
    long long coef;
    int degree;
    struct Term *next;
} Term;

typedef struct {
    Term *head;
    Term *last;
} Polynomial;

void poly_init(Polynomial *poly);
void poly_free(Polynomial *poly);
Status poly_copy(const Polynomial *source, Polynomial *out_copy);
Status poly_add_term(Polynomial *poly, long long coef, int degree);

Status poly_add(const Polynomial *first, const Polynomial *second, Polynomial *out_result);
Status poly_sub(const Polynomial *first, const Polynomial *second, Polynomial *out_result);
Status poly_mul(const Polynomial *first, const Polynomial *second, Polynomial *out_result);
Status poly_divmod(const Polynomial *dividend, const Polynomial *divisor, Polynomial *out_quotient,
                   Polynomial *out_remainder);
Status poly_eval(const Polynomial *poly, long long point, long long *out_value);
Status poly_diff(const Polynomial *poly, Polynomial *out_result);
Status poly_compose(const Polynomial *outer, const Polynomial *inner, Polynomial *out_result);

#endif
