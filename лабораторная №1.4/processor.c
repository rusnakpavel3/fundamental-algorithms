#include "processor.h"

#include <limits.h>
#include <stdio.h>

static bool is_arabic_digit(const int c)
{
    return c >= '0' && c <= '9';
}

static bool is_latin_letter(const int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool is_other_symbol(const int c)
{
    return !is_latin_letter(c) && !is_arabic_digit(c) && c != ' ';
}

static Status check_written(const int written, const size_t out_size, size_t *out_len)
{
    if (written < 0 || (size_t) written >= out_size) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }
    *out_len = (size_t) written;
    return STATUS_OK;
}

static Status write_counter(Processor *processor, char *out, const size_t out_size, size_t *out_len)
{
    const int written = snprintf(out, out_size, "%llu\n", processor->counter);

    processor->counter = 0;
    processor->line_started = false;
    return check_written(written, out_size, out_len);
}

static Status feed_drop_digits(const int c, char *out, size_t *out_len)
{
    if (is_arabic_digit(c)) {
        *out_len = 0;
        return STATUS_OK;
    }
    out[0] = (char) c;
    *out_len = 1;
    return STATUS_OK;
}

static Status feed_count(Processor *processor, const int c, const bool counts,
                         char *out, const size_t out_size, size_t *out_len)
{
    if (c == '\n') {
        return write_counter(processor, out, out_size, out_len);
    }

    processor->line_started = true;
    if (counts) {
        if (processor->counter == ULLONG_MAX) {
            return STATUS_ERR_COMPUTE_OVERFLOW;
        }
        processor->counter++;
    }
    *out_len = 0;
    return STATUS_OK;
}

static Status feed_hex(Processor *processor, const int c,
                       char *out, const size_t out_size, size_t *out_len)
{
    const char *separator = "";
    if (processor->need_separator) {
        separator = " ";
    }

    int written;
    if (is_arabic_digit(c)) {
        written = snprintf(out, out_size, "%s%c", separator, c);
    } else if (c == '\n') {
        written = snprintf(out, out_size, "%s%02X\n", separator, (unsigned int) c);
    } else {
        written = snprintf(out, out_size, "%s%02X", separator, (unsigned int) c);
    }

    processor->need_separator = (c != '\n');
    return check_written(written, out_size, out_len);
}

Status processor_init(Processor *processor, const Action action)
{
    if (processor == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    switch (action) {
        case ACTION_DROP_DIGITS:
        case ACTION_COUNT_LATIN:
        case ACTION_COUNT_OTHER:
        case ACTION_HEX_CODES:
            processor->action = action;
            processor->counter = 0;
            processor->line_started = false;
            processor->need_separator = false;
            return STATUS_OK;
    }
    return STATUS_ERR_VALUE_OUT_OF_RANGE;
}

Status processor_feed(Processor *processor, const int c,
                      char *out, const size_t out_size, size_t *out_len)
{
    if (processor == NULL || out == NULL || out_len == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (c < 0 || c > UCHAR_MAX || out_size < PROCESSOR_OUTPUT_MAX) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    *out_len = 0;

    switch (processor->action) {
        case ACTION_DROP_DIGITS:
            return feed_drop_digits(c, out, out_len);
        case ACTION_COUNT_LATIN:
            return feed_count(processor, c, is_latin_letter(c), out, out_size, out_len);
        case ACTION_COUNT_OTHER:
            return feed_count(processor, c, is_other_symbol(c), out, out_size, out_len);
        case ACTION_HEX_CODES:
            return feed_hex(processor, c, out, out_size, out_len);
    }
    return STATUS_ERR_VALUE_OUT_OF_RANGE;
}

Status processor_finish(Processor *processor,
                        char *out, const size_t out_size, size_t *out_len)
{
    if (processor == NULL || out == NULL || out_len == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (out_size < PROCESSOR_OUTPUT_MAX) {
        return STATUS_ERR_VALUE_OUT_OF_RANGE;
    }

    *out_len = 0;

    switch (processor->action) {
        case ACTION_DROP_DIGITS:
        case ACTION_HEX_CODES:
            return STATUS_OK;
        case ACTION_COUNT_LATIN:
        case ACTION_COUNT_OTHER:
            if (processor->line_started) {
                return write_counter(processor, out, out_size, out_len);
            }
            return STATUS_OK;
    }
    return STATUS_ERR_VALUE_OUT_OF_RANGE;
}
