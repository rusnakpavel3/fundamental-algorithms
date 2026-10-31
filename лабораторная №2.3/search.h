#ifndef SEARCH_H
#define SEARCH_H

#include <stddef.h>

#include "status.h"

typedef struct {
    size_t line;
    size_t column;
} Match;

typedef struct {
    const char *path;
    Status status;
    Match *matches;
    size_t count;
    size_t capacity;
} FileResult;

Status find_substring(FileResult **out_results, const char *substring, int file_count, ...);
void free_file_results(FileResult *results, const int file_count);

#endif
