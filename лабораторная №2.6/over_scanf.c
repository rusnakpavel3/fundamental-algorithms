#include "over_scanf.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "custom_scan.h"
#include "source.h"

#define SPEC_MAX_LENGTH 64

typedef enum {
    LENGTH_NONE,
    LENGTH_HH,
    LENGTH_H,
    LENGTH_L,
    LENGTH_LL,
    LENGTH_J,
    LENGTH_Z,
    LENGTH_T,
    LENGTH_BIG_L
} LengthModifier;

typedef struct {
    char text[SPEC_MAX_LENGTH];
    size_t length;
    bool suppress;
    bool has_width;
    LengthModifier modifier;
    char conversion;
} Spec;

typedef enum {
    CUSTOM_NONE,
    CUSTOM_ROMAN,
    CUSTOM_ZECKENDORF,
    CUSTOM_BASE_LOWER,
    CUSTOM_BASE_UPPER
} CustomKind;

static CustomKind custom_kind(const char *p)
{
    if (p[0] == 'R' && p[1] == 'o') {
        return CUSTOM_ROMAN;
    }
    if (p[0] == 'Z' && p[1] == 'r') {
        return CUSTOM_ZECKENDORF;
    }
    if (p[0] == 'C' && p[1] == 'v') {
        return CUSTOM_BASE_LOWER;
    }
    if (p[0] == 'C' && p[1] == 'V') {
        return CUSTOM_BASE_UPPER;
    }
    return CUSTOM_NONE;
}

static Status spec_append(Spec *spec, const char c)
{
    if (spec->length + 1 >= SPEC_MAX_LENGTH) {
        return STATUS_ERR_SPEC_TOO_LONG;
    }

    spec->text[spec->length] = c;
    spec->length++;
    spec->text[spec->length] = '\0';
    return STATUS_OK;
}

static Status parse_width(const char **cursor, Spec *spec)
{
    Status status = STATUS_OK;
    int value = 0;

    while (status == STATUS_OK && isdigit((unsigned char) **cursor)) {
        const int digit = **cursor - '0';
        if (value > (INT_MAX - digit) / 10) {
            return STATUS_ERR_FORMAT_INVALID;
        }
        value = value * 10 + digit;
        spec->has_width = true;

        status = spec_append(spec, **cursor);
        (*cursor)++;
    }

    if (status == STATUS_OK && spec->has_width && value == 0) {
        return STATUS_ERR_FORMAT_INVALID;
    }
    return status;
}

static Status parse_modifier(const char **cursor, Spec *spec)
{
    const char *p = *cursor;
    size_t count = 0;

    if (p[0] == 'h' && p[1] == 'h') {
        spec->modifier = LENGTH_HH;
        count = 2;
    } else if (p[0] == 'h') {
        spec->modifier = LENGTH_H;
        count = 1;
    } else if (p[0] == 'l' && p[1] == 'l') {
        spec->modifier = LENGTH_LL;
        count = 2;
    } else if (p[0] == 'l') {
        spec->modifier = LENGTH_L;
        count = 1;
    } else if (p[0] == 'j') {
        spec->modifier = LENGTH_J;
        count = 1;
    } else if (p[0] == 'z') {
        spec->modifier = LENGTH_Z;
        count = 1;
    } else if (p[0] == 't') {
        spec->modifier = LENGTH_T;
        count = 1;
    } else if (p[0] == 'L') {
        spec->modifier = LENGTH_BIG_L;
        count = 1;
    }

    Status status = STATUS_OK;
    for (size_t i = 0; i < count && status == STATUS_OK; i++) {
        status = spec_append(spec, p[i]);
    }

    *cursor = p + count;
    return status;
}

static Status parse_scanset(const char **cursor, Spec *spec)
{
    const char *p = *cursor;
    Status status = STATUS_OK;

    if (*p == '^') {
        status = spec_append(spec, *p);
        p++;
    }
    if (status == STATUS_OK && *p == ']') {
        status = spec_append(spec, *p);
        p++;
    }
    while (status == STATUS_OK && *p != ']') {
        if (*p == '\0') {
            return STATUS_ERR_FORMAT_INVALID;
        }
        status = spec_append(spec, *p);
        p++;
    }
    if (status == STATUS_OK) {
        status = spec_append(spec, ']');
        p++;
    }

    *cursor = p;
    return status;
}

static Status check_spec(const Spec *spec)
{
    const char c = spec->conversion;
    const LengthModifier m = spec->modifier;

    if (c == 'n') {
        if (spec->suppress || spec->has_width || m == LENGTH_BIG_L) {
            return STATUS_ERR_FORMAT_INVALID;
        }
        return m == LENGTH_NONE ? STATUS_OK : STATUS_ERR_FORMAT_UNSUPPORTED;
    }
    if (strchr("diouxX", c) != NULL) {
        return m == LENGTH_BIG_L ? STATUS_ERR_FORMAT_INVALID : STATUS_OK;
    }
    if (strchr("aAeEfFgG", c) != NULL) {
        if (m == LENGTH_NONE || m == LENGTH_L || m == LENGTH_BIG_L) {
            return STATUS_OK;
        }
        return STATUS_ERR_FORMAT_INVALID;
    }
    if (strchr("cs[", c) != NULL) {
        if (m == LENGTH_L) {
            return STATUS_ERR_FORMAT_UNSUPPORTED;
        }
        return m == LENGTH_NONE ? STATUS_OK : STATUS_ERR_FORMAT_INVALID;
    }
    if (c == 'p') {
        return m == LENGTH_NONE ? STATUS_OK : STATUS_ERR_FORMAT_INVALID;
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status parse_spec(const char **cursor, Spec *spec)
{
    const char *p = *cursor + 1;

    spec->text[0] = '\0';
    spec->length = 0;
    spec->suppress = false;
    spec->has_width = false;
    spec->modifier = LENGTH_NONE;
    spec->conversion = '\0';

    Status status = spec_append(spec, '%');

    if (status == STATUS_OK && *p == '*') {
        spec->suppress = true;
        status = spec_append(spec, *p);
        p++;
    }
    if (status == STATUS_OK) {
        status = parse_width(&p, spec);
    }
    if (status == STATUS_OK) {
        status = parse_modifier(&p, spec);
    }

    if (status == STATUS_OK) {
        if (*p == '\0') {
            status = STATUS_ERR_FORMAT_INVALID;
        } else {
            spec->conversion = *p;
            p++;
            status = spec_append(spec, spec->conversion);
        }
    }
    if (status == STATUS_OK && spec->conversion == '[') {
        status = parse_scanset(&p, spec);
    }
    if (status == STATUS_OK) {
        status = check_spec(spec);
    }
    if (status == STATUS_OK) {
        status = spec_append(spec, '%');
    }
    if (status == STATUS_OK) {
        status = spec_append(spec, 'n');
    }

    *cursor = p;
    return status;
}

static Status check_format(const char *format)
{
    const char *p = format;
    Status status = STATUS_OK;

    while (*p != '\0' && status == STATUS_OK) {
        if (*p != '%') {
            p++;
        } else if (p[1] == '%') {
            p += 2;
        } else if (custom_kind(p + 1) != CUSTOM_NONE) {
            p += 3;
        } else {
            Spec spec;
            status = parse_spec(&p, &spec);
        }
    }
    return status;
}

static Status match_char(Source *source, const char expected)
{
    const int c = source_get(source);

    if (c == EOF) {
        return source_end_status(source);
    }
    if (c != (unsigned char) expected) {
        source_unget(source, c);
        return STATUS_MATCH_FAILURE;
    }
    return STATUS_OK;
}

static int normalize_base(const int base)
{
    return base_is_valid(base) ? base : BASE_DEFAULT;
}

static Status scan_custom(Source *source, const CustomKind kind, va_list *args)
{
    if (kind == CUSTOM_ROMAN) {
        int *target = va_arg(*args, int *);
        return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL : scan_roman(source, target);
    }
    if (kind == CUSTOM_ZECKENDORF) {
        unsigned int *target = va_arg(*args, unsigned int *);
        return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL : scan_zeckendorf(source, target);
    }

    int *target = va_arg(*args, int *);
    const int base = normalize_base(va_arg(*args, int));
    if (target == NULL) {
        return STATUS_ERR_POINTER_ARGUMENT_NULL;
    }

    const LetterCase letters = kind == CUSTOM_BASE_UPPER ? LETTERS_UPPER : LETTERS_LOWER;
    return scan_base(source, base, letters, target);
}

static Status scan_signed(Source *source, const Spec *spec, va_list *args)
{
    int consumed = -1;

    switch (spec->modifier) {
        case LENGTH_NONE: {
            int *target = va_arg(*args, int *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_HH: {
            signed char *target = va_arg(*args, signed char *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_H: {
            short *target = va_arg(*args, short *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_L: {
            long *target = va_arg(*args, long *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_LL: {
            long long *target = va_arg(*args, long long *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_J: {
            intmax_t *target = va_arg(*args, intmax_t *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_Z: {
            size_t *target = va_arg(*args, size_t *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_T: {
            ptrdiff_t *target = va_arg(*args, ptrdiff_t *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_BIG_L:
            break;
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status scan_unsigned(Source *source, const Spec *spec, va_list *args)
{
    int consumed = -1;

    switch (spec->modifier) {
        case LENGTH_NONE: {
            unsigned int *target = va_arg(*args, unsigned int *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_HH: {
            unsigned char *target = va_arg(*args, unsigned char *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_H: {
            unsigned short *target = va_arg(*args, unsigned short *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_L: {
            unsigned long *target = va_arg(*args, unsigned long *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_LL: {
            unsigned long long *target = va_arg(*args, unsigned long long *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_J: {
            uintmax_t *target = va_arg(*args, uintmax_t *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_Z: {
            size_t *target = va_arg(*args, size_t *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_T: {
            ptrdiff_t *target = va_arg(*args, ptrdiff_t *);
            return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                                  : source_scan(source, spec->text, &consumed, target, &consumed);
        }
        case LENGTH_BIG_L:
            break;
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status scan_floating(Source *source, const Spec *spec, va_list *args)
{
    int consumed = -1;

    if (spec->modifier == LENGTH_NONE) {
        float *target = va_arg(*args, float *);
        return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                              : source_scan(source, spec->text, &consumed, target, &consumed);
    }
    if (spec->modifier == LENGTH_L) {
        double *target = va_arg(*args, double *);
        return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                              : source_scan(source, spec->text, &consumed, target, &consumed);
    }
    if (spec->modifier == LENGTH_BIG_L) {
        long double *target = va_arg(*args, long double *);
        return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                              : source_scan(source, spec->text, &consumed, target, &consumed);
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status scan_text(Source *source, const Spec *spec, va_list *args)
{
    int consumed = -1;
    char *target = va_arg(*args, char *);

    return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                          : source_scan(source, spec->text, &consumed, target, &consumed);
}

static Status scan_pointer(Source *source, const Spec *spec, va_list *args)
{
    int consumed = -1;
    void **target = va_arg(*args, void **);

    return target == NULL ? STATUS_ERR_POINTER_ARGUMENT_NULL
                          : source_scan(source, spec->text, &consumed, target, &consumed);
}

static Status store_count(const Source *source, va_list *args)
{
    int *target = va_arg(*args, int *);

    if (target == NULL) {
        return STATUS_ERR_POINTER_ARGUMENT_NULL;
    }
    if (source->consumed > (size_t) INT_MAX) {
        return STATUS_ERR_INPUT_TOO_LONG;
    }

    *target = (int) source->consumed;
    return STATUS_OK;
}

static Status scan_standard(Source *source, const char **cursor, va_list *args, bool *out_stored)
{
    Spec spec;
    const Status status = parse_spec(cursor, &spec);

    *out_stored = false;
    if (status != STATUS_OK) {
        return status;
    }

    if (spec.conversion == 'n') {
        return store_count(source, args);
    }
    if (spec.suppress) {
        int consumed = -1;
        return source_scan(source, spec.text, &consumed, &consumed);
    }

    *out_stored = true;
    switch (spec.conversion) {
        case 'd':
        case 'i':
            return scan_signed(source, &spec, args);
        case 'o':
        case 'u':
        case 'x':
        case 'X':
            return scan_unsigned(source, &spec, args);
        case 'p':
            return scan_pointer(source, &spec, args);
        case 'c':
        case 's':
        case '[':
            return scan_text(source, &spec, args);
        default:
            return scan_floating(source, &spec, args);
    }
}

static Status scan_all(Source *source, const char *format, va_list *args, int *out_assigned)
{
    const char *p = format;
    Status status = STATUS_OK;
    int assigned = 0;

    while (*p != '\0' && status == STATUS_OK) {
        if (isspace((unsigned char) *p)) {
            source_skip_spaces(source);
            p++;
        } else if (*p != '%') {
            status = match_char(source, *p);
            p++;
        } else if (p[1] == '%') {
            source_skip_spaces(source);
            status = match_char(source, '%');
            p += 2;
        } else {
            const CustomKind kind = custom_kind(p + 1);
            bool stored = true;

            if (kind != CUSTOM_NONE) {
                status = scan_custom(source, kind, args);
                p += 3;
            } else {
                status = scan_standard(source, &p, args, &stored);
            }

            if (status == STATUS_OK && stored) {
                assigned++;
            }
        }
    }

    if (status == STATUS_OK && source_has_error(source)) {
        status = STATUS_ERR_READ_FAILED;
    }
    if (status == STATUS_MATCH_FAILURE || (status == STATUS_INPUT_END && assigned > 0)) {
        status = STATUS_OK;
    }

    *out_assigned = assigned;
    return status;
}

static int result_code(const Status status, const int assigned)
{
    if (status == STATUS_OK) {
        return assigned;
    }
    if (status == STATUS_INPUT_END) {
        return EOF;
    }
    return -(int) status;
}

int overfscanf(FILE *stream, const char *format, ...)
{
    if (stream == NULL || format == NULL) {
        return result_code(STATUS_ERR_NULL_ARGUMENT, 0);
    }

    const Status format_status = check_format(format);
    if (format_status != STATUS_OK) {
        return result_code(format_status, 0);
    }

    Source source;
    source_from_file(&source, stream);

    int assigned = 0;
    va_list args;
    va_start(args, format);
    const Status status = scan_all(&source, format, &args, &assigned);
    va_end(args);

    return result_code(status, assigned);
}

int oversscanf(const char *str, const char *format, ...)
{
    if (str == NULL || format == NULL) {
        return result_code(STATUS_ERR_NULL_ARGUMENT, 0);
    }

    const Status format_status = check_format(format);
    if (format_status != STATUS_OK) {
        return result_code(format_status, 0);
    }

    Source source;
    source_from_text(&source, str);

    int assigned = 0;
    va_list args;
    va_start(args, format);
    const Status status = scan_all(&source, format, &args, &assigned);
    va_end(args);

    return result_code(status, assigned);
}

Status over_result_status(const int result)
{
    if (result >= 0) {
        return STATUS_OK;
    }
    if (result == EOF) {
        return STATUS_INPUT_END;
    }
    return (Status) -result;
}
