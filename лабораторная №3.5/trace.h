#ifndef TRACE_H
#define TRACE_H

#include <stddef.h>
#include <stdio.h>

#include "status.h"
#include "student.h"

Status trace_student(FILE *trace, const Student *student);
Status trace_above_average(FILE *trace, const Student *students, const size_t *indices, size_t found,
                           double total_average);

#endif
