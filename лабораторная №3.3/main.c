#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "file_io.h"
#include "huffman.h"
#include "status.h"


static void print_error(const Status status, const char *path)
{
    switch (status) {
        case STATUS_ERR_FILE_OPEN_FAILED:
        case STATUS_ERR_READ_FAILED:
            printf("Ошибка: %s: %s\n", status_message(status), path);
            break;
        case STATUS_OK:
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_ARGUMENT_COUNT:
        case STATUS_ERR_FILE_TOO_LARGE:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
            printf("Ошибка: %s\n", status_message(status));
            break;
    }
}


static void print_result(const HuffmanResult *result)
{
    printf(" Байт       Частота  Код\n");
    for (int b = 0; b < BYTE_VALUES; b++) {
        if (result->frequency[b] > 0) {
            printf("%5d  %12llu  %s\n", b, result->frequency[b], result->code[b]);
        }
    }
    printf("\n");
    printf("Размер исходного файла N: %llu байт\n", result->original_bytes);
    printf("Размер закодированных данных: %llu бит\n", result->encoded_bits);
    printf("Размер закодированных данных M: %llu байт\n", result->encoded_bytes);
    printf("Экономия 100 * (1 - M / N): %.2f%%\n", result->savings_percent);
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 2) {
        print_error(STATUS_ERR_ARGUMENT_COUNT, NULL);
        return 1;
    }

    unsigned long long frequency[BYTE_VALUES];
    Status status = read_frequencies(argv[1], frequency);
    if (status != STATUS_OK) {
        print_error(status, argv[1]);
        return 1;
    }

    HuffmanResult *result = (HuffmanResult *) malloc(sizeof *result);
    if (result == NULL) {
        print_error(STATUS_ERR_ALLOC_FAILED, argv[1]);
        return 1;
    }

    status = build_huffman(frequency, result);
    if (status == STATUS_OK) {
        print_result(result);
    } else {
        print_error(status, argv[1]);
    }

    free(result);
    return status == STATUS_OK ? 0 : 1;
}
