#include "student_file.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "line_reader.h"

#define START_LINE_CAPACITY 128
#define START_STUDENT_CAPACITY 16


static bool id_exists(const Student *students, const size_t count, const unsigned long id)
{
    for (size_t i = 0; i < count; i++) {
        if (students[i].id == id) {
            return true;
        }
    }
    return false;
}


Status read_students(FILE *file, Student **out_students, size_t *out_count, size_t *out_line)
{
    if (file == NULL || out_students == NULL || out_count == NULL || out_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_students = NULL;
    *out_count = 0;
    *out_line = 0;

    size_t line_capacity = START_LINE_CAPACITY;
    char *line = (char *) malloc(line_capacity);
    if (line == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    size_t capacity = START_STUDENT_CAPACITY;
    Student *students = (Student *) malloc(capacity * sizeof(Student));
    if (students == NULL) {
        free(line);
        return STATUS_ERR_ALLOC_FAILED;
    }

    size_t count = 0;
    size_t line_number = 0;
    Status status = STATUS_OK;
    while (status == STATUS_OK) {
        line_number++;
        bool has_line = false;
        status = read_line(file, &line, &line_capacity, &has_line);
        if (status != STATUS_OK || !has_line) {
            break;
        }
        if (line_is_blank(line)) {
            continue;
        }

        Student student;
        status = parse_student(line, &student);
        if (status != STATUS_OK) {
            break;
        }
        if (id_exists(students, count, student.id)) {
            student_free(&student);
            status = STATUS_ERR_DUPLICATE_ID;
            break;
        }
        if (count == capacity) {
            if (capacity > SIZE_MAX / 2 / sizeof(Student)) {
                student_free(&student);
                status = STATUS_ERR_SIZE_OVERFLOW;
                break;
            }
            const size_t new_capacity = capacity * 2;
            Student *grown = (Student *) realloc(students, new_capacity * sizeof(Student));
            if (grown == NULL) {
                student_free(&student);
                status = STATUS_ERR_ALLOC_FAILED;
                break;
            }
            students = grown;
            capacity = new_capacity;
        }
        students[count] = student;
        count++;
    }
    free(line);

    if (status == STATUS_OK && count == 0) {
        status = STATUS_ERR_NO_STUDENTS;
    }
    if (status != STATUS_OK) {
        students_free(students, count);
        *out_line = line_number;
        return status;
    }

    *out_students = students;
    *out_count = count;
    return STATUS_OK;
}
