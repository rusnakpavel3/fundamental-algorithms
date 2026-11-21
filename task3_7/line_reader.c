#include "line_reader.h"

#include <stdint.h>
#include <stdlib.h>


Status read_line(FILE *file, char **line, size_t *capacity, bool *out_has_line)
{
    if (file == NULL || line == NULL || *line == NULL || capacity == NULL || *capacity == 0
        || out_has_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    size_t length = 0;
    int c = 0;

    *out_has_line = false;
    while ((c = fgetc(file)) != EOF && c != '\n') {
        *out_has_line = true;
        if (c == '\0') {
            return STATUS_ERR_NUL_BYTE;
        }
        if (length + 1 >= *capacity) {
            if (*capacity > SIZE_MAX / 2) {
                return STATUS_ERR_SIZE_OVERFLOW;
            }
            char *grown = (char *) realloc(*line, *capacity * 2);
            if (grown == NULL) {
                return STATUS_ERR_ALLOC_FAILED;
            }
            *line = grown;
            *capacity *= 2;
        }
        (*line)[length] = (char) c;
        length++;
    }
    if (ferror(file)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (c == '\n') {
        *out_has_line = true;
    }
    if (length > 0 && (*line)[length - 1] == '\r') {
        length--;
    }
    (*line)[length] = '\0';
    return STATUS_OK;
}
