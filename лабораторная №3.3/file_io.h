#ifndef FILE_IO_H
#define FILE_IO_H

#include "huffman.h"
#include "status.h"

Status read_frequencies(const char *path, unsigned long long frequency[BYTE_VALUES]);

#endif
