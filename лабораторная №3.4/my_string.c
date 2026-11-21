#include "my_string.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static bool is_valid(const String *string)
{
    return string != NULL && string->data != NULL && string->length >= 0;
}


static int sign_of(const int value)
{
    if (value < 0) {
        return -1;
    }
    if (value > 0) {
        return 1;
    }
    return 0;
}


Status string_create(const char *text, String *out_string)
{
    if (text == NULL || out_string == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const size_t length = strlen(text);
    if (length > INT_MAX) {
        return STATUS_ERR_STRING_TOO_LONG;
    }

    char *data = (char *) malloc(length + 1);
    if (data == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i <= length; i++) {
        data[i] = text[i];
    }

    out_string->data = data;
    out_string->length = (int) length;
    return STATUS_OK;
}


void string_clear(String *string)
{
    if (string == NULL) {
        return;
    }
    free(string->data);
    string->data = NULL;
    string->length = 0;
}


Status string_compare_order(const String *first, const String *second, int *out_result)
{
    if (!is_valid(first) || !is_valid(second) || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (first->length < second->length) {
        *out_result = -1;
    } else if (first->length > second->length) {
        *out_result = 1;
    } else {
        *out_result = sign_of(strcmp(first->data, second->data));
    }
    return STATUS_OK;
}


Status string_compare_lex(const String *first, const String *second, int *out_result)
{
    if (!is_valid(first) || !is_valid(second) || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_result = sign_of(strcmp(first->data, second->data));
    return STATUS_OK;
}


Status string_copy(String *destination, const String *source)
{
    if (destination == NULL || !is_valid(source)) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    const size_t length = (size_t) source->length;
    char *data = (char *) malloc(length + 1);
    if (data == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i <= length; i++) {
        data[i] = source->data[i];
    }

    free(destination->data);
    destination->data = data;
    destination->length = source->length;
    return STATUS_OK;
}


Status string_copy_new(const String *source, String **out_copy)
{
    if (!is_valid(source) || out_copy == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    String *copy = (String *) malloc(sizeof(String));
    if (copy == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    const Status status = string_create(source->data, copy);
    if (status != STATUS_OK) {
        free(copy);
        return status;
    }

    *out_copy = copy;
    return STATUS_OK;
}


Status string_concat(String *destination, const String *source)
{
    if (!is_valid(destination) || !is_valid(source)) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (destination->length > INT_MAX - source->length) {
        return STATUS_ERR_STRING_TOO_LONG;
    }

    const size_t first_length = (size_t) destination->length;
    const size_t second_length = (size_t) source->length;
    char *data = (char *) malloc(first_length + second_length + 1);
    if (data == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i < first_length; i++) {
        data[i] = destination->data[i];
    }
    for (size_t i = 0; i < second_length; i++) {
        data[first_length + i] = source->data[i];
    }
    data[first_length + second_length] = '\0';

    free(destination->data);
    destination->data = data;
    destination->length = (int) (first_length + second_length);
    return STATUS_OK;
}
