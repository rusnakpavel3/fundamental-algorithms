#include "text_reader.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


Status parse_separator(const char *argument, unsigned char *out_separator)
{
    if (argument == NULL || out_separator == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (argument[0] != '\0' && argument[1] == '\0') {
        *out_separator = (unsigned char) argument[0];
        return STATUS_OK;
    }
    if (argument[0] == '\\' && argument[1] != '\0' && argument[2] == '\0') {
        switch (argument[1]) {
            case 't':
                *out_separator = '\t';
                return STATUS_OK;
            case 's':
                *out_separator = ' ';
                return STATUS_OK;
            case '\\':
                *out_separator = '\\';
                return STATUS_OK;
            default:
                break;
        }
    }
    return STATUS_ERR_SEPARATOR_INVALID;
}


static Status grow_buffer(char **buffer, size_t *capacity)
{
    if (*capacity > SIZE_MAX / 2) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }
    const size_t new_capacity = *capacity * 2;
    char *grown = (char *) realloc(*buffer, new_capacity);
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    *buffer = grown;
    *capacity = new_capacity;
    return STATUS_OK;
}


static Status flush_word(WordNode **root, char *word, size_t *length, unsigned long long *total)
{
    if (*length == 0) {
        return STATUS_OK;
    }
    word[*length] = '\0';
    *length = 0;
    if (*total == ULLONG_MAX) {
        return STATUS_ERR_COUNT_OVERFLOW;
    }
    (*total)++;
    return tree_insert(root, word, 1, true);
}


Status read_words(const char *path, const bool separator[CHAR_VALUES], WordNode **out_root, unsigned long long *out_total)
{
    if (path == NULL || separator == NULL || out_root == NULL || out_total == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_root = NULL;
    *out_total = 0;

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return STATUS_ERR_FILE_OPEN_FAILED;
    }
    size_t capacity = 32;
    char *word = (char *) malloc(capacity);
    if (word == NULL) {
        fclose(file);
        return STATUS_ERR_ALLOC_FAILED;
    }

    WordNode *root = NULL;
    unsigned long long total = 0;
    size_t length = 0;
    Status status = STATUS_OK;
    int c = 0;
    while (status == STATUS_OK && (c = fgetc(file)) != EOF) {
        if (c == '\0') {
            status = STATUS_ERR_NUL_BYTE;
        } else if (separator[c]) {
            status = flush_word(&root, word, &length, &total);
        } else {
            if (length + 1 >= capacity) {
                status = grow_buffer(&word, &capacity);
            }
            if (status == STATUS_OK) {
                word[length] = (char) c;
                length++;
            }
        }
    }
    if (status == STATUS_OK && ferror(file)) {
        status = STATUS_ERR_READ_FAILED;
    }
    if (status == STATUS_OK) {
        status = flush_word(&root, word, &length, &total);
    }

    free(word);
    fclose(file);
    if (status != STATUS_OK) {
        tree_free(root);
        return status;
    }
    *out_root = root;
    *out_total = total;
    return STATUS_OK;
}
