#ifndef LONGEST_H
#define LONGEST_H

#include <stddef.h>

#include "norms.h"
#include "status.h"

typedef struct {
    double norm;
    Vector *vectors;
    size_t count;
} LongestResult;

Status find_longest_vectors(LongestResult **out_results, const double epsilon, const size_t n,
                            const size_t vector_count, size_t norm_count, ...);
void free_longest_results(LongestResult *results, const size_t count);

#endif
