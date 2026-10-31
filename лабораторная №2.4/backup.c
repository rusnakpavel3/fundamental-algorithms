#include "backup.h"

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MAX_MEMORY_BYTES 200000000ULL

static void reset_result(BackupResult *result)
{
    result->files = NULL;
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;
}

static Status validate_files(const FileInfo *files, const size_t count, long long *out_size_sum)
{
    long long size_sum = 0;
    long long importance_sum = 0;

    for (size_t i = 0; i < count; i++) {
        if (files[i].name == NULL) {
            return STATUS_ERR_NULL_ARGUMENT;
        }
        if (files[i].name[0] == '\0' || files[i].size <= 0 || files[i].importance < 0) {
            return STATUS_ERR_INVALID_FILE;
        }

        if (size_sum > LLONG_MAX - files[i].size) {
            size_sum = LLONG_MAX;
        } else {
            size_sum += files[i].size;
        }

        if (importance_sum > LLONG_MAX - files[i].importance) {
            return STATUS_ERR_IMPORTANCE_OVERFLOW;
        }
        importance_sum += files[i].importance;
    }

    *out_size_sum = size_sum;
    return STATUS_OK;
}

static void fill_table(const FileInfo *files, const size_t count, const size_t width, long long *best,
                       unsigned char *taken)
{
    const unsigned long long limit = (unsigned long long) (width - 1);

    for (size_t i = 0; i < count; i++) {
        if ((unsigned long long) files[i].size > limit) {
            continue;
        }

        const size_t size = (size_t) files[i].size;
        const long long importance = files[i].importance;

        for (size_t w = width - 1; w >= size; w--) {
            const long long candidate = best[w - size] + importance;
            if (candidate > best[w]) {
                best[w] = candidate;
                taken[i * width + w] = 1;
            }
        }
    }
}

static size_t mark_selected(const FileInfo *files, const size_t count, const size_t width,
                            const unsigned char *taken, bool *selected)
{
    size_t w = width - 1;
    size_t selected_count = 0;

    for (size_t i = count; i > 0; i--) {
        const size_t index = i - 1;
        if (taken[index * width + w]) {
            selected[index] = true;
            w -= (size_t) files[index].size;
            selected_count++;
        }
    }
    return selected_count;
}

static Status build_result(const FileInfo *files, const size_t count, const bool *selected,
                           const size_t selected_count, BackupResult *out_result)
{
    if (selected_count == 0) {
        return STATUS_OK;
    }

    const FileInfo **chosen = (const FileInfo **) malloc(selected_count * sizeof(const FileInfo *));
    if (chosen == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    size_t k = 0;
    long long total_size = 0;
    long long total_importance = 0;

    for (size_t i = 0; i < count; i++) {
        if (selected[i]) {
            chosen[k] = &files[i];
            total_size += files[i].size;
            total_importance += files[i].importance;
            k++;
        }
    }

    out_result->files = chosen;
    out_result->count = selected_count;
    out_result->total_size = total_size;
    out_result->total_importance = total_importance;
    return STATUS_OK;
}

Status select_backup(const FileInfo *files, const size_t count, const long long capacity, BackupResult *out_result)
{
    if (out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    reset_result(out_result);

    if (count > 0 && files == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (capacity < 0) {
        return STATUS_ERR_CAPACITY_INVALID;
    }

    long long size_sum = 0;
    Status status = validate_files(files, count, &size_sum);
    if (status != STATUS_OK) {
        return status;
    }

    if (count == 0 || capacity == 0) {
        return STATUS_OK;
    }

    const long long limit = capacity < size_sum ? capacity : size_sum;

    if ((unsigned long long) count >= MAX_MEMORY_BYTES) {
        return STATUS_ERR_PROBLEM_TOO_LARGE;
    }

    const unsigned long long bytes_per_column = (unsigned long long) count + sizeof(long long);
    if ((unsigned long long) limit >= MAX_MEMORY_BYTES / bytes_per_column) {
        return STATUS_ERR_PROBLEM_TOO_LARGE;
    }

    const size_t width = (size_t) limit + 1;

    long long *best = (long long *) calloc(width, sizeof(long long));
    unsigned char *taken = (unsigned char *) calloc(count * width, 1);
    bool *selected = (bool *) calloc(count, sizeof(bool));

    if (best == NULL || taken == NULL || selected == NULL) {
        free(best);
        free(taken);
        free(selected);
        return STATUS_ERR_ALLOC_FAILED;
    }

    fill_table(files, count, width, best, taken);
    const size_t selected_count = mark_selected(files, count, width, taken, selected);
    status = build_result(files, count, selected, selected_count, out_result);

    free(best);
    free(taken);
    free(selected);
    return status;
}

void free_backup_result(BackupResult *result)
{
    if (result == NULL) {
        return;
    }

    free(result->files);
    reset_result(result);
}
