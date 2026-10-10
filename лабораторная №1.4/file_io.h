#ifndef FILE_IO_H
#define FILE_IO_H

#include "processor.h"
#include "status.h"

Status process_file(const char *input_path, const char *output_path, const Action action);

#endif
