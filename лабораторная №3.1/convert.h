#ifndef CONVERT_H
#define CONVERT_H

#include <stddef.h>

#include "status.h"

Status to_power_of_two_base(const int number, const int power, char *out, const size_t capacity);

#endif
