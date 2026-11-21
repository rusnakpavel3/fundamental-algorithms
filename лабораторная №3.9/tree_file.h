#ifndef TREE_FILE_H
#define TREE_FILE_H

#include <stddef.h>

#include "status.h"
#include "word_tree.h"

Status tree_save(const WordNode *root, const char *path, const char *protected_path);
Status tree_load(const char *path, WordNode **out_root, size_t *out_line);

#endif
