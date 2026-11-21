#include "schedule.h"

#include <stdint.h>
#include <stdlib.h>

static int compare_by_id(const void *a, const void *b)
{
    const Talk *x = (const Talk *) a;
    const Talk *y = (const Talk *) b;
    return (x->id > y->id) - (x->id < y->id);
}

static int compare_by_finish(const void *a, const void *b)
{
    const Talk *x = (const Talk *) a;
    const Talk *y = (const Talk *) b;
    return (x->finish > y->finish) - (x->finish < y->finish);
}

static Status check_talk(const Talk *talk)
{
    if (talk->id <= 0) {
        return STATUS_ERR_ID_INVALID;
    }
    if (talk->start < 0 || talk->start >= MINUTES_PER_DAY
        || talk->finish < 0 || talk->finish >= MINUTES_PER_DAY) {
        return STATUS_ERR_TIME_INVALID;
    }
    if (talk->start >= talk->finish) {
        return STATUS_ERR_TIME_ORDER;
    }
    return STATUS_OK;
}

static size_t greedy_count(const Talk *sorted, const size_t count, const size_t halls)
{
    size_t free_at[MINUTES_PER_DAY] = {0};
    size_t result = 0;

    free_at[0] = halls;
    for (size_t i = 0; i < count; i++) {
        int t = sorted[i].start;
        while (t >= 0 && free_at[t] == 0) {
            t--;
        }
        if (t >= 0) {
            free_at[t]--;
            free_at[sorted[i].finish]++;
            result++;
        }
    }
    return result;
}

Status max_talks(const Talk *talks, const size_t count, const long long halls, size_t *out_count)
{
    if (out_count == NULL || (talks == NULL && count > 0)) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_count = 0;
    if (halls <= 0) {
        return STATUS_ERR_HALLS_INVALID;
    }
    for (size_t i = 0; i < count; i++) {
        const Status status = check_talk(&talks[i]);
        if (status != STATUS_OK) {
            return status;
        }
    }
    if (count == 0) {
        return STATUS_OK;
    }

    if (count > SIZE_MAX / sizeof(Talk)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }
    Talk *copy = (Talk *) malloc(count * sizeof *copy);
    if (copy == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i < count; i++) {
        copy[i] = talks[i];
    }

    qsort(copy, count, sizeof *copy, compare_by_id);
    for (size_t i = 1; i < count; i++) {
        if (copy[i].id == copy[i - 1].id) {
            free(copy);
            return STATUS_ERR_DUPLICATE_ID;
        }
    }

    qsort(copy, count, sizeof *copy, compare_by_finish);
    const size_t usable = (unsigned long long) halls < (unsigned long long) count ? (size_t) halls : count;
    *out_count = greedy_count(copy, count, usable);

    free(copy);
    return STATUS_OK;
}
