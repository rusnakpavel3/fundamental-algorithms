#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

#include "processor.h"
#include "status.h"

typedef struct {
    Action action;
    bool has_output_path;
} Flag;

Status parse_flag(const char *str, Flag *out_flag);

Status required_argc(const Flag *flag, int *out_argc);

Status validate_path(const char *path);

Status build_output_path(const char *input_path, char **out_path);

#endif
