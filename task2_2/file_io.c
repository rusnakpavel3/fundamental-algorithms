#include "file_io.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "same_file.h"

#define LINE_START_CAPACITY 64
#define LIST_START_CAPACITY 8

static Status grow_line(char **line, size_t *capacity)
{
    if (*capacity > SIZE_MAX / 2) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    const size_t new_capacity = *capacity == 0 ? LINE_START_CAPACITY : *capacity * 2;
    char *grown = (char *) realloc(*line, new_capacity);
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    *line = grown;
    *capacity = new_capacity;
    return STATUS_OK;
}

static Status read_line(FILE *in, char **line, size_t *capacity, bool *out_has_line)
{
    size_t length = 0;
    Status status = STATUS_OK;
    int c = 0;

    *out_has_line = false;

    while ((c = fgetc(in)) != EOF) {
        *out_has_line = true;
        if (c == '\n') {
            break;
        }
        if (c == '\0') {
            return STATUS_ERR_NUL_BYTE;
        }
        if (length + 1 >= *capacity) {
            status = grow_line(line, capacity);
            if (status != STATUS_OK) {
                return status;
            }
        }
        (*line)[length] = (char) c;
        length++;
    }

    if (ferror(in)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (*out_has_line && *capacity == 0) {
        status = grow_line(line, capacity);
    }
    if (status == STATUS_OK && *out_has_line) {
        (*line)[length] = '\0';
    }
    return status;
}

static Status append_employee(Employee **items, size_t *count, size_t *capacity, const Employee *employee)
{
    if (*count == *capacity) {
        if (*capacity > SIZE_MAX / 2 / sizeof(Employee)) {
            return STATUS_ERR_SIZE_OVERFLOW;
        }

        const size_t new_capacity = *capacity == 0 ? LIST_START_CAPACITY : *capacity * 2;
        Employee *grown = (Employee *) realloc(*items, new_capacity * sizeof(Employee));
        if (grown == NULL) {
            return STATUS_ERR_ALLOC_FAILED;
        }

        *items = grown;
        *capacity = new_capacity;
    }

    (*items)[*count] = *employee;
    (*count)++;
    return STATUS_OK;
}

static void free_employees(Employee *items, const size_t count)
{
    for (size_t i = 0; i < count; i++) {
        employee_release(&items[i]);
    }
    free(items);
}

static Status load_employees(FILE *in, Employee **out_items, size_t *out_count, size_t *out_bad_line)
{
    char *line = NULL;
    size_t line_capacity = 0;
    Employee *items = NULL;
    size_t count = 0;
    size_t capacity = 0;
    size_t line_number = 0;
    Status status = STATUS_OK;
    bool has_line = true;

    while (status == STATUS_OK && has_line) {
        line_number++;
        status = read_line(in, &line, &line_capacity, &has_line);

        if (status == STATUS_OK && has_line) {
            Employee employee = {0, NULL, NULL, 0.0};
            bool has_record = false;

            status = parse_employee_line(line, &employee, &has_record);
            if (status == STATUS_OK && has_record) {
                status = append_employee(&items, &count, &capacity, &employee);
                if (status != STATUS_OK) {
                    employee_release(&employee);
                }
            }
        }
    }

    free(line);

    if (status != STATUS_OK) {
        *out_bad_line = line_number;
        free_employees(items, count);
        return status;
    }

    *out_items = items;
    *out_count = count;
    return STATUS_OK;
}

static Status write_output(const char *path, const Employee *items, const size_t count)
{
    FILE *out = fopen(path, "w");
    if (out == NULL) {
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }

    Status status = STATUS_OK;
    for (size_t i = 0; i < count && status == STATUS_OK; i++) {
        if (fprintf(out, "%lld %s %s %.15g\n", items[i].id, items[i].first_name,
                    items[i].last_name, items[i].salary) < 0) {
            status = STATUS_ERR_WRITE_FAILED;
        }
    }

    if (fclose(out) == EOF && status == STATUS_OK) {
        status = STATUS_ERR_CLOSE_FAILED;
    }
    return status;
}

Status sort_employee_file(const char *input_path, const char *output_path,
                          const SortOrder order, size_t *out_count,
                          size_t *out_bad_line)
{
    if (input_path == NULL || output_path == NULL || out_count == NULL || out_bad_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_count = 0;
    *out_bad_line = 0;

    if (order != SORT_ASCENDING && order != SORT_DESCENDING) {
        return STATUS_ERR_FLAG_UNKNOWN;
    }

    FILE *in = fopen(input_path, "r");
    if (in == NULL) {
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }

    Employee *items = NULL;
    size_t count = 0;
    bool same = false;

    Status status = file_matches_path(in, output_path, &same);
    if (status == STATUS_OK && same) {
        status = STATUS_ERR_SAME_FILE;
    }
    if (status == STATUS_OK) {
        status = load_employees(in, &items, &count, out_bad_line);
    }
    fclose(in);

    if (status == STATUS_OK && count > 1) {
        qsort(items, count, sizeof(Employee), order == SORT_ASCENDING ? employee_compare_asc : employee_compare_desc);
    }
    if (status == STATUS_OK) {
        status = write_output(output_path, items, count);
    }
    if (status == STATUS_OK) {
        *out_count = count;
    }

    free_employees(items, count);
    return status;
}
