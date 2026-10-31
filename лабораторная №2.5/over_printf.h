#ifndef OVER_PRINTF_H
#define OVER_PRINTF_H

#include <stdio.h>

#include "status.h"

int overfprintf(FILE *stream, const char *format, ...);
int oversprintf(char *str, const char *format, ...);
Status over_result_status(const int result);

#endif
