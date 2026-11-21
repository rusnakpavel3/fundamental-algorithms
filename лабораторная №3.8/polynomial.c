#include "polynomial.h"

#include <limits.h>
#include <stddef.h>
#include <stdlib.h>


static long long absolute(const long long value)
{
    if (value < 0) {
        return -value;
    }
    return value;
}


static bool add_checked(const long long a, const long long b, long long *out_sum)
{
    if (b > 0 && a > LLONG_MAX - b) {
        return false;
    }
    if (b < 0 && a < -LLONG_MAX - b) {
        return false;
    }
    *out_sum = a + b;
    return true;
}


static bool mul_checked(const long long a, const long long b, long long *out_product)
{
    if (a == 0 || b == 0) {
        *out_product = 0;
        return true;
    }
    if (absolute(a) > LLONG_MAX / absolute(b)) {
        return false;
    }
    *out_product = a * b;
    return true;
}


static int degree_of(const Polynomial *poly)
{
    if (poly->head == NULL) {
        return 0;
    }
    return poly->head->degree;
}


static Term *create_term(const long long coef, const int degree)
{
    Term *term = (Term *) malloc(sizeof(Term));
    if (term == NULL) {
        return NULL;
    }
    term->coef = coef;
    term->degree = degree;
    term->next = NULL;
    return term;
}


static Status append_term(Polynomial *poly, const long long coef, const int degree)
{
    Term *term = create_term(coef, degree);
    if (term == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    if (poly->last == NULL) {
        poly->head = term;
    } else {
        poly->last->next = term;
    }
    poly->last = term;
    return STATUS_OK;
}


void poly_init(Polynomial *poly)
{
    if (poly == NULL) {
        return;
    }
    poly->head = NULL;
    poly->last = NULL;
}


void poly_free(Polynomial *poly)
{
    if (poly == NULL) {
        return;
    }
    while (poly->head != NULL) {
        Term *next = poly->head->next;
        free(poly->head);
        poly->head = next;
    }
    poly->last = NULL;
}


Status poly_copy(const Polynomial *source, Polynomial *out_copy)
{
    if (source == NULL || out_copy == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Polynomial copy;
    poly_init(&copy);
    for (const Term *term = source->head; term != NULL; term = term->next) {
        const Status status = append_term(&copy, term->coef, term->degree);
        if (status != STATUS_OK) {
            poly_free(&copy);
            return status;
        }
    }
    *out_copy = copy;
    return STATUS_OK;
}


Status poly_add_term(Polynomial *poly, const long long coef, const int degree)
{
    if (poly == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (degree < 0 || degree > MAX_DEGREE) {
        return STATUS_ERR_DEGREE_TOO_LARGE;
    }
    if (coef == 0) {
        return STATUS_OK;
    }

    Term *previous = NULL;
    Term *current = poly->head;
    while (current != NULL && current->degree > degree) {
        previous = current;
        current = current->next;
    }

    if (current != NULL && current->degree == degree) {
        long long sum = 0;
        if (!add_checked(current->coef, coef, &sum)) {
            return STATUS_ERR_COEFFICIENT_OVERFLOW;
        }
        if (sum != 0) {
            current->coef = sum;
            return STATUS_OK;
        }
        if (previous == NULL) {
            poly->head = current->next;
        } else {
            previous->next = current->next;
        }
        if (poly->last == current) {
            poly->last = previous;
        }
        free(current);
        return STATUS_OK;
    }

    Term *term = create_term(coef, degree);
    if (term == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    term->next = current;
    if (previous == NULL) {
        poly->head = term;
    } else {
        previous->next = term;
    }
    if (current == NULL) {
        poly->last = term;
    }
    return STATUS_OK;
}


Status poly_add(const Polynomial *first, const Polynomial *second, Polynomial *out_result)
{
    if (first == NULL || second == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Polynomial result;
    Status status = poly_copy(first, &result);
    if (status != STATUS_OK) {
        return status;
    }
    for (const Term *term = second->head; term != NULL && status == STATUS_OK; term = term->next) {
        status = poly_add_term(&result, term->coef, term->degree);
    }
    if (status != STATUS_OK) {
        poly_free(&result);
        return status;
    }
    *out_result = result;
    return STATUS_OK;
}


Status poly_sub(const Polynomial *first, const Polynomial *second, Polynomial *out_result)
{
    if (first == NULL || second == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Polynomial result;
    Status status = poly_copy(first, &result);
    if (status != STATUS_OK) {
        return status;
    }
    for (const Term *term = second->head; term != NULL && status == STATUS_OK; term = term->next) {
        status = poly_add_term(&result, -term->coef, term->degree);
    }
    if (status != STATUS_OK) {
        poly_free(&result);
        return status;
    }
    *out_result = result;
    return STATUS_OK;
}


Status poly_mul(const Polynomial *first, const Polynomial *second, Polynomial *out_result)
{
    if (first == NULL || second == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Polynomial result;
    poly_init(&result);
    if (first->head == NULL || second->head == NULL) {
        *out_result = result;
        return STATUS_OK;
    }
    const int max_degree = degree_of(first) + degree_of(second);
    if (max_degree > MAX_DEGREE) {
        return STATUS_ERR_DEGREE_TOO_LARGE;
    }

    const size_t size = (size_t) max_degree + 1;
    long long *sums = (long long *) calloc(size, sizeof(long long));
    if (sums == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    Status status = STATUS_OK;
    for (const Term *a = first->head; a != NULL && status == STATUS_OK; a = a->next) {
        for (const Term *b = second->head; b != NULL && status == STATUS_OK; b = b->next) {
            const int degree = a->degree + b->degree;
            long long product = 0;
            if (!mul_checked(a->coef, b->coef, &product) || !add_checked(sums[degree], product, &sums[degree])) {
                status = STATUS_ERR_COEFFICIENT_OVERFLOW;
            }
        }
    }

    for (int degree = max_degree; degree >= 0 && status == STATUS_OK; degree--) {
        if (sums[degree] != 0) {
            status = append_term(&result, sums[degree], degree);
        }
    }
    free(sums);
    if (status != STATUS_OK) {
        poly_free(&result);
        return status;
    }
    *out_result = result;
    return STATUS_OK;
}


static Status subtract_shifted(Polynomial *remainder, const Polynomial *divisor, const long long factor,
                               const int shift)
{
    for (const Term *term = divisor->head; term != NULL; term = term->next) {
        long long product = 0;
        if (!mul_checked(factor, term->coef, &product)) {
            return STATUS_ERR_COEFFICIENT_OVERFLOW;
        }
        const Status status = poly_add_term(remainder, -product, term->degree + shift);
        if (status != STATUS_OK) {
            return status;
        }
    }
    return STATUS_OK;
}


Status poly_divmod(const Polynomial *dividend, const Polynomial *divisor, Polynomial *out_quotient,
                   Polynomial *out_remainder)
{
    if (dividend == NULL || divisor == NULL || out_quotient == NULL || out_remainder == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (divisor->head == NULL) {
        return STATUS_ERR_DIVISION_BY_ZERO;
    }
    Polynomial quotient;
    poly_init(&quotient);
    Polynomial remainder;
    Status status = poly_copy(dividend, &remainder);
    if (status != STATUS_OK) {
        return status;
    }

    const Term *lead = divisor->head;
    while (status == STATUS_OK && remainder.head != NULL && remainder.head->degree >= lead->degree) {
        const long long factor = remainder.head->coef / lead->coef;
        if (factor == 0) {
            break;
        }
        const int shift = remainder.head->degree - lead->degree;
        status = append_term(&quotient, factor, shift);
        if (status == STATUS_OK) {
            status = subtract_shifted(&remainder, divisor, factor, shift);
        }
    }

    if (status != STATUS_OK) {
        poly_free(&quotient);
        poly_free(&remainder);
        return status;
    }
    *out_quotient = quotient;
    *out_remainder = remainder;
    return STATUS_OK;
}


Status poly_eval(const Polynomial *poly, const long long point, long long *out_value)
{
    if (poly == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    long long value = 0;
    int degree = degree_of(poly);
    for (const Term *term = poly->head; term != NULL; term = term->next) {
        while (degree > term->degree) {
            if (!mul_checked(value, point, &value)) {
                return STATUS_ERR_VALUE_OVERFLOW;
            }
            degree--;
        }
        if (!add_checked(value, term->coef, &value)) {
            return STATUS_ERR_VALUE_OVERFLOW;
        }
    }
    while (degree > 0) {
        if (!mul_checked(value, point, &value)) {
            return STATUS_ERR_VALUE_OVERFLOW;
        }
        degree--;
    }
    *out_value = value;
    return STATUS_OK;
}


Status poly_diff(const Polynomial *poly, Polynomial *out_result)
{
    if (poly == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Polynomial result;
    poly_init(&result);
    Status status = STATUS_OK;
    for (const Term *term = poly->head; term != NULL && status == STATUS_OK; term = term->next) {
        if (term->degree > 0) {
            long long coef = 0;
            if (!mul_checked(term->coef, term->degree, &coef)) {
                status = STATUS_ERR_COEFFICIENT_OVERFLOW;
            } else {
                status = append_term(&result, coef, term->degree - 1);
            }
        }
    }
    if (status != STATUS_OK) {
        poly_free(&result);
        return status;
    }
    *out_result = result;
    return STATUS_OK;
}


static Status multiply_in_place(Polynomial *poly, const Polynomial *factor)
{
    Polynomial product;
    const Status status = poly_mul(poly, factor, &product);
    if (status != STATUS_OK) {
        return status;
    }
    poly_free(poly);
    *poly = product;
    return STATUS_OK;
}


Status poly_compose(const Polynomial *outer, const Polynomial *inner, Polynomial *out_result)
{
    if (outer == NULL || inner == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if ((long long) degree_of(outer) * degree_of(inner) > MAX_DEGREE) {
        return STATUS_ERR_DEGREE_TOO_LARGE;
    }
    Polynomial result;
    poly_init(&result);
    Status status = STATUS_OK;
    int degree = degree_of(outer);
    for (const Term *term = outer->head; term != NULL && status == STATUS_OK; term = term->next) {
        while (degree > term->degree && status == STATUS_OK) {
            status = multiply_in_place(&result, inner);
            degree--;
        }
        if (status == STATUS_OK) {
            status = poly_add_term(&result, term->coef, 0);
        }
    }
    while (degree > 0 && status == STATUS_OK) {
        status = multiply_in_place(&result, inner);
        degree--;
    }
    if (status != STATUS_OK) {
        poly_free(&result);
        return status;
    }
    *out_result = result;
    return STATUS_OK;
}
