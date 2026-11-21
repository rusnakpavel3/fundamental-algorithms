#include "bracket_tree.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void skip_spaces(const char **cursor)
{
    while (**cursor != '\0' && isspace((unsigned char) **cursor)) {
        (*cursor)++;
    }
}


static bool is_name_char(const char c)
{
    return c != '\0' && c != '(' && c != ')' && c != ',' && !isspace((unsigned char) c);
}


static Status node_create(const char *start, const size_t length, Node **out_node)
{
    Node *node = (Node *) malloc(sizeof(Node));
    if (node == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    node->name = (char *) malloc(length + 1);
    if (node->name == NULL) {
        free(node);
        return STATUS_ERR_ALLOC_FAILED;
    }
    memcpy(node->name, start, length);
    node->name[length] = '\0';
    node->first_child = NULL;
    node->next_sibling = NULL;

    *out_node = node;
    return STATUS_OK;
}


static Status parse_node(const char **cursor, const size_t depth, Node **out_node)
{
    skip_spaces(cursor);
    if (depth > MAX_TREE_DEPTH) {
        return STATUS_ERR_TOO_DEEP;
    }

    const char *start = *cursor;
    while (is_name_char(**cursor)) {
        (*cursor)++;
    }
    if (*cursor == start) {
        return STATUS_ERR_EXPECTED_NAME;
    }

    Node *node = NULL;
    Status status = node_create(start, (size_t) (*cursor - start), &node);
    if (status != STATUS_OK) {
        return status;
    }

    skip_spaces(cursor);
    if (**cursor == '(') {
        (*cursor)++;
        Node *last = NULL;
        bool more = true;
        while (status == STATUS_OK && more) {
            Node *child = NULL;
            status = parse_node(cursor, depth + 1, &child);
            if (status == STATUS_OK) {
                if (last == NULL) {
                    node->first_child = child;
                } else {
                    last->next_sibling = child;
                }
                last = child;

                skip_spaces(cursor);
                if (**cursor == ',') {
                    (*cursor)++;
                } else if (**cursor == ')') {
                    (*cursor)++;
                    more = false;
                } else {
                    status = STATUS_ERR_EXPECTED_SEPARATOR;
                }
            }
        }
    }

    if (status != STATUS_OK) {
        tree_free(node);
        return status;
    }
    *out_node = node;
    return STATUS_OK;
}


static size_t char_position(const char *line, const char *cursor)
{
    size_t position = 1;
    for (const char *p = line; p < cursor; p++) {
        if (((unsigned char) *p & 0xC0) != 0x80) {
            position++;
        }
    }
    return position;
}


Status tree_parse(const char *line, Node **out_root, size_t *out_position)
{
    if (line == NULL || out_root == NULL || out_position == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_root = NULL;
    *out_position = 0;

    const char *cursor = line;
    skip_spaces(&cursor);
    if (*cursor == '\0') {
        return STATUS_ERR_EMPTY_LINE;
    }

    Node *root = NULL;
    Status status = parse_node(&cursor, 1, &root);
    if (status == STATUS_OK) {
        skip_spaces(&cursor);
        if (*cursor != '\0') {
            status = STATUS_ERR_TRAILING_TEXT;
            tree_free(root);
        }
    }

    if (status != STATUS_OK) {
        *out_position = char_position(line, cursor);
        return status;
    }
    *out_root = root;
    return STATUS_OK;
}


void tree_free(Node *root)
{
    Node *node = root;
    while (node != NULL) {
        Node *next = node->next_sibling;
        tree_free(node->first_child);
        free(node->name);
        free(node);
        node = next;
    }
}
