#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdio.h>

#include "schedule.h"
#include "status.h"

Status read_input(FILE *stream, Talk **out_talks, size_t *out_count, long long *out_halls, size_t *out_line);

#endif
