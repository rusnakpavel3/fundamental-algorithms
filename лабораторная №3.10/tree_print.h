#ifndef TREE_PRINT_H
#define TREE_PRINT_H

#include <stdio.h>

#include "bracket_tree.h"
#include "status.h"

Status tree_write(FILE *file, const Node *root);

#endif
