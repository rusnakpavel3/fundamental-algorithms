#ifndef STUDENT_FILE_H
#define STUDENT_FILE_H

#include <stddef.h>
#include <stdio.h>

#include "status.h"
#include "student.h"

Status read_students(FILE *file, Student **out_students, size_t *out_count, size_t *out_line);

#endif
