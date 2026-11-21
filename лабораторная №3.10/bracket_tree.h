#ifndef BRACKET_TREE_H
#define BRACKET_TREE_H

#include <stddef.h>

#include "status.h"

#define MAX_TREE_DEPTH 1000

typedef struct Node {
    char *name;
    struct Node *first_child;
    struct Node *next_sibling;
} Node;

Status tree_parse(const char *line, Node **out_root, size_t *out_position);

void tree_free(Node *root);

#endif
