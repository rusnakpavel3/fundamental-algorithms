#include "search.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_START_CAPACITY 64
#define MATCHES_START_CAPACITY 4

static size_t string_length(const char *str)
{
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

static Status grow_buffer(char **text, size_t *capacity)
{
    size_t new_capacity = 0;

    if (*capacity == 0) {
        new_capacity = BUFFER_START_CAPACITY;
    } else if (*capacity > SIZE_MAX / 2) {
        return STATUS_ERR_FILE_TOO_LARGE;
    } else {
        new_capacity = *capacity * 2;
    }

    char *grown = realloc(*text, new_capacity);
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    *text = grown;
    *capacity = new_capacity;
    return STATUS_OK;
}

static Status read_file(const char *path, char **out_text, size_t *out_length)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return STATUS_ERR_FILE_OPEN_FAILED;
    }

    char *text = NULL;
    size_t length = 0;
    size_t capacity = 0;
    Status status = STATUS_OK;
    int c = 0;

    while (status == STATUS_OK && (c = fgetc(file)) != EOF) {
        if (length == capacity) {
            status = grow_buffer(&text, &capacity);
        }
        if (status == STATUS_OK) {
            text[length] = (char) c;
            length++;
        }
    }

    if (status == STATUS_OK && ferror(file)) {
        status = STATUS_ERR_FILE_READ_FAILED;
    }

    fclose(file);

    if (status != STATUS_OK) {
        free(text);
        return status;
    }

    *out_text = text;
    *out_length = length;
    return STATUS_OK;
}



static Status append_match(FileResult *result, const size_t line, const size_t column)
{
    if (result->count == result->capacity) {
        size_t new_capacity = 0;

        if (result->capacity == 0) {
            new_capacity = MATCHES_START_CAPACITY;
        } else if (result->capacity > SIZE_MAX / 2 / sizeof(Match)) {
            return STATUS_ERR_SIZE_OVERFLOW;
        } else {
            new_capacity = result->capacity * 2;
        }

        Match *grown = realloc(result->matches, new_capacity * sizeof(Match));
        if (grown == NULL) {
            return STATUS_ERR_ALLOC_FAILED;
        }

        result->matches = grown;
        result->capacity = new_capacity;
    }

    result->matches[result->count].line = line;
    result->matches[result->count].column = column;
    result->count++;
    return STATUS_OK;
}

static bool matches_at(const char *text, const size_t position, const char *substring, const size_t substring_length)
{
    for (size_t j = 0; j < substring_length; j++) {
        if (text[position + j] != substring[j]) {
            return false;
        }
    }
    return true;
}

static Status search_text(const char *text, const size_t length, const char *substring,
                          const size_t substring_length, FileResult *result)
{
    if (length < substring_length) {
        return STATUS_OK;
    }

    const size_t last_start = length - substring_length;
    size_t line = 1;
    size_t column = 1;

    for (size_t i = 0; i <= last_start; i++) {
        if (matches_at(text, i, substring, substring_length)) {
            const Status status = append_match(result, line, column);
            if (status != STATUS_OK) {
                return status;
            }
        }

        if (text[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }
    return STATUS_OK;
}

static Status process_file(const char *path, const char *substring, const size_t substring_length,
                           FileResult *result)
{
    result->path = path;

    if (path == NULL) {
        result->status = STATUS_ERR_PATH_NULL;
        return STATUS_OK;
    }
    if (path[0] == '\0') {
        result->status = STATUS_ERR_PATH_EMPTY;
        return STATUS_OK;
    }

    char *text = NULL;
    size_t length = 0;
    Status status = read_file(path, &text, &length);

    if (status == STATUS_ERR_ALLOC_FAILED) {
        return status;
    }
    if (status != STATUS_OK) {
        result->status = status;
        return STATUS_OK;
    }

    status = search_text(text, length, substring, substring_length, result);
    free(text);
    return status;
}

Status find_substring(FileResult **out_results, const char *substring, int file_count, ...)
{
    if (out_results == NULL || substring == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_results = NULL;

    if (substring[0] == '\0') {
        return STATUS_ERR_SUBSTRING_EMPTY;
    }
    if (file_count < 0) {
        return STATUS_ERR_COUNT_NEGATIVE;
    }
    if (file_count == 0) {
        return STATUS_OK;
    }
    if ((size_t) file_count > SIZE_MAX / sizeof(FileResult)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    FileResult *results = malloc((size_t) file_count * sizeof(FileResult));
    if (results == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    for (int i = 0; i < file_count; i++) {
        results[i].path = NULL;
        results[i].status = STATUS_OK;
        results[i].matches = NULL;
        results[i].count = 0;
        results[i].capacity = 0;
    }

    const size_t substring_length = string_length(substring);
    Status status = STATUS_OK;
    va_list args;
    va_start(args, file_count);

    for (int i = 0; i < file_count && status == STATUS_OK; i++) {
        const char *path = va_arg(args, const char *);
        status = process_file(path, substring, substring_length, &results[i]);
    }

    va_end(args);

    if (status != STATUS_OK) {
        free_file_results(results, file_count);
        return status;
    }

    *out_results = results;
    return STATUS_OK;
}

void free_file_results(FileResult *results, const int file_count)
{
    if (results == NULL) {
        return;
    }

    for (int i = 0; i < file_count; i++) {
        free(results[i].matches);
    }
    free(results);
}
