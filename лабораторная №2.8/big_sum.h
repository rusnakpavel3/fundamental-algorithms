#ifndef BIG_SUM_H
#define BIG_SUM_H

#include "status.h"

Status add_in_column(const char *a, const char *b, const int base, char **out_sum);
Status sum_in_base(char **out_result, int base, int count, ...);

#endif
