#include "file_io.h"

#include <stdio.h>

#define BLOCK_SIZE 4096


Status read_frequencies(const char *path, unsigned long long frequency[BYTE_VALUES])
{
    if (path == NULL || frequency == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    for (int b = 0; b < BYTE_VALUES; b++) {
        frequency[b] = 0;
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return STATUS_ERR_FILE_OPEN_FAILED;
    }

    unsigned char block[BLOCK_SIZE];
    Status status = STATUS_OK;
    size_t got = 0;
    while (status == STATUS_OK && (got = fread(block, 1, sizeof block, file)) > 0) {
        status = count_bytes(block, got, frequency);
    }
    if (status == STATUS_OK && ferror(file)) {
        status = STATUS_ERR_READ_FAILED;
    }

    fclose(file);
    return status;
}
