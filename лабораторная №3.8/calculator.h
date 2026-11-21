#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdbool.h>

#include "polynomial.h"
#include "script.h"
#include "status.h"

typedef struct {
    Polynomial left;
    Polynomial right;
    bool has_right;
    long long point;
    bool has_point;
    Polynomial result;
    long long value;
    bool is_value;
} Outcome;

void outcome_init(Outcome *outcome);
void outcome_free(Outcome *outcome);
Status run_command(const Command *command, Polynomial *accumulator, Outcome *out_outcome);

#endif
