#include "poly_parse.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>


static bool is_space(const char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static bool is_letter(const char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


static void skip_spaces(const char *text, size_t *pos)
{
    while (is_space(text[*pos])) {
        (*pos)++;
    }
}


static Status read_digits(const char *text, size_t *pos, const size_t max_digits, long long *out_value)
{
    size_t count = 0;
    while (is_digit(text[*pos + count])) {
        count++;
    }
    if (count == 0) {
        return STATUS_ERR_BAD_POLYNOMIAL;
    }
    if (count > max_digits) {
        return STATUS_ERR_NUMBER_TOO_LONG;
    }
    char *end = NULL;
    const long long value = strtoll(&text[*pos], &end, 10);
    if (end != &text[*pos + count]) {
        return STATUS_ERR_BAD_POLYNOMIAL;
    }
    *pos += count;
    *out_value = value;
    return STATUS_OK;
}


static Status read_degree(const char *text, size_t *pos, int *out_degree)
{
    if (text[*pos] != '^') {
        *out_degree = 1;
        return STATUS_OK;
    }
    (*pos)++;
    long long degree = 0;
    const Status status = read_digits(text, pos, MAX_DEGREE_DIGITS, &degree);
    if (status == STATUS_ERR_NUMBER_TOO_LONG || (status == STATUS_OK && degree > MAX_DEGREE)) {
        return STATUS_ERR_DEGREE_TOO_LARGE;
    }
    if (status != STATUS_OK) {
        return status;
    }
    *out_degree = (int) degree;
    return STATUS_OK;
}


static Status read_term(const char *text, size_t *pos, long long *out_coef, int *out_degree)
{
    bool has_number = false;
    long long coef = 1;
    if (is_digit(text[*pos])) {
        const Status status = read_digits(text, pos, MAX_NUMBER_DIGITS, &coef);
        if (status != STATUS_OK) {
            return status;
        }
        has_number = true;
    }

    int degree = 0;
    if (text[*pos] == 'x') {
        (*pos)++;
        const Status status = read_degree(text, pos, &degree);
        if (status != STATUS_OK) {
            return status;
        }
    } else if (is_letter(text[*pos])) {
        return STATUS_ERR_BAD_VARIABLE;
    } else if (!has_number) {
        return STATUS_ERR_BAD_POLYNOMIAL;
    }
    *out_coef = coef;
    *out_degree = degree;
    return STATUS_OK;
}


Status parse_polynomial(const char *text, Polynomial *out_poly)
{
    if (text == NULL || out_poly == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t pos = 0;
    skip_spaces(text, &pos);
    if (text[pos] == '\0') {
        return STATUS_ERR_EMPTY_ARGUMENT;
    }

    Polynomial poly;
    poly_init(&poly);
    Status status = STATUS_OK;
    bool first = true;
    while (status == STATUS_OK && text[pos] != '\0') {
        bool negative = false;
        if (text[pos] == '+' || text[pos] == '-') {
            negative = text[pos] == '-';
            pos++;
            skip_spaces(text, &pos);
        } else if (!first) {
            status = STATUS_ERR_BAD_POLYNOMIAL;
        }

        long long coef = 0;
        int degree = 0;
        if (status == STATUS_OK) {
            status = read_term(text, &pos, &coef, &degree);
        }
        if (status == STATUS_OK) {
            if (negative) {
                coef = -coef;
            }
            status = poly_add_term(&poly, coef, degree);
            skip_spaces(text, &pos);
            first = false;
        }
    }

    if (status != STATUS_OK) {
        poly_free(&poly);
        return status;
    }
    *out_poly = poly;
    return STATUS_OK;
}


Status parse_integer(const char *text, long long *out_value)
{
    if (text == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t pos = 0;
    skip_spaces(text, &pos);
    if (text[pos] == '\0') {
        return STATUS_ERR_EMPTY_ARGUMENT;
    }
    bool negative = false;
    if (text[pos] == '+' || text[pos] == '-') {
        negative = text[pos] == '-';
        pos++;
    }
    long long value = 0;
    Status status = read_digits(text, &pos, MAX_NUMBER_DIGITS, &value);
    if (status == STATUS_ERR_BAD_POLYNOMIAL) {
        status = STATUS_ERR_BAD_NUMBER;
    }
    if (status != STATUS_OK) {
        return status;
    }
    skip_spaces(text, &pos);
    if (text[pos] != '\0') {
        return STATUS_ERR_BAD_NUMBER;
    }
    if (negative) {
        value = -value;
    }
    *out_value = value;
    return STATUS_OK;
}
