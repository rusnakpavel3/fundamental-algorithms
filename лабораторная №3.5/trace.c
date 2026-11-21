#include "trace.h"


Status trace_student(FILE *trace, const Student *student)
{
    if (trace == NULL || student == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    double average = 0.0;
    const Status status = student_average(student, &average);
    if (status != STATUS_OK) {
        return status;
    }
    if (fprintf(trace, "Студент id %lu: %s %s, группа %s, средний балл %.2f\n",
                student->id, student->last_name, student->first_name, student->group, average) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    if (fflush(trace) != 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    return STATUS_OK;
}


Status trace_above_average(FILE *trace, const Student *students, const size_t *indices, const size_t found,
                           const double total_average)
{
    if (trace == NULL || students == NULL || indices == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (fprintf(trace, "Средний балл выше среднего по всем студентам (%.2f):\n", total_average) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    if (found == 0) {
        if (fprintf(trace, "  таких студентов нет\n") < 0) {
            return STATUS_ERR_WRITE_FAILED;
        }
    }
    for (size_t i = 0; i < found; i++) {
        const Student *student = &students[indices[i]];
        if (fprintf(trace, "  %s %s\n", student->last_name, student->first_name) < 0) {
            return STATUS_ERR_WRITE_FAILED;
        }
    }
    if (fflush(trace) != 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    return STATUS_OK;
}
