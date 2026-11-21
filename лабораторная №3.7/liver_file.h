#ifndef LIVER_FILE_H
#define LIVER_FILE_H

#include <stddef.h>
#include <stdio.h>

#include "date.h"
#include "liver_list.h"
#include "status.h"

Status read_livers(FILE *file, const Date *today, LiverList *list, size_t *out_line);
Status export_livers(const LiverList *list, const char *input_path, const char *output_path);

#endif
