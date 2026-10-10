#ifndef SAME_FILE_H
#define SAME_FILE_H

#include <stdbool.h>
#include <stdio.h>

#include "status.h"

Status file_matches_path(FILE *opened, const char *path, bool *out_same);

#endif
