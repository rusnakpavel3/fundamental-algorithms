#ifndef TOKENS_H
#define TOKENS_H

#include "status.h"

Status merge_files(const char *path1, const char *path2, const char *out_path);

Status transform_file(const char *in_path, const char *out_path);

#endif
