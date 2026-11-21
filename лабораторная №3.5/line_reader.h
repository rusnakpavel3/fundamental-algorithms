#ifndef LINE_READER_H
#define LINE_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "status.h"

Status read_line(FILE *file, char **line, size_t *capacity, bool *out_has_line);

#endif
