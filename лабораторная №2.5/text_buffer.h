#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include <stddef.h>

#include "status.h"

#define TEXT_BUFFER_INITIAL_CAPACITY 64

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} TextBuffer;

Status text_buffer_init(TextBuffer *buffer);
Status text_buffer_reserve(TextBuffer *buffer, const size_t extra);
Status text_buffer_append(TextBuffer *buffer, const char *text, const size_t length);
Status text_buffer_append_char(TextBuffer *buffer, const char c);
void text_buffer_free(TextBuffer *buffer);

#endif
