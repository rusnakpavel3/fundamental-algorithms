#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "status.h"

Status console_read_line(FILE *stream, char *buffer, size_t size, bool *out_end);

#endif
