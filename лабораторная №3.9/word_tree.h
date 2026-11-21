#ifndef WORD_TREE_H
#define WORD_TREE_H

#include <stdbool.h>
#include <stddef.h>

#include "status.h"

#define MAX_TREE_DEPTH 1000

typedef struct WordNode {
    char *word;
    size_t length;
    unsigned long long count;
    struct WordNode *left;
    struct WordNode *right;
} WordNode;

typedef struct {
    const WordNode **items;
    size_t count;
    size_t total;
} WordList;

Status tree_insert(WordNode **root, const char *word, const unsigned long long count, const bool merge);
Status tree_find(const WordNode *root, const char *word, unsigned long long *out_count);
Status tree_size(const WordNode *root, size_t *out_size);
Status tree_depth(const WordNode *root, size_t *out_depth);
Status tree_top_words(const WordNode *root, const size_t n, WordList *out_list);
Status tree_extremes(const WordNode *root, const WordNode **out_longest, const WordNode **out_shortest);
Status tree_equal(const WordNode *first, const WordNode *second, bool *out_equal);
void word_list_free(WordList *list);
void tree_free(WordNode *root);

#endif
