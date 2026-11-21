#ifndef SCRIPT_H
#define SCRIPT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "status.h"

typedef enum {
    COMMAND_ADD,
    COMMAND_SUB,
    COMMAND_MULT,
    COMMAND_DIV,
    COMMAND_MOD,
    COMMAND_EVAL,
    COMMAND_DIFF,
    COMMAND_CMPS
} CommandKind;

#define COMMAND_KIND_COUNT 8

typedef struct {
    CommandKind kind;
    char *args[2];
    size_t arg_count;
} Command;

typedef struct {
    char *text;
    size_t pos;
    size_t line;
} Cursor;

Status read_text(FILE *file, char **out_text);
Status strip_comments(char *text, size_t *out_line);
void cursor_init(Cursor *cursor, char *text);
Status next_command(Cursor *cursor, char **out_command, size_t *out_line, bool *out_found);
Status parse_command(char *command, Command *out_command);
const char *command_name(CommandKind kind);
size_t full_arg_count(CommandKind kind);

#endif
