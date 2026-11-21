#ifndef STUDENT_H
#define STUDENT_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define GRADE_COUNT 5
#define MIN_GRADE 2
#define MAX_GRADE 5
#define MAX_ID_DIGITS 9

typedef struct {
    unsigned long id;
    char *first_name;
    char *last_name;
    char *group;
    unsigned char *grades;
} Student;

typedef enum {
    FIELD_LAST_NAME,
    FIELD_FIRST_NAME,
    FIELD_GROUP
} TextField;

typedef enum {
    SORT_BY_ID,
    SORT_BY_LAST_NAME,
    SORT_BY_FIRST_NAME,
    SORT_BY_GROUP
} SortKey;

bool line_is_blank(const char *line);
Status parse_id(const char *text, unsigned long *out_id);
Status parse_student(char *line, Student *out_student);
void student_free(Student *student);
void students_free(Student *students, size_t count);

Status find_by_id(const Student *students, size_t count, unsigned long id, size_t *out_index);
Status find_by_text(const Student *students, size_t count, TextField field, const char *key,
                    size_t *out_indices, size_t *out_found);

int compare_by_id(const void *a, const void *b);
int compare_by_last_name(const void *a, const void *b);
int compare_by_first_name(const void *a, const void *b);
int compare_by_group(const void *a, const void *b);
Status sort_students(Student *students, size_t count, SortKey key);

Status student_average(const Student *student, double *out_average);
Status find_above_average(const Student *students, size_t count, size_t *out_indices, size_t *out_found,
                          double *out_total_average);

#endif
