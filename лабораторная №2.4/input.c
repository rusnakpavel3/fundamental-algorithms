#include "input.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LINE_START_CAPACITY 64
#define FILES_START_CAPACITY 8
#define HEADER_TOKENS 2
#define FILE_TOKENS 3
#define MAX_TOKENS 3

typedef struct {
    char *buffer;
    size_t capacity;
    size_t number;
} LineReader;

typedef struct {
    FileInfo *files;
    size_t *lines;
    size_t count;
    size_t capacity;
} FileList;

static Status grow_line(LineReader *reader)
{
    size_t new_capacity = 0;

    if (reader->capacity == 0) {
        new_capacity = LINE_START_CAPACITY;
    } else if (reader->capacity > SIZE_MAX / 2) {
        return STATUS_ERR_SIZE_OVERFLOW;
    } else {
        new_capacity = reader->capacity * 2;
    }

    char *grown = (char *) realloc(reader->buffer, new_capacity);
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    reader->buffer = grown;
    reader->capacity = new_capacity;
    return STATUS_OK;
}

static Status read_line(FILE *stream, LineReader *reader, bool *out_eof)
{
    size_t length = 0;
    bool has_nul = false;
    int c = 0;

    *out_eof = false;

    while ((c = fgetc(stream)) != EOF && c != '\n') {
        if (c == '\0') {
            has_nul = true;
        }
        if (length + 1 >= reader->capacity) {
            const Status status = grow_line(reader);
            if (status != STATUS_OK) {
                return status;
            }
        }
        reader->buffer[length] = (char) c;
        length++;
    }

    if (ferror(stream)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (c == EOF && length == 0) {
        *out_eof = true;
        return STATUS_OK;
    }

    if (reader->capacity == 0) {
        const Status status = grow_line(reader);
        if (status != STATUS_OK) {
            return status;
        }
    }

    reader->buffer[length] = '\0';
    reader->number++;

    if (has_nul) {
        return STATUS_ERR_NUL_BYTE;
    }
    return STATUS_OK;
}

static size_t split_tokens(char *line, char **tokens, const size_t max_tokens)
{
    size_t count = 0;
    char *p = line;

    while (*p != '\0') {
        while (*p != '\0' && isspace((unsigned char) *p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        if (count < max_tokens) {
            tokens[count] = p;
        }
        count++;

        while (*p != '\0' && !isspace((unsigned char) *p)) {
            p++;
        }
        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }
    return count;
}

static Status parse_long_long(const char *str, long long *out)
{
    const char *p = str;
    bool negative = false;

    if (*p == '+' || *p == '-') {
        negative = (*p == '-');
        p++;
    }
    if (!isdigit((unsigned char) *p)) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    const unsigned long long limit = negative ? (unsigned long long) LLONG_MAX + 1ULL
                                              : (unsigned long long) LLONG_MAX;
    unsigned long long magnitude = 0;
    bool too_big = false;

    while (isdigit((unsigned char) *p)) {
        const unsigned long long digit = (unsigned long long) (*p - '0');

        if (!too_big) {
            if (magnitude > (limit - digit) / 10ULL) {
                too_big = true;
            } else {
                magnitude = magnitude * 10ULL + digit;
            }
        }
        p++;
    }

    if (*p != '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }
    if (too_big) {
        return STATUS_ERR_NUMBER_TOO_BIG;
    }

    if (negative && magnitude == limit) {
        *out = LLONG_MIN;
    } else if (negative) {
        *out = -(long long) magnitude;
    } else {
        *out = (long long) magnitude;
    }
    return STATUS_OK;
}

static Status parse_field(const char *str, const long long min_value, const Status invalid_status, long long *out)
{
    long long value = 0;
    const Status status = parse_long_long(str, &value);

    if (status == STATUS_ERR_NUMBER_TOO_BIG) {
        return status;
    }
    if (status != STATUS_OK || value < min_value) {
        return invalid_status;
    }

    *out = value;
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

static Status append_file(FileList *list, const FileInfo file, const size_t line)
{
    if (list->count == list->capacity) {
        size_t new_capacity = 0;

        if (list->capacity == 0) {
            new_capacity = FILES_START_CAPACITY;
        } else if (list->capacity > SIZE_MAX / 2 / sizeof(FileInfo)) {
            return STATUS_ERR_SIZE_OVERFLOW;
        } else {
            new_capacity = list->capacity * 2;
        }

        FileInfo *grown_files = (FileInfo *) realloc(list->files, new_capacity * sizeof(FileInfo));
        if (grown_files == NULL) {
            return STATUS_ERR_ALLOC_FAILED;
        }
        list->files = grown_files;

        size_t *grown_lines = (size_t *) realloc(list->lines, new_capacity * sizeof(size_t));
        if (grown_lines == NULL) {
            return STATUS_ERR_ALLOC_FAILED;
        }
        list->lines = grown_lines;

        list->capacity = new_capacity;
    }

    list->files[list->count] = file;
    list->lines[list->count] = line;
    list->count++;
    return STATUS_OK;
}

static Status read_nonblank_line(FILE *stream, LineReader *reader, char **tokens, size_t *out_token_count,
                                 bool *out_eof)
{
    *out_token_count = 0;

    while (true) {
        const Status status = read_line(stream, reader, out_eof);
        if (status != STATUS_OK || *out_eof) {
            return status;
        }

        *out_token_count = split_tokens(reader->buffer, tokens, MAX_TOKENS);
        if (*out_token_count > 0) {
            return STATUS_OK;
        }
    }
}

static Status read_header(FILE *stream, LineReader *reader, long long *out_count, long long *out_capacity)
{
    char *tokens[MAX_TOKENS] = {NULL, NULL, NULL};
    size_t token_count = 0;
    bool eof = false;

    Status status = read_nonblank_line(stream, reader, tokens, &token_count, &eof);
    if (status != STATUS_OK) {
        return status;
    }
    if (eof) {
        return STATUS_ERR_INPUT_EMPTY;
    }
    if (token_count != HEADER_TOKENS) {
        return STATUS_ERR_HEADER_FORMAT;
    }

    status = parse_field(tokens[0], 1, STATUS_ERR_COUNT_INVALID, out_count);
    if (status == STATUS_OK) {
        status = parse_field(tokens[1], 0, STATUS_ERR_CAPACITY_INVALID, out_capacity);
    }
    return status;
}

static Status read_file_line(FILE *stream, LineReader *reader, FileInfo *out_file)
{
    char *tokens[MAX_TOKENS] = {NULL, NULL, NULL};
    size_t token_count = 0;
    bool eof = false;

    Status status = read_nonblank_line(stream, reader, tokens, &token_count, &eof);
    if (status != STATUS_OK) {
        return status;
    }
    if (eof) {
        return STATUS_ERR_TOO_FEW_LINES;
    }
    if (token_count != FILE_TOKENS) {
        return STATUS_ERR_LINE_FORMAT;
    }

    long long size = 0;
    long long importance = 0;

    status = parse_field(tokens[1], 1, STATUS_ERR_SIZE_INVALID, &size);
    if (status == STATUS_OK) {
        status = parse_field(tokens[2], 0, STATUS_ERR_IMPORTANCE_INVALID, &importance);
    }
    if (status != STATUS_OK) {
        return status;
    }

    char *name = NULL;
    status = copy_string(tokens[0], &name);
    if (status != STATUS_OK) {
        return status;
    }

    out_file->name = name;
    out_file->size = size;
    out_file->importance = importance;
    return STATUS_OK;
}

static int compare_names(const void *a, const void *b)
{
    const FileInfo *const *first = a;
    const FileInfo *const *second = b;
    return strcmp((*first)->name, (*second)->name);
}

static Status find_duplicate(const FileList *list, size_t *out_line)
{
    *out_line = 0;

    if (list->count < 2) {
        return STATUS_OK;
    }
    if (list->count > SIZE_MAX / sizeof(const FileInfo *)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    const FileInfo **sorted = (const FileInfo **) malloc(list->count * sizeof(const FileInfo *));
    if (sorted == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    for (size_t i = 0; i < list->count; i++) {
        sorted[i] = &list->files[i];
    }

    qsort(sorted, list->count, sizeof(const FileInfo *), compare_names);

    Status status = STATUS_OK;
    for (size_t i = 1; i < list->count && status == STATUS_OK; i++) {
        if (strcmp(sorted[i - 1]->name, sorted[i]->name) == 0) {
            const size_t first = (size_t) (sorted[i - 1] - list->files);
            const size_t second = (size_t) (sorted[i] - list->files);
            *out_line = list->lines[first > second ? first : second];
            status = STATUS_ERR_DUPLICATE_NAME;
        }
    }

    free(sorted);
    return status;
}

void free_files(FileInfo *files, const size_t count)
{
    if (files == NULL) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        free(files[i].name);
    }
    free(files);
}

Status read_backup_input(FILE *stream, FileInfo **out_files, size_t *out_count, long long *out_capacity,
                         size_t *out_error_line)
{
    if (stream == NULL || out_files == NULL || out_count == NULL || out_capacity == NULL
        || out_error_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_files = NULL;
    *out_count = 0;
    *out_capacity = 0;
    *out_error_line = 0;

    LineReader reader = {NULL, 0, 0};
    FileList list = {NULL, NULL, 0, 0};
    long long expected = 0;
    long long capacity = 0;

    Status status = read_header(stream, &reader, &expected, &capacity);

    while (status == STATUS_OK && (unsigned long long) list.count < (unsigned long long) expected) {
        FileInfo file = {NULL, 0, 0};
        status = read_file_line(stream, &reader, &file);
        if (status == STATUS_OK) {
            status = append_file(&list, file, reader.number);
            if (status != STATUS_OK) {
                free(file.name);
            }
        }
    }

    if (status != STATUS_OK) {
        *out_error_line = reader.number;
    } else {
        status = find_duplicate(&list, out_error_line);
    }

    free(reader.buffer);
    free(list.lines);

    if (status != STATUS_OK) {
        free_files(list.files, list.count);
        return status;
    }

    *out_files = list.files;
    *out_count = list.count;
    *out_capacity = capacity;
    return STATUS_OK;
}
