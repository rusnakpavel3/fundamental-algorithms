#ifndef FILE_IO_H
#define FILE_IO_H

#include <stddef.h>

#include "employee.h"
#include "status.h"

Status sort_employee_file(const char *input_path, const char *output_path,
                          const SortOrder order, size_t *out_count,
                          size_t *out_bad_line);

#endif
