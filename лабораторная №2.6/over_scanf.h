#ifndef OVER_SCANF_H
#define OVER_SCANF_H

#include <stdio.h>

#include "status.h"

int overfscanf(FILE *stream, const char *format, ...);
int oversscanf(const char *str, const char *format, ...);
Status over_result_status(const int result);

#endif
