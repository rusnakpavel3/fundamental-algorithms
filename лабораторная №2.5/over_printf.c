#include "over_printf.h"

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "custom_formats.h"
#include "text_buffer.h"

#define SPEC_MAX_LENGTH 64
#define CUSTOM_TEXT_SIZE DUMP_BUFFER_SIZE(sizeof(double))

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
    bool has_options;
    bool has_hash;
    bool has_zero;
    bool has_precision;
    LengthModifier modifier;
    char conversion;
} Spec;

typedef enum {
    CUSTOM_NONE,
    CUSTOM_ROMAN,
    CUSTOM_ZECKENDORF,
    CUSTOM_TO_BASE_LOWER,
    CUSTOM_TO_BASE_UPPER,
    CUSTOM_FROM_BASE_LOWER,
    CUSTOM_FROM_BASE_UPPER,
    CUSTOM_DUMP_INT,
    CUSTOM_DUMP_UINT,
    CUSTOM_DUMP_DOUBLE,
    CUSTOM_DUMP_FLOAT
} CustomKind;

static Status append_formatted(TextBuffer *out, const char *spec, ...)
{
    va_list args;
    va_list copy;

    va_start(args, spec);
    va_copy(copy, args);
    const int needed = vsnprintf(NULL, 0, spec, copy);
    va_end(copy);

    Status status = STATUS_OK;
    if (needed < 0) {
        status = STATUS_ERR_STANDARD_FORMAT_FAILED;
    } else {
        status = text_buffer_reserve(out, (size_t) needed);
    }

    if (status == STATUS_OK) {
        const int written = vsnprintf(out->data + out->length, (size_t) needed + 1, spec, args);
        if (written != needed) {
            status = STATUS_ERR_STANDARD_FORMAT_FAILED;
        } else {
            out->length += (size_t) needed;
        }
    }

    va_end(args);
    return status;
}

static Status append_text(TextBuffer *out, const char *text)
{
    return text_buffer_append(out, text, strlen(text));
}

static int normalize_base(const int base)
{
    return base_is_valid(base) ? base : BASE_DEFAULT;
}

static CustomKind custom_kind(const char *p)
{
    static const char *const NAMES[] = {"Ro", "Zr", "Cv", "CV", "to", "TO", "mi", "mu", "md", "mf"};
    static const CustomKind KINDS[] = {
        CUSTOM_ROMAN, CUSTOM_ZECKENDORF, CUSTOM_TO_BASE_LOWER, CUSTOM_TO_BASE_UPPER,
        CUSTOM_FROM_BASE_LOWER, CUSTOM_FROM_BASE_UPPER, CUSTOM_DUMP_INT, CUSTOM_DUMP_UINT,
        CUSTOM_DUMP_DOUBLE, CUSTOM_DUMP_FLOAT
    };

    if (p[0] == '\0') {
        return CUSTOM_NONE;
    }

    for (size_t i = 0; i < sizeof NAMES / sizeof NAMES[0]; i++) {
        if (p[0] == NAMES[i][0] && p[1] == NAMES[i][1]) {
            return KINDS[i];
        }
    }
    return CUSTOM_NONE;
}

static Status format_to_base(TextBuffer *out, const LetterCase letters, va_list *args)
{
    const int value = va_arg(*args, int);
    const int base = normalize_base(va_arg(*args, int));
    char text[CUSTOM_TEXT_SIZE];

    const Status status = int_to_base(value, base, letters, text, sizeof text);
    if (status != STATUS_OK) {
        return status;
    }
    return append_text(out, text);
}

static Status format_from_base(TextBuffer *out, const LetterCase letters, va_list *args)
{
    const char *number = va_arg(*args, char *);
    const int base = normalize_base(va_arg(*args, int));

    if (number == NULL) {
        return STATUS_ERR_POINTER_ARGUMENT_NULL;
    }

    long long value = 0;
    const Status status = base_to_long_long(number, base, letters, &value);
    if (status != STATUS_OK) {
        return status;
    }
    return append_formatted(out, "%lld", value);
}

static Status format_dump(TextBuffer *out, const CustomKind kind, va_list *args)
{
    char text[CUSTOM_TEXT_SIZE];
    Status status = STATUS_OK;

    if (kind == CUSTOM_DUMP_INT) {
        const int32_t value = va_arg(*args, int32_t);
        status = memory_dump(&value, sizeof value, text, sizeof text);
    } else if (kind == CUSTOM_DUMP_UINT) {
        const uint32_t value = va_arg(*args, uint32_t);
        status = memory_dump(&value, sizeof value, text, sizeof text);
    } else if (kind == CUSTOM_DUMP_DOUBLE) {
        const double value = va_arg(*args, double);
        status = memory_dump(&value, sizeof value, text, sizeof text);
    } else {
        const double promoted = va_arg(*args, double);
        if (isfinite(promoted) && fabs(promoted) > FLT_MAX) {
            return STATUS_ERR_FLOAT_OUT_OF_RANGE;
        }
        const float value = (float) promoted;
        status = memory_dump(&value, sizeof value, text, sizeof text);
    }

    if (status != STATUS_OK) {
        return status;
    }
    return append_text(out, text);
}

static Status format_custom(TextBuffer *out, const CustomKind kind, va_list *args)
{
    char text[CUSTOM_TEXT_SIZE];
    Status status = STATUS_OK;

    switch (kind) {
        case CUSTOM_ROMAN:
            status = roman_from_int(va_arg(*args, int), text, sizeof text);
            break;
        case CUSTOM_ZECKENDORF:
            status = zeckendorf_from_uint(va_arg(*args, unsigned int), text, sizeof text);
            break;
        case CUSTOM_TO_BASE_LOWER:
            return format_to_base(out, LETTERS_LOWER, args);
        case CUSTOM_TO_BASE_UPPER:
            return format_to_base(out, LETTERS_UPPER, args);
        case CUSTOM_FROM_BASE_LOWER:
            return format_from_base(out, LETTERS_LOWER, args);
        case CUSTOM_FROM_BASE_UPPER:
            return format_from_base(out, LETTERS_UPPER, args);
        case CUSTOM_DUMP_INT:
        case CUSTOM_DUMP_UINT:
        case CUSTOM_DUMP_DOUBLE:
        case CUSTOM_DUMP_FLOAT:
            return format_dump(out, kind, args);
        case CUSTOM_NONE:
            return STATUS_ERR_FORMAT_INVALID;
    }

    if (status != STATUS_OK) {
        return status;
    }
    return append_text(out, text);
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

static Status spec_append_int(Spec *spec, const int value)
{
    char digits[16];
    const int written = snprintf(digits, sizeof digits, "%d", value);
    if (written < 0 || (size_t) written >= sizeof digits) {
        return STATUS_ERR_STANDARD_FORMAT_FAILED;
    }

    Status status = STATUS_OK;
    for (int i = 0; i < written && status == STATUS_OK; i++) {
        status = spec_append(spec, digits[i]);
    }
    return status;
}

static Status spec_append_digits(Spec *spec, const char **cursor)
{
    Status status = STATUS_OK;
    int value = 0;

    while (status == STATUS_OK && isdigit((unsigned char) **cursor)) {
        const int digit = **cursor - '0';
        if (value > (INT_MAX - digit) / 10) {
            return STATUS_ERR_FORMAT_INVALID;
        }
        value = value * 10 + digit;

        status = spec_append(spec, **cursor);
        (*cursor)++;
    }
    return status;
}

static bool is_flag(const char c)
{
    return c == '-' || c == '+' || c == ' ' || c == '#' || c == '0';
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

static Status parse_spec(const char **cursor, va_list *args, Spec *spec)
{
    const char *p = *cursor + 1;

    spec->text[0] = '\0';
    spec->length = 0;
    spec->has_options = false;
    spec->has_hash = false;
    spec->has_zero = false;
    spec->has_precision = false;
    spec->modifier = LENGTH_NONE;
    spec->conversion = '\0';

    Status status = spec_append(spec, '%');

    while (status == STATUS_OK && is_flag(*p)) {
        spec->has_hash = spec->has_hash || *p == '#';
        spec->has_zero = spec->has_zero || *p == '0';
        status = spec_append(spec, *p);
        p++;
    }

    spec->has_options = spec->length > 1 || *p == '*' || *p == '.' || isdigit((unsigned char) *p);

    if (status == STATUS_OK && *p == '*') {
        const int width = va_arg(*args, int);
        p++;
        if (width == INT_MIN) {
            status = STATUS_ERR_FORMAT_INVALID;
        } else {
            status = spec_append_int(spec, width);
        }
    } else if (status == STATUS_OK) {
        status = spec_append_digits(spec, &p);
    }

    if (status == STATUS_OK && *p == '.') {
        p++;
        if (*p == '*') {
            const int precision = va_arg(*args, int);
            p++;
            spec->has_precision = true;
            if (precision >= 0) {
                status = spec_append(spec, '.');
                if (status == STATUS_OK) {
                    status = spec_append_int(spec, precision);
                }
            }
        } else {
            spec->has_precision = true;
            status = spec_append(spec, '.');
            if (status == STATUS_OK) {
                status = spec_append_digits(spec, &p);
            }
        }
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

    *cursor = p;
    return status;
}

static Status format_signed(TextBuffer *out, const Spec *spec, va_list *args)
{
    switch (spec->modifier) {
        case LENGTH_NONE:
        case LENGTH_HH:
        case LENGTH_H:
            return append_formatted(out, spec->text, va_arg(*args, int));
        case LENGTH_L:
            return append_formatted(out, spec->text, va_arg(*args, long));
        case LENGTH_LL:
            return append_formatted(out, spec->text, va_arg(*args, long long));
        case LENGTH_J:
            return append_formatted(out, spec->text, va_arg(*args, intmax_t));
        case LENGTH_Z:
            return append_formatted(out, spec->text, va_arg(*args, size_t));
        case LENGTH_T:
            return append_formatted(out, spec->text, va_arg(*args, ptrdiff_t));
        case LENGTH_BIG_L:
            break;
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status format_unsigned(TextBuffer *out, const Spec *spec, va_list *args)
{
    switch (spec->modifier) {
        case LENGTH_NONE:
        case LENGTH_HH:
        case LENGTH_H:
            return append_formatted(out, spec->text, va_arg(*args, unsigned int));
        case LENGTH_L:
            return append_formatted(out, spec->text, va_arg(*args, unsigned long));
        case LENGTH_LL:
            return append_formatted(out, spec->text, va_arg(*args, unsigned long long));
        case LENGTH_J:
            return append_formatted(out, spec->text, va_arg(*args, uintmax_t));
        case LENGTH_Z:
            return append_formatted(out, spec->text, va_arg(*args, size_t));
        case LENGTH_T:
            return append_formatted(out, spec->text, va_arg(*args, ptrdiff_t));
        case LENGTH_BIG_L:
            break;
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status format_floating(TextBuffer *out, const Spec *spec, va_list *args)
{
    if (spec->modifier == LENGTH_NONE || spec->modifier == LENGTH_L) {
        return append_formatted(out, spec->text, va_arg(*args, double));
    }
    if (spec->modifier == LENGTH_BIG_L) {
        return append_formatted(out, spec->text, va_arg(*args, long double));
    }
    return STATUS_ERR_FORMAT_INVALID;
}

static Status format_char(TextBuffer *out, const Spec *spec, va_list *args)
{
    if (spec->modifier == LENGTH_L) {
        return STATUS_ERR_FORMAT_UNSUPPORTED;
    }
    if (spec->modifier != LENGTH_NONE) {
        return STATUS_ERR_FORMAT_INVALID;
    }
    return append_formatted(out, spec->text, va_arg(*args, int));
}

static Status format_string(TextBuffer *out, const Spec *spec, va_list *args)
{
    if (spec->modifier == LENGTH_L) {
        return STATUS_ERR_FORMAT_UNSUPPORTED;
    }
    if (spec->modifier != LENGTH_NONE) {
        return STATUS_ERR_FORMAT_INVALID;
    }

    const char *text = va_arg(*args, char *);
    if (text == NULL) {
        return STATUS_ERR_POINTER_ARGUMENT_NULL;
    }
    return append_formatted(out, spec->text, text);
}

static Status format_pointer(TextBuffer *out, const Spec *spec, va_list *args)
{
    if (spec->modifier != LENGTH_NONE) {
        return STATUS_ERR_FORMAT_INVALID;
    }
    return append_formatted(out, spec->text, va_arg(*args, void *));
}

static Status store_count(const TextBuffer *out, const Spec *spec, va_list *args)
{
    if (spec->modifier == LENGTH_BIG_L) {
        return STATUS_ERR_FORMAT_INVALID;
    }
    if (spec->modifier != LENGTH_NONE) {
        return STATUS_ERR_FORMAT_UNSUPPORTED;
    }

    int *target = va_arg(*args, int *);
    if (target == NULL) {
        return STATUS_ERR_POINTER_ARGUMENT_NULL;
    }
    if (out->length > (size_t) INT_MAX) {
        return STATUS_ERR_OUTPUT_TOO_LONG;
    }

    *target = (int) out->length;
    return STATUS_OK;
}

static bool options_are_valid(const Spec *spec)
{
    const char c = spec->conversion;

    if (c == 'n') {
        return !spec->has_options;
    }
    if (spec->has_hash && strchr("oxXaAeEfFgG", c) == NULL) {
        return false;
    }
    if (spec->has_zero && strchr("diouxXaAeEfFgG", c) == NULL) {
        return false;
    }
    if (spec->has_precision && strchr("diouxXaAeEfFgGs", c) == NULL) {
        return false;
    }
    return true;
}

static Status format_standard(TextBuffer *out, const char **cursor, va_list *args)
{
    Spec spec;
    const Status status = parse_spec(cursor, args, &spec);
    if (status != STATUS_OK) {
        return status;
    }
    if (spec.conversion == '\0' || !options_are_valid(&spec)) {
        return STATUS_ERR_FORMAT_INVALID;
    }

    switch (spec.conversion) {
        case 'd':
        case 'i':
            return format_signed(out, &spec, args);
        case 'u':
        case 'o':
        case 'x':
        case 'X':
            return format_unsigned(out, &spec, args);
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'a':
        case 'A':
            return format_floating(out, &spec, args);
        case 'c':
            return format_char(out, &spec, args);
        case 's':
            return format_string(out, &spec, args);
        case 'p':
            return format_pointer(out, &spec, args);
        case 'n':
            return store_count(out, &spec, args);
        default:
            return STATUS_ERR_FORMAT_INVALID;
    }
}

static Status format_all(TextBuffer *out, const char *format, va_list *args)
{
    const char *p = format;
    Status status = STATUS_OK;

    while (*p != '\0' && status == STATUS_OK) {
        if (*p != '%') {
            status = text_buffer_append_char(out, *p);
            p++;
        } else if (p[1] == '%') {
            status = text_buffer_append_char(out, '%');
            p += 2;
        } else {
            const CustomKind kind = custom_kind(p + 1);
            if (kind != CUSTOM_NONE) {
                status = format_custom(out, kind, args);
                p += 3;
            } else {
                status = format_standard(out, &p, args);
            }
        }
    }

    if (status == STATUS_OK && out->length > (size_t) INT_MAX) {
        status = STATUS_ERR_OUTPUT_TOO_LONG;
    }
    return status;
}

static int result_code(const Status status, const size_t length)
{
    if (status != STATUS_OK) {
        return -(int) status;
    }
    return (int) length;
}

int overfprintf(FILE *stream, const char *format, ...)
{
    if (stream == NULL || format == NULL) {
        return result_code(STATUS_ERR_NULL_ARGUMENT, 0);
    }

    TextBuffer buffer;
    Status status = text_buffer_init(&buffer);
    if (status != STATUS_OK) {
        return result_code(status, 0);
    }

    va_list args;
    va_start(args, format);
    status = format_all(&buffer, format, &args);
    va_end(args);

    if (status == STATUS_OK && buffer.length > 0
        && fwrite(buffer.data, 1, buffer.length, stream) != buffer.length) {
        status = STATUS_ERR_WRITE_FAILED;
    }

    const int result = result_code(status, buffer.length);
    text_buffer_free(&buffer);
    return result;
}

int oversprintf(char *str, const char *format, ...)
{
    if (str == NULL || format == NULL) {
        return result_code(STATUS_ERR_NULL_ARGUMENT, 0);
    }

    TextBuffer buffer;
    Status status = text_buffer_init(&buffer);
    if (status != STATUS_OK) {
        str[0] = '\0';
        return result_code(status, 0);
    }

    va_list args;
    va_start(args, format);
    status = format_all(&buffer, format, &args);
    va_end(args);

    if (status == STATUS_OK) {
        memcpy(str, buffer.data, buffer.length + 1);
    } else {
        str[0] = '\0';
    }

    const int result = result_code(status, buffer.length);
    text_buffer_free(&buffer);
    return result;
}

Status over_result_status(const int result)
{
    if (result >= 0) {
        return STATUS_OK;
    }
    return (Status) -result;
}
