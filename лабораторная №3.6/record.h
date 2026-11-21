#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>

#include "datetime.h"
#include "status.h"

#define MAX_COORDINATE_INTEGER_DIGITS 6
#define MAX_COORDINATE_FRACTION_DIGITS 6

typedef enum {
    MARK_START,
    MARK_MIDDLE,
    MARK_END
} StopMark;

typedef struct {
    double x;
    double y;
    const char *name;
} StopPoint;

typedef struct {
    const char *vehicle;
    DateTime arrival;
    DateTime departure;
    StopMark mark;
} RecordFields;

typedef struct {
    char *vehicle;
    DateTime arrival;
    DateTime departure;
    StopMark mark;
    size_t point;
    size_t line;
} Record;

typedef struct {
    Record *items;
    size_t count;
    size_t capacity;
} RecordArray;

Status parse_coordinates(char *line, double *out_x, double *out_y);
Status parse_record(char *line, RecordFields *out_fields);

void records_init(RecordArray *records);
void records_free(RecordArray *records);
Status records_append(RecordArray *records, const RecordFields *fields, size_t point, size_t line);
Status records_sort(RecordArray *records);

#endif
