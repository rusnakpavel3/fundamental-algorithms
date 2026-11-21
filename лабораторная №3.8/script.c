#include "script.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define START_TEXT_CAPACITY 256


static bool is_space(const char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}


static bool is_letter(const char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


static bool is_blank(const char *text)
{
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (!is_space(text[i])) {
            return false;
        }
    }
    return true;
}


Status read_text(FILE *file, char **out_text)
{
    if (file == NULL || out_text == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t capacity = START_TEXT_CAPACITY;
    char *text = (char *) malloc(capacity);
    if (text == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    size_t length = 0;
    Status status = STATUS_OK;
    int c = 0;
    while (status == STATUS_OK && (c = fgetc(file)) != EOF) {
        if (c == '\0') {
            status = STATUS_ERR_NUL_BYTE;
        } else if (length + 1 >= capacity) {
            if (capacity > SIZE_MAX / 2) {
                status = STATUS_ERR_SIZE_OVERFLOW;
            } else {
                char *grown = (char *) realloc(text, capacity * 2);
                if (grown == NULL) {
                    status = STATUS_ERR_ALLOC_FAILED;
                } else {
                    text = grown;
                    capacity *= 2;
                }
            }
        }
        if (status == STATUS_OK) {
            text[length] = (char) c;
            length++;
        }
    }
    if (status == STATUS_OK && ferror(file)) {
        status = STATUS_ERR_READ_FAILED;
    }
    if (status != STATUS_OK) {
        free(text);
        return status;
    }
    text[length] = '\0';
    *out_text = text;
    return STATUS_OK;
}


Status strip_comments(char *text, size_t *out_line)
{
    if (text == NULL || out_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t line = 1;
    size_t block_line = 0;
    bool in_line_comment = false;
    bool in_block_comment = false;
    for (size_t i = 0; text[i] != '\0'; i++) {
        const char c = text[i];
        if (c == '\n') {
            line++;
            in_line_comment = false;
        } else if (in_line_comment) {
            text[i] = ' ';
        } else if (in_block_comment) {
            if (c == '[') {
                *out_line = line;
                return STATUS_ERR_NESTED_COMMENT;
            }
            if (c == ']') {
                in_block_comment = false;
            }
            text[i] = ' ';
        } else if (c == '%') {
            in_line_comment = true;
            text[i] = ' ';
        } else if (c == '[') {
            in_block_comment = true;
            block_line = line;
            text[i] = ' ';
        } else if (c == ']') {
            *out_line = line;
            return STATUS_ERR_UNEXPECTED_BRACKET;
        }
    }
    if (in_block_comment) {
        *out_line = block_line;
        return STATUS_ERR_UNCLOSED_COMMENT;
    }
    return STATUS_OK;
}


void cursor_init(Cursor *cursor, char *text)
{
    if (cursor == NULL) {
        return;
    }
    cursor->text = text;
    cursor->pos = 0;
    cursor->line = 1;
}


Status next_command(Cursor *cursor, char **out_command, size_t *out_line, bool *out_found)
{
    if (cursor == NULL || cursor->text == NULL || out_command == NULL || out_line == NULL || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    char *text = cursor->text;
    *out_found = false;
    while (is_space(text[cursor->pos])) {
        if (text[cursor->pos] == '\n') {
            cursor->line++;
        }
        cursor->pos++;
    }
    if (text[cursor->pos] == '\0') {
        return STATUS_OK;
    }

    const size_t start = cursor->pos;
    *out_line = cursor->line;
    while (text[cursor->pos] != '\0' && text[cursor->pos] != ';') {
        if (text[cursor->pos] == '\n') {
            cursor->line++;
        }
        cursor->pos++;
    }
    if (text[cursor->pos] == '\0') {
        return STATUS_ERR_MISSING_SEMICOLON;
    }
    text[cursor->pos] = '\0';
    cursor->pos++;
    *out_command = &text[start];
    *out_found = true;
    return STATUS_OK;
}


const char *command_name(const CommandKind kind)
{
    switch (kind) {
        case COMMAND_ADD:
            return "Add";
        case COMMAND_SUB:
            return "Sub";
        case COMMAND_MULT:
            return "Mult";
        case COMMAND_DIV:
            return "Div";
        case COMMAND_MOD:
            return "Mod";
        case COMMAND_EVAL:
            return "Eval";
        case COMMAND_DIFF:
            return "Diff";
        case COMMAND_CMPS:
            return "Cmps";
    }
    return "?";
}


size_t full_arg_count(const CommandKind kind)
{
    if (kind == COMMAND_DIFF) {
        return 1;
    }
    return 2;
}


static Status find_kind(const char *name, const size_t length, CommandKind *out_kind)
{
    for (int k = 0; k < COMMAND_KIND_COUNT; k++) {
        const CommandKind kind = (CommandKind) k;
        const char *known = command_name(kind);
        if (strlen(known) == length && strncmp(name, known, length) == 0) {
            *out_kind = kind;
            return STATUS_OK;
        }
    }
    return STATUS_ERR_UNKNOWN_COMMAND;
}


Status parse_command(char *command, Command *out_command)
{
    if (command == NULL || out_command == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t pos = 0;
    while (is_space(command[pos])) {
        pos++;
    }
    const size_t name_start = pos;
    while (is_letter(command[pos])) {
        pos++;
    }
    Command result;
    Status status = find_kind(&command[name_start], pos - name_start, &result.kind);
    if (status != STATUS_OK) {
        return status;
    }

    while (is_space(command[pos])) {
        pos++;
    }
    if (command[pos] != '(') {
        return STATUS_ERR_EXPECTED_OPEN_PAREN;
    }
    pos++;
    const size_t args_start = pos;
    while (command[pos] != '\0' && command[pos] != ')') {
        pos++;
    }
    if (command[pos] != ')') {
        return STATUS_ERR_EXPECTED_CLOSE_PAREN;
    }
    command[pos] = '\0';
    if (!is_blank(&command[pos + 1])) {
        return STATUS_ERR_EXTRA_TEXT;
    }

    char *args = &command[args_start];
    result.arg_count = 0;
    result.args[0] = NULL;
    result.args[1] = NULL;
    if (!is_blank(args)) {
        result.args[0] = args;
        result.arg_count = 1;
        for (size_t i = 0; args[i] != '\0'; i++) {
            if (args[i] == ',') {
                if (result.arg_count == 2) {
                    return STATUS_ERR_WRONG_ARGUMENT_COUNT;
                }
                args[i] = '\0';
                result.args[1] = &args[i + 1];
                result.arg_count = 2;
            }
        }
        for (size_t a = 0; a < result.arg_count; a++) {
            if (is_blank(result.args[a])) {
                return STATUS_ERR_EMPTY_ARGUMENT;
            }
        }
    }

    const size_t full = full_arg_count(result.kind);
    if (result.arg_count != full && result.arg_count + 1 != full) {
        return STATUS_ERR_WRONG_ARGUMENT_COUNT;
    }
    *out_command = result;
    return STATUS_OK;
}
