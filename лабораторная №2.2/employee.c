#include "employee.h"

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define EMPLOYEE_FIELD_COUNT 4

static Status split_fields(char *line, char *fields[EMPLOYEE_FIELD_COUNT], size_t *out_count)
{
    size_t count = 0;
    char *p = line;

    while (*p != '\0') {
        while (isspace((unsigned char) *p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        if (count == EMPLOYEE_FIELD_COUNT) {
            return STATUS_ERR_LINE_FIELD_COUNT;
        }

        fields[count] = p;
        count++;

        while (*p != '\0' && !isspace((unsigned char) *p)) {
            p++;
        }
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    *out_count = count;
    return STATUS_OK;
}


static bool is_latin_word(const char *text)
{
    if (text[0] == '\0') {
        return false;
    }
    for (const char *p = text; *p != '\0'; p++) {
        const bool lower = *p >= 'a' && *p <= 'z';
        const bool upper = *p >= 'A' && *p <= 'Z';
        if (!lower && !upper) {
            return false;
        }
    }
    return true;
}


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static Status parse_id(const char *text, long long *out_id)
{
    if (!is_digit(text[0])) {
        return STATUS_ERR_ID_INVALID;
    }

    long long value = 0;
    const char *p = text;

    while (is_digit(*p)) {
        const long long digit = *p - '0';

        if (value > (LLONG_MAX - digit) / 10) {
            return STATUS_ERR_ID_OUT_OF_RANGE;
        }
        value = value * 10 + digit;
        p++;
    }

    if (*p != '\0') {
        return STATUS_ERR_ID_INVALID;
    }

    *out_id = value;
    return STATUS_OK;
}

static Status parse_salary(const char *text, double *out_salary)
{
    if (text[0] == '-') {
        return STATUS_ERR_SALARY_NEGATIVE;
    }

    int digits = 0;
    bool leading_zeros = true;

    for (const char *p = text; *p != '\0' && *p != 'e' && *p != 'E'; p++) {
        if (is_digit(*p)) {
            if (*p != '0') {
                leading_zeros = false;
            }
            if (!leading_zeros) {
                digits++;
            }
        } else if (*p != '.' && *p != '+') {
            return STATUS_ERR_SALARY_INVALID;
        }
    }
    if (digits > DBL_DIG) {
        return STATUS_ERR_SALARY_TOO_PRECISE;
    }

    char *end = NULL;
    const double value = strtod(text, &end);

    if (end == text || *end != '\0') {
        return STATUS_ERR_SALARY_INVALID;
    }
    if (!isfinite(value) || (digits > 0 && value < DBL_MIN)) {
        return STATUS_ERR_SALARY_OUT_OF_RANGE;
    }

    *out_salary = value;
    return STATUS_OK;
}

static Status copy_string(const char *source, char **out_copy)
{
    const size_t length = strlen(source);

    char *copy = (char *) malloc(length + 1);
    if (copy == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    memcpy(copy, source, length + 1);
    *out_copy = copy;
    return STATUS_OK;
}

Status parse_employee_line(char *line, Employee *out, bool *out_has_record)
{
    if (line == NULL || out == NULL || out_has_record == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_has_record = false;

    char *fields[EMPLOYEE_FIELD_COUNT] = {NULL, NULL, NULL, NULL};
    size_t count = 0;

    Status status = split_fields(line, fields, &count);
    if (status != STATUS_OK) {
        return status;
    }
    if (count == 0) {
        return STATUS_OK;
    }
    if (count != EMPLOYEE_FIELD_COUNT) {
        return STATUS_ERR_LINE_FIELD_COUNT;
    }

    long long id = 0;
    status = parse_id(fields[0], &id);
    if (status != STATUS_OK) {
        return status;
    }
    if (!is_latin_word(fields[1])) {
        return STATUS_ERR_NAME_INVALID;
    }
    if (!is_latin_word(fields[2])) {
        return STATUS_ERR_SURNAME_INVALID;
    }

    double salary = 0.0;
    status = parse_salary(fields[3], &salary);
    if (status != STATUS_OK) {
        return status;
    }

    char *first_name = NULL;
    status = copy_string(fields[1], &first_name);
    if (status != STATUS_OK) {
        return status;
    }

    char *last_name = NULL;
    status = copy_string(fields[2], &last_name);
    if (status != STATUS_OK) {
        free(first_name);
        return status;
    }

    out->id = id;
    out->first_name = first_name;
    out->last_name = last_name;
    out->salary = salary;
    *out_has_record = true;
    return STATUS_OK;
}

void employee_release(Employee *employee)
{
    if (employee == NULL) {
        return;
    }

    free(employee->first_name);
    free(employee->last_name);
    employee->first_name = NULL;
    employee->last_name = NULL;
}

static int compare_ignoring_case(const char *a, const char *b)
{
    size_t i = 0;

    while (a[i] != '\0' && b[i] != '\0') {
        const int ca = tolower((unsigned char) a[i]);
        const int cb = tolower((unsigned char) b[i]);

        if (ca != cb) {
            return ca < cb ? -1 : 1;
        }
        i++;
    }

    if (a[i] == b[i]) {
        return 0;
    }
    return a[i] == '\0' ? -1 : 1;
}

int employee_compare_asc(const void *left, const void *right)
{
    const Employee *a = left;
    const Employee *b = right;

    if (a->salary != b->salary) {
        return a->salary < b->salary ? -1 : 1;
    }

    int result = compare_ignoring_case(a->last_name, b->last_name);
    if (result != 0) {
        return result;
    }

    result = compare_ignoring_case(a->first_name, b->first_name);
    if (result != 0) {
        return result;
    }

    if (a->id != b->id) {
        return a->id < b->id ? -1 : 1;
    }
    return 0;
}

int employee_compare_desc(const void *left, const void *right)
{
    return employee_compare_asc(right, left);
}
