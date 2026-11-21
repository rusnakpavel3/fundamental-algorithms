#include "record.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIELDS 6
#define START_RECORD_CAPACITY 16


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static bool is_space(const char c)
{
    return c == ' ' || c == '\t';
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


static Status parse_coordinate(const char *text, double *out_value)
{
    size_t i = 0;
    if (text[i] == '-') {
        i++;
    }
    size_t integer_digits = 0;
    while (is_digit(text[i])) {
        integer_digits++;
        i++;
    }
    if (integer_digits == 0 || integer_digits > MAX_COORDINATE_INTEGER_DIGITS) {
        return STATUS_ERR_BAD_COORDINATES;
    }
    if (text[i] == '.') {
        i++;
        size_t fraction_digits = 0;
        while (is_digit(text[i])) {
            fraction_digits++;
            i++;
        }
        if (fraction_digits == 0 || fraction_digits > MAX_COORDINATE_FRACTION_DIGITS) {
            return STATUS_ERR_BAD_COORDINATES;
        }
    }
    if (text[i] != '\0') {
        return STATUS_ERR_BAD_COORDINATES;
    }

    char *end = NULL;
    const double value = strtod(text, &end);
    if (end == text || *end != '\0') {
        return STATUS_ERR_BAD_COORDINATES;
    }
    *out_value = value;
    return STATUS_OK;
}


Status parse_coordinates(char *line, double *out_x, double *out_y)
{
    if (line == NULL || out_x == NULL || out_y == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    char *fields[MAX_FIELDS];
    const size_t count = split_fields(line, fields, MAX_FIELDS);
    if (count != 2) {
        return STATUS_ERR_BAD_COORDINATES;
    }
    double x = 0.0;
    double y = 0.0;
    Status status = parse_coordinate(fields[0], &x);
    if (status == STATUS_OK) {
        status = parse_coordinate(fields[1], &y);
    }
    if (status != STATUS_OK) {
        return status;
    }
    *out_x = x;
    *out_y = y;
    return STATUS_OK;
}


static Status parse_mark(const char *text, StopMark *out_mark)
{
    if (strcmp(text, "start") == 0) {
        *out_mark = MARK_START;
    } else if (strcmp(text, "middle") == 0) {
        *out_mark = MARK_MIDDLE;
    } else if (strcmp(text, "end") == 0) {
        *out_mark = MARK_END;
    } else {
        return STATUS_ERR_BAD_MARK;
    }
    return STATUS_OK;
}


Status parse_record(char *line, RecordFields *out_fields)
{
    if (line == NULL || out_fields == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    char *fields[MAX_FIELDS];
    const size_t count = split_fields(line, fields, MAX_FIELDS);
    if (count != MAX_FIELDS) {
        return STATUS_ERR_FIELD_COUNT;
    }

    RecordFields record;
    record.vehicle = fields[0];
    Status status = datetime_parse(fields[1], fields[2], &record.arrival);
    if (status == STATUS_OK) {
        status = datetime_parse(fields[3], fields[4], &record.departure);
    }
    if (status == STATUS_OK && record.departure.seconds < record.arrival.seconds) {
        status = STATUS_ERR_DEPARTURE_BEFORE_ARRIVAL;
    }
    if (status == STATUS_OK) {
        status = parse_mark(fields[5], &record.mark);
    }
    if (status != STATUS_OK) {
        return status;
    }
    *out_fields = record;
    return STATUS_OK;
}


void records_init(RecordArray *records)
{
    if (records == NULL) {
        return;
    }
    records->items = NULL;
    records->count = 0;
    records->capacity = 0;
}


void records_free(RecordArray *records)
{
    if (records == NULL) {
        return;
    }
    for (size_t i = 0; i < records->count; i++) {
        free(records->items[i].vehicle);
    }
    free(records->items);
    records->items = NULL;
    records->count = 0;
    records->capacity = 0;
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


static Status reserve_record(RecordArray *records)
{
    if (records->count < records->capacity) {
        return STATUS_OK;
    }
    size_t new_capacity = START_RECORD_CAPACITY;
    if (records->capacity > 0) {
        if (records->capacity > SIZE_MAX / 2 / sizeof(Record)) {
            return STATUS_ERR_SIZE_OVERFLOW;
        }
        new_capacity = records->capacity * 2;
    }
    Record *grown = (Record *) realloc(records->items, new_capacity * sizeof(Record));
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    records->items = grown;
    records->capacity = new_capacity;
    return STATUS_OK;
}


Status records_append(RecordArray *records, const RecordFields *fields, const size_t point, const size_t line)
{
    if (records == NULL || fields == NULL || fields->vehicle == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Status status = reserve_record(records);
    if (status != STATUS_OK) {
        return status;
    }
    Record record;
    record.vehicle = NULL;
    record.arrival = fields->arrival;
    record.departure = fields->departure;
    record.mark = fields->mark;
    record.point = point;
    record.line = line;
    status = copy_text(fields->vehicle, &record.vehicle);
    if (status != STATUS_OK) {
        return status;
    }
    records->items[records->count] = record;
    records->count++;
    return STATUS_OK;
}


static int compare_size(const size_t a, const size_t b)
{
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}


static int compare_records(const void *first, const void *second)
{
    const Record *a = (const Record *) first;
    const Record *b = (const Record *) second;
    const int by_vehicle = strcmp(a->vehicle, b->vehicle);
    if (by_vehicle != 0) {
        return by_vehicle;
    }
    if (a->arrival.seconds < b->arrival.seconds) {
        return -1;
    }
    if (a->arrival.seconds > b->arrival.seconds) {
        return 1;
    }
    const int by_point = compare_size(a->point, b->point);
    if (by_point != 0) {
        return by_point;
    }
    return compare_size(a->line, b->line);
}


Status records_sort(RecordArray *records)
{
    if (records == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (records->count > 1) {
        qsort(records->items, records->count, sizeof(Record), compare_records);
    }
    return STATUS_OK;
}
