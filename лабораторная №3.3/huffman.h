#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stddef.h>

#include "status.h"

#define BYTE_VALUES 256
#define MAX_CODE_LENGTH 255

typedef struct {
    unsigned long long frequency[BYTE_VALUES];
    char code[BYTE_VALUES][MAX_CODE_LENGTH + 1];
    unsigned long long original_bytes;
    unsigned long long encoded_bits;
    unsigned long long encoded_bytes;
    double savings_percent;
} HuffmanResult;

Status count_bytes(const unsigned char *block, const size_t size, unsigned long long frequency[BYTE_VALUES]);
Status build_huffman(const unsigned long long frequency[BYTE_VALUES], HuffmanResult *out);

#endif
