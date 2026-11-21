#include "console.h"


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
