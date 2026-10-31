#include "source.h"

#include <ctype.h>
#include <stdarg.h>

void source_from_file(Source *source, FILE *file)
{
    if (source == NULL) {
        return;
    }
    source->file = file;
    source->text = NULL;
    source->position = 0;
    source->consumed = 0;
}

void source_from_text(Source *source, const char *text)
{
    if (source == NULL) {
        return;
    }
    source->file = NULL;
    source->text = text;
    source->position = 0;
    source->consumed = 0;
}

int source_get(Source *source)
{
    int c = EOF;

    if (source == NULL) {
        return EOF;
    }
    if (source->file != NULL) {
        c = fgetc(source->file);
    } else if (source->text != NULL && source->text[source->position] != '\0') {
        c = (unsigned char) source->text[source->position];
        source->position++;
    }

    if (c != EOF) {
        source->consumed++;
    }
    return c;
}

void source_unget(Source *source, const int c)
{
    if (source == NULL || c == EOF) {
        return;
    }

    if (source->file != NULL) {
        ungetc(c, source->file);
    } else {
        source->position--;
    }
    source->consumed--;
}

int source_skip_spaces(Source *source)
{
    int c = source_get(source);
    while (c != EOF && isspace(c)) {
        c = source_get(source);
    }
    source_unget(source, c);
    return c;
}

bool source_has_error(const Source *source)
{
    return source != NULL && source->file != NULL && ferror(source->file);
}

Status source_end_status(const Source *source)
{
    return source_has_error(source) ? STATUS_ERR_READ_FAILED : STATUS_INPUT_END;
}

Status source_scan(Source *source, const char *spec, int *consumed, ...)
{
    if (source == NULL || spec == NULL || consumed == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (source->file == NULL && source->text == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    va_list args;
    int result = 0;

    va_start(args, consumed);
    if (source->file != NULL) {
        result = vfscanf(source->file, spec, args);
    } else {
        result = vsscanf(source->text + source->position, spec, args);
    }
    va_end(args);

    if (*consumed < 0) {
        if (source_has_error(source)) {
            return STATUS_ERR_READ_FAILED;
        }
        return result == EOF ? STATUS_INPUT_END : STATUS_MATCH_FAILURE;
    }

    if (source->file == NULL) {
        source->position += (size_t) *consumed;
    }
    source->consumed += (size_t) *consumed;
    return STATUS_OK;
}
