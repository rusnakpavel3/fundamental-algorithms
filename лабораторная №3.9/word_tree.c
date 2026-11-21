#include "word_tree.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static size_t utf8_length(const char *word)
{
    size_t length = 0;
    for (const char *p = word; *p != '\0'; p++) {
        if (((unsigned char) *p & 0xC0) != 0x80) {
            length++;
        }
    }
    return length;
}


static WordNode *new_node(const char *word, const unsigned long long count)
{
    const size_t size = strlen(word) + 1;
    WordNode *node = (WordNode *) malloc(sizeof *node);
    if (node == NULL) {
        return NULL;
    }
    node->word = (char *) malloc(size);
    if (node->word == NULL) {
        free(node);
        return NULL;
    }
    memcpy(node->word, word, size);
    node->length = utf8_length(word);
    node->count = count;
    node->left = NULL;
    node->right = NULL;
    return node;
}


Status tree_insert(WordNode **root, const char *word, const unsigned long long count, const bool merge)
{
    if (root == NULL || word == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (word[0] == '\0') {
        return STATUS_ERR_INPUT_EMPTY;
    }
    if (count == 0) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    WordNode **link = root;
    size_t depth = 1;
    while (*link != NULL) {
        const int compare = strcmp(word, (*link)->word);
        if (compare == 0) {
            if (!merge) {
                return STATUS_ERR_WORD_DUPLICATE;
            }
            if ((*link)->count > ULLONG_MAX - count) {
                return STATUS_ERR_COUNT_OVERFLOW;
            }
            (*link)->count += count;
            return STATUS_OK;
        }
        link = compare < 0 ? &(*link)->left : &(*link)->right;
        depth++;
    }

    if (depth > MAX_TREE_DEPTH) {
        return STATUS_ERR_TREE_TOO_DEEP;
    }
    WordNode *node = new_node(word, count);
    if (node == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    *link = node;
    return STATUS_OK;
}


Status tree_find(const WordNode *root, const char *word, unsigned long long *out_count)
{
    if (word == NULL || out_count == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_count = 0;

    const WordNode *node = root;
    while (node != NULL) {
        const int compare = strcmp(word, node->word);
        if (compare == 0) {
            *out_count = node->count;
            return STATUS_OK;
        }
        node = compare < 0 ? node->left : node->right;
    }
    return STATUS_OK;
}


static size_t count_nodes(const WordNode *node)
{
    if (node == NULL) {
        return 0;
    }
    return 1 + count_nodes(node->left) + count_nodes(node->right);
}


static size_t depth_of(const WordNode *node)
{
    if (node == NULL) {
        return 0;
    }
    const size_t left = depth_of(node->left);
    const size_t right = depth_of(node->right);
    return 1 + (left > right ? left : right);
}


Status tree_size(const WordNode *root, size_t *out_size)
{
    if (out_size == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_size = count_nodes(root);
    return STATUS_OK;
}


Status tree_depth(const WordNode *root, size_t *out_depth)
{
    if (out_depth == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_depth = depth_of(root);
    return STATUS_OK;
}


static void fill_in_order(const WordNode *node, const WordNode **items, size_t *index)
{
    if (node == NULL) {
        return;
    }
    fill_in_order(node->left, items, index);
    items[*index] = node;
    (*index)++;
    fill_in_order(node->right, items, index);
}


static int compare_by_frequency(const void *a, const void *b)
{
    const WordNode *x = *(const WordNode *const *) a;
    const WordNode *y = *(const WordNode *const *) b;
    if (x->count != y->count) {
        return x->count > y->count ? -1 : 1;
    }
    return strcmp(x->word, y->word);
}


Status tree_top_words(const WordNode *root, const size_t n, WordList *out_list)
{
    if (out_list == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    out_list->items = NULL;
    out_list->count = 0;
    out_list->total = 0;
    if (n == 0) {
        return STATUS_ERR_NUMBER_INVALID;
    }

    const size_t total = count_nodes(root);
    if (total == 0) {
        return STATUS_ERR_TREE_EMPTY;
    }
    if (total > SIZE_MAX / sizeof(const WordNode *)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }
    const WordNode **items = (const WordNode **) malloc(total * sizeof *items);
    if (items == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    size_t index = 0;
    fill_in_order(root, items, &index);
    qsort(items, total, sizeof *items, compare_by_frequency);

    out_list->items = items;
    out_list->total = total;
    out_list->count = n < total ? n : total;
    return STATUS_OK;
}


static void find_extremes(const WordNode *node, const WordNode **longest, const WordNode **shortest)
{
    if (node == NULL) {
        return;
    }
    find_extremes(node->left, longest, shortest);
    if (*longest == NULL || node->length > (*longest)->length) {
        *longest = node;
    }
    if (*shortest == NULL || node->length < (*shortest)->length) {
        *shortest = node;
    }
    find_extremes(node->right, longest, shortest);
}


Status tree_extremes(const WordNode *root, const WordNode **out_longest, const WordNode **out_shortest)
{
    if (out_longest == NULL || out_shortest == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_longest = NULL;
    *out_shortest = NULL;
    if (root == NULL) {
        return STATUS_ERR_TREE_EMPTY;
    }
    find_extremes(root, out_longest, out_shortest);
    return STATUS_OK;
}


static bool same_tree(const WordNode *first, const WordNode *second)
{
    if (first == NULL || second == NULL) {
        return first == second;
    }
    return first->count == second->count
           && strcmp(first->word, second->word) == 0
           && same_tree(first->left, second->left)
           && same_tree(first->right, second->right);
}


Status tree_equal(const WordNode *first, const WordNode *second, bool *out_equal)
{
    if (out_equal == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_equal = same_tree(first, second);
    return STATUS_OK;
}


void word_list_free(WordList *list)
{
    if (list == NULL) {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->total = 0;
}


void tree_free(WordNode *root)
{
    if (root == NULL) {
        return;
    }
    tree_free(root->left);
    tree_free(root->right);
    free(root->word);
    free(root);
}
