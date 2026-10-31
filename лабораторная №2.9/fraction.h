#ifndef FRACTION_H
#define FRACTION_H

#include "status.h"

typedef enum {
    FRACTION_FINITE = 0,
    FRACTION_INFINITE
} FractionKind;

Status fraction_kind(const double x, const int base, const double epsilon, FractionKind *out_kind);
Status classify_fractions(FractionKind **out_results, const double epsilon, const int base, int count, ...);

#endif
