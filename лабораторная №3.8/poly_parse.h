#ifndef POLY_PARSE_H
#define POLY_PARSE_H

#include "polynomial.h"
#include "status.h"

#define MAX_NUMBER_DIGITS 18
#define MAX_DEGREE_DIGITS 5

Status parse_polynomial(const char *text, Polynomial *out_poly);
Status parse_integer(const char *text, long long *out_value);

#endif
