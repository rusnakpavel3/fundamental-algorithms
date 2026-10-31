#include "text_buffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Status text_buffer_init(TextBuffer *buffer)
{
    if (buffer == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    buffer->data = (char *) malloc(TEXT_BUFFER_INITIAL_CAPACITY);
    if (buffer->data == NULL) {
        buffer->length = 0;
        buffer->capacity = 0;
        return STATUS_ERR_ALLOC_FAILED;
    }

    buffer->data[0] = '\0';
    buffer->length = 0;
    buffer->capacity = TEXT_BUFFER_INITIAL_CAPACITY;
    return STATUS_OK;
}

Status text_buffer_reserve(TextBuffer *buffer, const size_t extra)
{
    if (buffer == NULL || buffer->data == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (extra > SIZE_MAX - buffer->length - 1) {
        return STATUS_ERR_OUTPUT_TOO_LONG;
    }

    const size_t needed = buffer->length + extra + 1;
    if (needed <= buffer->capacity) {
        return STATUS_OK;
    }

    size_t new_capacity = buffer->capacity > 0 ? buffer->capacity : 1;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2) {
            new_capacity = needed;
        } else {
            new_capacity *= 2;
        }
    }

    char *grown = (char *) realloc(buffer->data, new_capacity);
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    buffer->data = grown;
    buffer->capacity = new_capacity;
    return STATUS_OK;
}

Status text_buffer_append(TextBuffer *buffer, const char *text, const size_t length)
{
    if (buffer == NULL || text == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const Status status = text_buffer_reserve(buffer, length);
    if (status != STATUS_OK) {
        return status;
    }

    memcpy(buffer->data + buffer->length, text, length);
    buffer->length += length;
    buffer->data[buffer->length] = '\0';
    return STATUS_OK;
}

Status text_buffer_append_char(TextBuffer *buffer, const char c)
{
    return text_buffer_append(buffer, &c, 1);
}

void text_buffer_free(TextBuffer *buffer)
{
    if (buffer == NULL) {
        return;
    }

    free(buffer->data);
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}
