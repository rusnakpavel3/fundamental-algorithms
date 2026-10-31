#ifndef STRING_OPS_H
#define STRING_OPS_H

#include <stddef.h>

#include "status.h"

Status string_length(const char *text, size_t *out_length);
Status string_reverse(const char *source, char **out_result);
Status string_upper_odd(const char *source, char **out_result);
Status string_group_by_class(const char *source, char **out_result);
Status string_concat_random(const char *const *strings, const size_t count,
                            const unsigned int seed, char **out_result);

#endif
