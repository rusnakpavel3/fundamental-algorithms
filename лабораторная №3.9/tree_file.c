#include "tree_file.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "same_file.h"


static Status write_node(FILE *file, const WordNode *node)
{
    if (node == NULL) {
        return STATUS_OK;
    }
    if (fprintf(file, "%llu %s\n", node->count, node->word) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    const Status status = write_node(file, node->left);
    if (status != STATUS_OK) {
        return status;
    }
    return write_node(file, node->right);
}


Status tree_save(const WordNode *root, const char *path, const char *protected_path)
{
    if (path == NULL || protected_path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (path[0] == '\0') {
        return STATUS_ERR_INPUT_EMPTY;
    }

    FILE *protected_file = fopen(protected_path, "rb");
    if (protected_file != NULL) {
        bool same = false;
        const Status status = file_matches_path(protected_file, path, &same);
        fclose(protected_file);
        if (status != STATUS_OK) {
            return status;
        }
        if (same) {
            return STATUS_ERR_SAME_FILE;
        }
    }

    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }
    Status status = write_node(file, root);
    if (fclose(file) == EOF && status == STATUS_OK) {
        status = STATUS_ERR_CLOSE_FAILED;
    }
    return status;
}


static Status read_line(FILE *file, char **line, size_t *capacity, bool *out_has_line)
{
    size_t length = 0;
    int c = 0;

    *out_has_line = false;
    while ((c = fgetc(file)) != EOF && c != '\n') {
        *out_has_line = true;
        if (c == '\0') {
            return STATUS_ERR_TREE_FILE_FORMAT;
        }
        if (length + 1 >= *capacity) {
            if (*capacity > SIZE_MAX / 2) {
                return STATUS_ERR_SIZE_OVERFLOW;
            }
            char *grown = (char *) realloc(*line, *capacity * 2);
            if (grown == NULL) {
                return STATUS_ERR_ALLOC_FAILED;
            }
            *line = grown;
            *capacity *= 2;
        }
        (*line)[length] = (char) c;
        length++;
    }
    if (ferror(file)) {
        return STATUS_ERR_READ_FAILED;
    }
    if (c == '\n') {
        *out_has_line = true;
    }
    if (length > 0 && (*line)[length - 1] == '\r') {
        length--;
    }
    (*line)[length] = '\0';
    return STATUS_OK;
}


static Status parse_line(char *line, unsigned long long *out_count, char **out_word)
{
    unsigned long long count = 0;
    size_t i = 0;

    if (!isdigit((unsigned char) line[0])) {
        return STATUS_ERR_TREE_FILE_FORMAT;
    }
    while (isdigit((unsigned char) line[i])) {
        const unsigned long long digit = (unsigned long long) (line[i] - '0');
        if (count > (ULLONG_MAX - digit) / 10) {
            return STATUS_ERR_COUNT_OVERFLOW;
        }
        count = count * 10 + digit;
        i++;
    }
    if (count == 0 || line[i] != ' ' || line[i + 1] == '\0') {
        return STATUS_ERR_TREE_FILE_FORMAT;
    }

    *out_count = count;
    *out_word = &line[i + 1];
    return STATUS_OK;
}


Status tree_load(const char *path, WordNode **out_root, size_t *out_line)
{
    if (path == NULL || out_root == NULL || out_line == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_root = NULL;
    *out_line = 0;
    if (path[0] == '\0') {
        return STATUS_ERR_INPUT_EMPTY;
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return STATUS_ERR_FILE_OPEN_FAILED;
    }
    size_t capacity = 64;
    char *line = (char *) malloc(capacity);
    if (line == NULL) {
        fclose(file);
        return STATUS_ERR_ALLOC_FAILED;
    }

    WordNode *root = NULL;
    size_t number = 0;
    bool has_line = true;
    Status status = STATUS_OK;
    while (status == STATUS_OK && has_line) {
        number++;
        status = read_line(file, &line, &capacity, &has_line);
        if (status == STATUS_OK && has_line) {
            unsigned long long count = 0;
            char *word = NULL;
            status = parse_line(line, &count, &word);
            if (status == STATUS_OK) {
                status = tree_insert(&root, word, count, false);
            }
        }
    }

    free(line);
    fclose(file);
    if (status != STATUS_OK) {
        tree_free(root);
        *out_line = number;
        return status;
    }
    *out_root = root;
    return STATUS_OK;
}
