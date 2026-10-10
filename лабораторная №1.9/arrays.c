#include "arrays.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

Status random_int(const int low, const int high, int *out)
{
    if (out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (low > high) {
        return STATUS_ERR_RANGE_INVALID;
    }

    const unsigned long long span = (unsigned long long) ((long long) high - (long long) low) + 1ULL;

    unsigned long long r = 0;
    for (int i = 0; i < 4; i++) {
        r = r * 32768ULL + (unsigned long long) (rand() % 32768);
    }

    *out = (int) ((long long) low + (long long) (r % span));
    return STATUS_OK;
}

Status fill_random(int *array, const size_t size, const int low, const int high)
{
    if (array == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (size == 0) {
        return STATUS_ERR_SIZE_INVALID;
    }
    if (low > high) {
        return STATUS_ERR_RANGE_INVALID;
    }

    for (size_t i = 0; i < size; i++) {
        const Status status = random_int(low, high, &array[i]);
        if (status != STATUS_OK) {
            return status;
        }
    }
    return STATUS_OK;
}

Status swap_min_max(int *array, const size_t size,
                    size_t *out_min_index, size_t *out_max_index)
{
    if (array == NULL || out_min_index == NULL || out_max_index == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (size == 0) {
        return STATUS_ERR_SIZE_INVALID;
    }

    size_t min_index = 0;
    size_t max_index = 0;

    for (size_t i = 1; i < size; i++) {
        if (array[i] < array[min_index]) {
            min_index = i;
        }
        if (array[i] > array[max_index]) {
            max_index = i;
        }
    }

    const int temp = array[min_index];
    array[min_index] = array[max_index];
    array[max_index] = temp;

    *out_min_index = min_index;
    *out_max_index = max_index;
    return STATUS_OK;
}

static int compare_ints(const void *first, const void *second)
{
    const int x = *(const int *) first;
    const int y = *(const int *) second;

    if (x < y) {
        return -1;
    }
    if (x > y) {
        return 1;
    }
    return 0;
}

static int nearest_in_sorted(const int *sorted, const size_t size, const int value)
{
    size_t left = 0;
    size_t right = size;

    while (left < right) {
        const size_t middle = left + (right - left) / 2;
        if (sorted[middle] < value) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }

    if (left == size) {
        return sorted[size - 1];
    }
    if (left == 0) {
        return sorted[0];
    }

    const long long distance_right = (long long) sorted[left] - (long long) value;
    const long long distance_left = (long long) value - (long long) sorted[left - 1];

    if (distance_right < distance_left) {
        return sorted[left];
    }
    return sorted[left - 1];
}

Status build_sum_with_nearest(const int *a, const size_t a_size,
                              const int *b, const size_t b_size,
                              int **out_c)
{
    if (a == NULL || b == NULL || out_c == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (a_size == 0 || b_size == 0) {
        return STATUS_ERR_SIZE_INVALID;
    }
    *out_c = NULL;

    int *sorted_b = malloc(b_size * sizeof(int));
    if (sorted_b == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    memcpy(sorted_b, b, b_size * sizeof(int));
    qsort(sorted_b, b_size, sizeof(int), compare_ints);

    int *c = malloc(a_size * sizeof(int));
    if (c == NULL) {
        free(sorted_b);
        return STATUS_ERR_ALLOC_FAILED;
    }

    for (size_t i = 0; i < a_size; i++) {
        const int nearest = nearest_in_sorted(sorted_b, b_size, a[i]);
        const long long sum = (long long) a[i] + (long long) nearest;

        if (sum < INT_MIN || sum > INT_MAX) {
            free(sorted_b);
            free(c);
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }
        c[i] = (int) sum;
    }

    free(sorted_b);
    *out_c = c;
    return STATUS_OK;
}
