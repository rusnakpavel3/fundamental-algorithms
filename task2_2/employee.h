#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <stdbool.h>

#include "status.h"

typedef enum {
    SORT_ASCENDING,
    SORT_DESCENDING
} SortOrder;

typedef struct {
    long long id;
    char *first_name;
    char *last_name;
    double salary;
} Employee;

Status parse_employee_line(char *line, Employee *out, bool *out_has_record);
void employee_release(Employee *employee);
int employee_compare_asc(const void *left, const void *right);
int employee_compare_desc(const void *left, const void *right);

#endif
