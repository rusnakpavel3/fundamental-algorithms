#ifndef ARRAYS_H
#define ARRAYS_H

#include <stddef.h>

#include "status.h"

Status random_int(const int low, const int high, int *out);

Status fill_random(int *array, const size_t size, const int low, const int high);

Status swap_min_max(int *array, const size_t size,
                    size_t *out_min_index, size_t *out_max_index);

Status build_sum_with_nearest(const int *a, const size_t a_size,
                              const int *b, const size_t b_size,
                              int **out_c);

#endif
