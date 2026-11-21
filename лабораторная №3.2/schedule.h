#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stddef.h>

#include "status.h"

#define MINUTES_PER_DAY 1440

typedef struct {
    long long id;
    int start;
    int finish;
} Talk;

Status max_talks(const Talk *talks, const size_t count, const long long halls, size_t *out_count);

#endif
