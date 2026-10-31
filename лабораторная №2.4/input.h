#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>
#include <stdio.h>

#include "backup.h"
#include "status.h"

Status read_backup_input(FILE *stream, FileInfo **out_files, size_t *out_count, long long *out_capacity,
                         size_t *out_error_line);
void free_files(FileInfo *files, const size_t count);

#endif
