#include "huffman.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    unsigned long long weight;
    int min_byte;
    int byte;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *items[BYTE_VALUES];
    int size;
} Heap;


static Node *new_node(const unsigned long long weight, const int min_byte, const int byte, Node *left, Node *right)
{
    Node *node = (Node *) malloc(sizeof *node);
    if (node == NULL) {
        return NULL;
    }
    node->weight = weight;
    node->min_byte = min_byte;
    node->byte = byte;
    node->left = left;
    node->right = right;
    return node;
}


static void free_tree(Node *node)
{
    if (node == NULL) {
        return;
    }
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}


static bool node_less(const Node *a, const Node *b)
{
    if (a->weight != b->weight) {
        return a->weight < b->weight;
    }
    return a->min_byte < b->min_byte;
}


static void swap_nodes(Node **a, Node **b)
{
    Node *tmp = *a;
    *a = *b;
    *b = tmp;
}


static void heap_push(Heap *heap, Node *node)
{
    int i = heap->size;
    heap->items[i] = node;
    heap->size++;

    while (i > 0) {
        const int parent = (i - 1) / 2;
        if (!node_less(heap->items[i], heap->items[parent])) {
            break;
        }
        swap_nodes(&heap->items[i], &heap->items[parent]);
        i = parent;
    }
}


static Node *heap_pop(Heap *heap)
{
    Node *top = heap->items[0];
    heap->size--;
    heap->items[0] = heap->items[heap->size];

    int i = 0;
    while (true) {
        const int left = 2 * i + 1;
        const int right = 2 * i + 2;
        int smallest = i;
        if (left < heap->size && node_less(heap->items[left], heap->items[smallest])) {
            smallest = left;
        }
        if (right < heap->size && node_less(heap->items[right], heap->items[smallest])) {
            smallest = right;
        }
        if (smallest == i) {
            break;
        }
        swap_nodes(&heap->items[i], &heap->items[smallest]);
        i = smallest;
    }
    return top;
}


static void assign_codes(const Node *node, char *path, const int depth, HuffmanResult *out)
{
    if (node->left == NULL) {
        path[depth] = '\0';
        strcpy(out->code[node->byte], path);
        return;
    }
    path[depth] = '0';
    assign_codes(node->left, path, depth + 1, out);
    path[depth] = '1';
    assign_codes(node->right, path, depth + 1, out);
}


Status count_bytes(const unsigned char *block, const size_t size, unsigned long long frequency[BYTE_VALUES])
{
    if ((block == NULL && size > 0) || frequency == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    for (size_t i = 0; i < size; i++) {
        if (frequency[block[i]] == ULLONG_MAX) {
            return STATUS_ERR_FILE_TOO_LARGE;
        }
        frequency[block[i]]++;
    }
    return STATUS_OK;
}


Status build_huffman(const unsigned long long frequency[BYTE_VALUES], HuffmanResult *out)
{
    if (frequency == NULL || out == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    out->original_bytes = 0;
    out->encoded_bits = 0;
    out->encoded_bytes = 0;
    out->savings_percent = 0.0;
    for (int b = 0; b < BYTE_VALUES; b++) {
        out->frequency[b] = frequency[b];
        out->code[b][0] = '\0';
        if (frequency[b] > ULLONG_MAX - out->original_bytes) {
            return STATUS_ERR_FILE_TOO_LARGE;
        }
        out->original_bytes += frequency[b];
    }
    if (out->original_bytes == 0) {
        return STATUS_OK;
    }

    Heap heap;
    heap.size = 0;
    Status status = STATUS_OK;
    for (int b = 0; b < BYTE_VALUES && status == STATUS_OK; b++) {
        if (frequency[b] > 0) {
            Node *leaf = new_node(frequency[b], b, b, NULL, NULL);
            if (leaf == NULL) {
                status = STATUS_ERR_ALLOC_FAILED;
            } else {
                heap_push(&heap, leaf);
            }
        }
    }

    while (status == STATUS_OK && heap.size > 1) {
        Node *first = heap_pop(&heap);
        Node *second = heap_pop(&heap);
        const int min_byte = first->min_byte < second->min_byte ? first->min_byte : second->min_byte;
        Node *parent = new_node(first->weight + second->weight, min_byte, -1, first, second);
        if (parent == NULL) {
            free_tree(first);
            free_tree(second);
            status = STATUS_ERR_ALLOC_FAILED;
        } else {
            heap_push(&heap, parent);
        }
    }

    if (status != STATUS_OK) {
        while (heap.size > 0) {
            free_tree(heap_pop(&heap));
        }
        return status;
    }

    Node *root = heap_pop(&heap);
    if (root->left == NULL) {
        strcpy(out->code[root->byte], "0");
    } else {
        char path[MAX_CODE_LENGTH + 1];
        assign_codes(root, path, 0, out);
    }
    free_tree(root);

    for (int b = 0; b < BYTE_VALUES; b++) {
        if (frequency[b] > 0) {
            const unsigned long long length = (unsigned long long) strlen(out->code[b]);
            if (frequency[b] > (ULLONG_MAX - out->encoded_bits) / length) {
                return STATUS_ERR_SIZE_OVERFLOW;
            }
            out->encoded_bits += frequency[b] * length;
        }
    }
    out->encoded_bytes = out->encoded_bits / 8 + (out->encoded_bits % 8 != 0);
    out->savings_percent = 100.0 * (double) (out->original_bytes - out->encoded_bytes) / (double) out->original_bytes;
    return STATUS_OK;
}
