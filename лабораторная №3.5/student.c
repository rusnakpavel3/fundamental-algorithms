#include "student.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define FIELD_COUNT 9
#define FIRST_GRADE_FIELD 4


static bool is_space(const char c)
{
    return c == ' ' || c == '\t';
}


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static bool is_latin_letter(const char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


bool line_is_blank(const char *line)
{
    if (line == NULL) {
        return true;
    }
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (!is_space(line[i])) {
            return false;
        }
    }
    return true;
}


static size_t split_fields(char *line, char *fields[], const size_t max_fields)
{
    size_t count = 0;
    size_t i = 0;
    while (line[i] != '\0') {
        while (is_space(line[i])) {
            line[i] = '\0';
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        if (count == max_fields) {
            return max_fields + 1;
        }
        fields[count] = &line[i];
        count++;
        while (line[i] != '\0' && !is_space(line[i])) {
            i++;
        }
    }
    return count;
}


static bool parse_number(const char *text, unsigned long *out_value)
{
    const size_t length = strlen(text);
    if (length == 0 || length > MAX_ID_DIGITS) {
        return false;
    }
    for (size_t i = 0; i < length; i++) {
        if (!is_digit(text[i])) {
            return false;
        }
    }

    char *end = NULL;
    const unsigned long value = strtoul(text, &end, 10);
    if (end == text || *end != '\0') {
        return false;
    }
    *out_value = value;
    return true;
}


static bool is_latin_word(const char *text)
{
    if (text[0] == '\0') {
        return false;
    }
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (!is_latin_letter(text[i])) {
            return false;
        }
    }
    return true;
}


static Status copy_text(const char *text, char **out_copy)
{
    const size_t length = strlen(text);
    char *copy = (char *) malloc(length + 1);
    if (copy == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i <= length; i++) {
        copy[i] = text[i];
    }
    *out_copy = copy;
    return STATUS_OK;
}


Status parse_id(const char *text, unsigned long *out_id)
{
    if (text == NULL || out_id == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!parse_number(text, out_id)) {
        return STATUS_ERR_BAD_ID;
    }
    return STATUS_OK;
}


Status parse_student(char *line, Student *out_student)
{
    if (line == NULL || out_student == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    char *fields[FIELD_COUNT];
    if (split_fields(line, fields, FIELD_COUNT) != FIELD_COUNT) {
        return STATUS_ERR_FIELD_COUNT;
    }

    unsigned long id = 0;
    if (!parse_number(fields[0], &id)) {
        return STATUS_ERR_BAD_ID;
    }
    if (!is_latin_word(fields[1])) {
        return STATUS_ERR_BAD_FIRST_NAME;
    }
    if (!is_latin_word(fields[2])) {
        return STATUS_ERR_BAD_LAST_NAME;
    }

    unsigned char grades[GRADE_COUNT];
    for (size_t g = 0; g < GRADE_COUNT; g++) {
        unsigned long grade = 0;
        if (!parse_number(fields[FIRST_GRADE_FIELD + g], &grade) || grade < MIN_GRADE || grade > MAX_GRADE) {
            return STATUS_ERR_BAD_GRADE;
        }
        grades[g] = (unsigned char) grade;
    }

    Student student = {id, NULL, NULL, NULL, NULL};
    Status status = copy_text(fields[1], &student.first_name);
    if (status == STATUS_OK) {
        status = copy_text(fields[2], &student.last_name);
    }
    if (status == STATUS_OK) {
        status = copy_text(fields[3], &student.group);
    }
    if (status == STATUS_OK) {
        student.grades = (unsigned char *) malloc(GRADE_COUNT * sizeof(unsigned char));
        if (student.grades == NULL) {
            status = STATUS_ERR_ALLOC_FAILED;
        }
    }
    if (status != STATUS_OK) {
        student_free(&student);
        return status;
    }

    for (size_t g = 0; g < GRADE_COUNT; g++) {
        student.grades[g] = grades[g];
    }
    *out_student = student;
    return STATUS_OK;
}


void student_free(Student *student)
{
    if (student == NULL) {
        return;
    }
    free(student->first_name);
    free(student->last_name);
    free(student->group);
    free(student->grades);
    student->first_name = NULL;
    student->last_name = NULL;
    student->group = NULL;
    student->grades = NULL;
}


void students_free(Student *students, const size_t count)
{
    if (students == NULL) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        student_free(&students[i]);
    }
    free(students);
}


Status find_by_id(const Student *students, const size_t count, const unsigned long id, size_t *out_index)
{
    if (students == NULL || out_index == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    for (size_t i = 0; i < count; i++) {
        if (students[i].id == id) {
            *out_index = i;
            return STATUS_OK;
        }
    }
    return STATUS_ERR_NOT_FOUND;
}


static const char *field_text(const Student *student, const TextField field)
{
    if (field == FIELD_LAST_NAME) {
        return student->last_name;
    } else if (field == FIELD_FIRST_NAME) {
        return student->first_name;
    } else {
        return student->group;
    }
}


Status find_by_text(const Student *students, const size_t count, const TextField field, const char *key,
                    size_t *out_indices, size_t *out_found)
{
    if (students == NULL || key == NULL || out_indices == NULL || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t found = 0;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(field_text(&students[i], field), key) == 0) {
            out_indices[found] = i;
            found++;
        }
    }
    *out_found = found;
    return STATUS_OK;
}


static int compare_numbers(const unsigned long a, const unsigned long b)
{
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}


int compare_by_id(const void *a, const void *b)
{
    const Student *first = (const Student *) a;
    const Student *second = (const Student *) b;
    return compare_numbers(first->id, second->id);
}


int compare_by_last_name(const void *a, const void *b)
{
    const Student *first = (const Student *) a;
    const Student *second = (const Student *) b;
    int result = strcmp(first->last_name, second->last_name);
    if (result == 0) {
        result = strcmp(first->first_name, second->first_name);
    }
    if (result == 0) {
        result = compare_numbers(first->id, second->id);
    }
    return result;
}


int compare_by_first_name(const void *a, const void *b)
{
    const Student *first = (const Student *) a;
    const Student *second = (const Student *) b;
    int result = strcmp(first->first_name, second->first_name);
    if (result == 0) {
        result = strcmp(first->last_name, second->last_name);
    }
    if (result == 0) {
        result = compare_numbers(first->id, second->id);
    }
    return result;
}


int compare_by_group(const void *a, const void *b)
{
    const Student *first = (const Student *) a;
    const Student *second = (const Student *) b;
    int result = strcmp(first->group, second->group);
    if (result == 0) {
        result = compare_numbers(first->id, second->id);
    }
    return result;
}


Status sort_students(Student *students, const size_t count, const SortKey key)
{
    if (students == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (key == SORT_BY_ID) {
        qsort(students, count, sizeof(Student), compare_by_id);
    } else if (key == SORT_BY_LAST_NAME) {
        qsort(students, count, sizeof(Student), compare_by_last_name);
    } else if (key == SORT_BY_FIRST_NAME) {
        qsort(students, count, sizeof(Student), compare_by_first_name);
    } else {
        qsort(students, count, sizeof(Student), compare_by_group);
    }
    return STATUS_OK;
}


static unsigned int grade_sum(const Student *student)
{
    unsigned int sum = 0;
    for (size_t g = 0; g < GRADE_COUNT; g++) {
        sum += student->grades[g];
    }
    return sum;
}


Status student_average(const Student *student, double *out_average)
{
    if (student == NULL || student->grades == NULL || out_average == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_average = (double) grade_sum(student) / GRADE_COUNT;
    return STATUS_OK;
}


Status find_above_average(const Student *students, const size_t count, size_t *out_indices, size_t *out_found,
                          double *out_total_average)
{
    if (students == NULL || out_indices == NULL || out_found == NULL || out_total_average == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (count == 0) {
        return STATUS_ERR_NO_STUDENTS;
    }
    if (count > ULLONG_MAX / (MAX_GRADE * GRADE_COUNT)) {
        return STATUS_ERR_SUM_OVERFLOW;
    }

    unsigned long long total = 0;
    for (size_t i = 0; i < count; i++) {
        total += grade_sum(&students[i]);
    }

    const unsigned long long n = (unsigned long long) count;
    size_t found = 0;
    for (size_t i = 0; i < count; i++) {
        const unsigned long long sum = grade_sum(&students[i]);
        if (sum * n > total) {
            out_indices[found] = i;
            found++;
        }
    }

    *out_found = found;
    *out_total_average = (double) total / ((double) n * GRADE_COUNT);
    return STATUS_OK;
}
