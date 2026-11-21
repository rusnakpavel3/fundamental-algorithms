#include "liver_file.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "line_reader.h"

#define START_LINE_CAPACITY 128


static bool is_blank(const char *line)
{
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            return false;
        }
    }
    return true;
}


Status read_livers(FILE *file, const Date *today, LiverList *list, size_t *out_line)
{
    if (file == NULL || today == NULL || list == NULL || out_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_line = 0;

    size_t capacity = START_LINE_CAPACITY;
    char *line = (char *) malloc(capacity);
    if (line == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    size_t line_number = 0;
    Status status = STATUS_OK;
    while (status == STATUS_OK) {
        line_number++;
        bool has_line = false;
        status = read_line(file, &line, &capacity, &has_line);
        if (status != STATUS_OK || !has_line) {
            break;
        }
        if (is_blank(line)) {
            continue;
        }
        Liver liver;
        status = liver_parse_line(line, today, &liver);
        if (status != STATUS_OK) {
            break;
        }
        status = list_insert_loaded(list, &liver);
        liver_free(&liver);
    }
    free(line);

    if (status != STATUS_OK) {
        *out_line = line_number;
    }
    return status;
}


static Status write_liver(FILE *file, const Liver *liver)
{
    if (fprintf(file, "%s %s", liver->surname, liver->name) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    if (liver->patronymic[0] != '\0') {
        if (fprintf(file, " %s", liver->patronymic) < 0) {
            return STATUS_ERR_WRITE_FAILED;
        }
    }
    if (fprintf(file, " %02d.%02d.%04d %c %.2f\n", liver->birth.day, liver->birth.month, liver->birth.year,
                liver->gender, liver->income) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    return STATUS_OK;
}


Status export_livers(const LiverList *list, const char *input_path, const char *output_path)
{
    if (list == NULL || input_path == NULL || output_path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (strcmp(input_path, output_path) == 0) {
        return STATUS_ERR_SAME_FILE;
    }

    FILE *output = fopen(output_path, "w");
    if (output == NULL) {
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }
    Status status = STATUS_OK;
    for (const Node *current = list->head; current != NULL && status == STATUS_OK; current = current->next) {
        status = write_liver(output, &current->data);
    }
    if (fclose(output) == EOF && status == STATUS_OK) {
        status = STATUS_ERR_CLOSE_FAILED;
    }
    return status;
}
