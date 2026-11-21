#include "console.h"

#include <ctype.h>
#include <stdint.h>


Status console_read_line(FILE *stream, char *buffer, const size_t size, bool *out_end)
{
    if (stream == NULL || buffer == NULL || out_end == NULL || size < 2) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_end = false;
    buffer[0] = '\0';

    size_t length = 0;
    bool too_long = false;
    bool got_any = false;
    int c = 0;
    while ((c = fgetc(stream)) != EOF && c != '\n') {
        got_any = true;
        if (length + 1 < size) {
            buffer[length] = (char) c;
            length++;
        } else {
            too_long = true;
        }
    }
    if (ferror(stream)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (c == EOF && !got_any) {
        *out_end = true;
        return STATUS_OK;
    }
    if (length > 0 && buffer[length - 1] == '\r') {
        length--;
    }
    buffer[length] = '\0';
    if (too_long) {
        return STATUS_ERR_LINE_TOO_LONG;
    }
    return STATUS_OK;
}


Status parse_count(const char *text, size_t *out_value)
{
    if (text == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (text[0] == '\0') {
        return STATUS_ERR_NUMBER_INVALID;
    }

    size_t value = 0;
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (!isdigit((unsigned char) text[i])) {
            return STATUS_ERR_NUMBER_INVALID;
        }
        const size_t digit = (size_t) (text[i] - '0');
        if (value > (SIZE_MAX - digit) / 10) {
            value = SIZE_MAX;
        } else {
            value = value * 10 + digit;
        }
    }
    if (value == 0) {
        return STATUS_ERR_NUMBER_INVALID;
    }
    *out_value = value;
    return STATUS_OK;
}
