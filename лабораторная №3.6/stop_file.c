#include "stop_file.h"

#include <stdbool.h>
#include <stdlib.h>

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


Status read_stop_file(FILE *file, const size_t point_index, StopPoint *out_point, RecordArray *records,
                      size_t *out_line)
{
    if (file == NULL || out_point == NULL || records == NULL || out_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_line = 0;

    size_t capacity = START_LINE_CAPACITY;
    char *line = (char *) malloc(capacity);
    if (line == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    bool has_point = false;
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
        if (!has_point) {
            status = parse_coordinates(line, &out_point->x, &out_point->y);
            has_point = true;
        } else {
            RecordFields fields;
            status = parse_record(line, &fields);
            if (status == STATUS_OK) {
                status = records_append(records, &fields, point_index, line_number);
            }
        }
    }
    free(line);

    if (status == STATUS_OK && !has_point) {
        return STATUS_ERR_NO_COORDINATES;
    }
    if (status != STATUS_OK) {
        *out_line = line_number;
    }
    return status;
}
