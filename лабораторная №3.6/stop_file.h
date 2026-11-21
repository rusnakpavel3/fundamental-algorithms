#ifndef STOP_FILE_H
#define STOP_FILE_H

#include <stddef.h>
#include <stdio.h>

#include "record.h"
#include "status.h"

Status read_stop_file(FILE *file, size_t point_index, StopPoint *out_point, RecordArray *records,
                      size_t *out_line);

#endif
