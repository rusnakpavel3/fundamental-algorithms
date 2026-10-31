#include "longest.h"

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static bool is_longest(const double norm, const double max, const double epsilon)
{
    return max - norm <= epsilon * max;
}

static Status collect_longest(const Vector *vectors, const size_t count, const NormFunction function,
                              const void *parameters, const double epsilon, double *norms,
                              LongestResult *out_result)
{
    double max = 0.0;

    for (size_t i = 0; i < count; i++) {
        const Status status = function(&vectors[i], parameters, &norms[i]);
        if (status != STATUS_OK) {
            return status;
        }
        if (!isfinite(norms[i]) || norms[i] < 0.0) {
            return STATUS_ERR_NORM_INVALID;
        }
        max = fmax(max, norms[i]);
    }

    size_t longest = 0;
    for (size_t i = 0; i < count; i++) {
        if (is_longest(norms[i], max, epsilon)) {
            longest++;
        }
    }

    Vector *chosen = (Vector *) malloc(longest * sizeof(Vector));
    if (chosen == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    size_t position = 0;
    for (size_t i = 0; i < count; i++) {
        if (is_longest(norms[i], max, epsilon)) {
            chosen[position] = vectors[i];
            position++;
        }
    }

    out_result->norm = max;
    out_result->vectors = chosen;
    out_result->count = longest;
    return STATUS_OK;
}

static Status read_vectors(va_list *args, const size_t n, Vector *vectors, const size_t count)
{
    for (size_t i = 0; i < count; i++) {
        vectors[i] = va_arg(*args, Vector);

        const Status status = check_vector(&vectors[i]);
        if (status != STATUS_OK) {
            return status;
        }
        if (vectors[i].dimension != n) {
            return STATUS_ERR_DIMENSION_MISMATCH;
        }
    }
    return STATUS_OK;
}

static Status process_norms(va_list *args, const Vector *vectors, const size_t vector_count,
                            const double epsilon, double *norms, LongestResult *results,
                            const size_t norm_count)
{
    Status status = STATUS_OK;

    for (size_t k = 0; k < norm_count && status == STATUS_OK; k++) {
        const NormFunction function = va_arg(*args, NormFunction);
        const void *parameters = va_arg(*args, const void *);

        if (function == NULL) {
            status = STATUS_ERR_NORM_FUNCTION_NULL;
        } else {
            status = collect_longest(vectors, vector_count, function, parameters, epsilon,
                                     norms, &results[k]);
        }
    }
    return status;
}

Status find_longest_vectors(LongestResult **out_results, const double epsilon, const size_t n,
                            const size_t vector_count, size_t norm_count, ...)
{
    if (out_results == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_results = NULL;

    if (!isfinite(epsilon) || epsilon <= 0.0 || epsilon >= 1.0) {
        return STATUS_ERR_EPSILON_INVALID;
    }
    if (n == 0) {
        return STATUS_ERR_DIMENSION_INVALID;
    }
    if (vector_count == 0) {
        return STATUS_ERR_NO_VECTORS;
    }
    if (norm_count == 0) {
        return STATUS_OK;
    }
    if (vector_count > SIZE_MAX / sizeof(Vector) || vector_count > SIZE_MAX / sizeof(double)
        || norm_count > SIZE_MAX / sizeof(LongestResult)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    Vector *vectors = (Vector *) malloc(vector_count * sizeof(Vector));
    double *norms = (double *) malloc(vector_count * sizeof(double));
    LongestResult *results = (LongestResult *) malloc(norm_count * sizeof(LongestResult));

    if (vectors == NULL || norms == NULL || results == NULL) {
        free(vectors);
        free(norms);
        free(results);
        return STATUS_ERR_ALLOC_FAILED;
    }

    for (size_t k = 0; k < norm_count; k++) {
        results[k].norm = 0.0;
        results[k].vectors = NULL;
        results[k].count = 0;
    }

    va_list args;
    va_start(args, norm_count);
    Status status = read_vectors(&args, n, vectors, vector_count);
    if (status == STATUS_OK) {
        status = process_norms(&args, vectors, vector_count, epsilon, norms, results, norm_count);
    }
    va_end(args);

    free(vectors);
    free(norms);

    if (status != STATUS_OK) {
        free_longest_results(results, norm_count);
        return status;
    }

    *out_results = results;
    return STATUS_OK;
}

void free_longest_results(LongestResult *results, const size_t count)
{
    if (results == NULL) {
        return;
    }

    for (size_t k = 0; k < count; k++) {
        free(results[k].vectors);
    }
    free(results);
}
