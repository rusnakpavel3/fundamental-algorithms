#ifndef MY_STRING_H
#define MY_STRING_H

#include "status.h"

typedef struct {
    char *data;
    int length;
} String;

Status string_create(const char *text, String *out_string);
void string_clear(String *string);
Status string_compare_order(const String *first, const String *second, int *out_result);
Status string_compare_lex(const String *first, const String *second, int *out_result);
Status string_copy(String *destination, const String *source);
Status string_copy_new(const String *source, String **out_copy);
Status string_concat(String *destination, const String *source);

#endif
