#ifndef FILE_IO_H
#define FILE_IO_H

#include "status.h"

Status convert_file(const char *in_path, const char *out_path,
                    unsigned long long *out_bad_number);

#endif
