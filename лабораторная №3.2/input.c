#include "input.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define LINE_BUFFER_SIZE 128
#define MAX_FIELDS 3

static Status read_line(FILE *stream, char *line, const size_t size, bool *out_has_line)
{
    size_t length = 0;
    int c;

    *out_has_line = false;
    while ((c = fgetc(stream)) != EOF && c != '\n') {
        *out_has_line = true;
        if (c == '\0') {
            return STATUS_ERR_NUL_CHARACTER;
        }
        if (length + 1 >= size) {
            return STATUS_ERR_LINE_TOO_LONG;
        }
        line[length] = (char) c;
        length++;
    }
    if (ferror(stream)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (c == '\n') {
        *out_has_line = true;
    }
    line[length] = '\0';
    return STATUS_OK;
}

static size_t split_fields(char *line, char **fields, const size_t max_fields)
{
    size_t count = 0;
    char *p = line;

    while (*p != '\0') {
        while (isspace((unsigned char) *p)) {
            *p = '\0';
            p++;
        }
        if (*p == '\0') {
            break;
        }
        if (count < max_fields) {
            fields[count] = p;
        }
        count++;
        while (*p != '\0' && !isspace((unsigned char) *p)) {
            p++;
        }
    }
    return count;
}

static Status next_line(FILE *stream, char *line, char **fields, size_t *out_field_count,
                        size_t *line_number, bool *out_has_line)
{
    do {
        (*line_number)++;
        const Status status = read_line(stream, line, LINE_BUFFER_SIZE, out_has_line);
        if (status != STATUS_OK) {
            return status;
        }
        if (!*out_has_line) {
            return STATUS_OK;
        }
        *out_field_count = split_fields(line, fields, MAX_FIELDS);
    } while (*out_field_count == 0);
    return STATUS_OK;
}

static Status parse_positive(const char *text, long long *out)
{
    long long value = 0;
    bool too_big = false;

    if (*text == '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    for (const char *p = text; *p != '\0'; p++) {
        if (!isdigit((unsigned char) *p)) {
            return STATUS_ERR_NUMBER_INVALID;
        }
        const int digit = *p - '0';
        if (!too_big) {
            if (value > (LLONG_MAX - digit) / 10) {
                too_big = true;
            } else {
                value = value * 10 + digit;
            }
        }
    }
    if (too_big) {
        return STATUS_ERR_NUMBER_TOO_BIG;
    }
    if (value == 0) {
        return STATUS_ERR_NUMBER_NOT_POSITIVE;
    }
    *out = value;
    return STATUS_OK;
}

static Status parse_time(const char *text, int *out_minutes)
{
    if (!isdigit((unsigned char) text[0]) || !isdigit((unsigned char) text[1]) || text[2] != ':'
        || !isdigit((unsigned char) text[3]) || !isdigit((unsigned char) text[4]) || text[5] != '\0') {
        return STATUS_ERR_TIME_FORMAT;
    }
    const int hours = (text[0] - '0') * 10 + (text[1] - '0');
    const int minutes = (text[3] - '0') * 10 + (text[4] - '0');
    if (hours > 23 || minutes > 59) {
        return STATUS_ERR_TIME_FORMAT;
    }
    *out_minutes = hours * 60 + minutes;
    return STATUS_OK;
}

static Status parse_talk(char *const *fields, const size_t field_count, Talk *out_talk)
{
    if (field_count != 3) {
        return STATUS_ERR_LINE_FORMAT;
    }

    Talk talk = {0, 0, 0};
    Status status = parse_positive(fields[0], &talk.id);
    if (status == STATUS_OK) {
        status = parse_time(fields[1], &talk.start);
    }
    if (status == STATUS_OK) {
        status = parse_time(fields[2], &talk.finish);
    }
    if (status == STATUS_OK && talk.start >= talk.finish) {
        status = STATUS_ERR_TIME_ORDER;
    }
    if (status == STATUS_OK) {
        *out_talk = talk;
    }
    return status;
}

static Status append_talk(Talk **items, size_t *count, size_t *capacity, const Talk *talk)
{
    if (*count == *capacity) {
        size_t new_capacity = 16;
        if (*capacity != 0) {
            if (*capacity > SIZE_MAX / 2 / sizeof(Talk)) {
                return STATUS_ERR_SIZE_OVERFLOW;
            }
            new_capacity = *capacity * 2;
        }
        Talk *grown = (Talk *) realloc(*items, new_capacity * sizeof **items);
        if (grown == NULL) {
            return STATUS_ERR_ALLOC_FAILED;
        }
        *items = grown;
        *capacity = new_capacity;
    }
    (*items)[*count] = *talk;
    (*count)++;
    return STATUS_OK;
}

Status read_input(FILE *stream, Talk **out_talks, size_t *out_count, long long *out_halls, size_t *out_line)
{
    if (stream == NULL || out_talks == NULL || out_count == NULL || out_halls == NULL || out_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_talks = NULL;
    *out_count = 0;
    *out_halls = 0;
    *out_line = 0;

    char line[LINE_BUFFER_SIZE];
    char *fields[MAX_FIELDS];
    size_t field_count = 0;
    size_t line_number = 0;
    bool has_line = false;

    Status status = next_line(stream, line, fields, &field_count, &line_number, &has_line);
    if (status == STATUS_OK && !has_line) {
        status = STATUS_ERR_INPUT_EMPTY;
    }
    if (status == STATUS_OK && field_count != 2) {
        status = STATUS_ERR_HEADER_FORMAT;
    }

    long long expected = 0;
    long long halls = 0;
    if (status == STATUS_OK) {
        status = parse_positive(fields[0], &expected);
    }
    if (status == STATUS_OK) {
        status = parse_positive(fields[1], &halls);
    }
    if (status == STATUS_OK && (unsigned long long) expected > (unsigned long long) SIZE_MAX) {
        status = STATUS_ERR_NUMBER_TOO_BIG;
    }

    Talk *talks = NULL;
    size_t count = 0;
    size_t capacity = 0;
    while (status == STATUS_OK && count < (size_t) expected) {
        status = next_line(stream, line, fields, &field_count, &line_number, &has_line);
        if (status == STATUS_OK && !has_line) {
            status = STATUS_ERR_NOT_ENOUGH_LINES;
        }
        Talk talk = {0, 0, 0};
        if (status == STATUS_OK) {
            status = parse_talk(fields, field_count, &talk);
        }
        if (status == STATUS_OK) {
            status = append_talk(&talks, &count, &capacity, &talk);
        }
    }

    if (status != STATUS_OK) {
        free(talks);
        *out_line = line_number;
        return status;
    }
    *out_talks = talks;
    *out_count = count;
    *out_halls = halls;
    return STATUS_OK;
}
