#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "input.h"
#include "schedule.h"
#include "status.h"

static void print_input_error(const Status status, const size_t line)
{
    switch (status) {
        case STATUS_ERR_HEADER_FORMAT:
        case STATUS_ERR_LINE_FORMAT:
        case STATUS_ERR_LINE_TOO_LONG:
        case STATUS_ERR_NUL_CHARACTER:
        case STATUS_ERR_NUMBER_INVALID:
        case STATUS_ERR_NUMBER_NOT_POSITIVE:
        case STATUS_ERR_NUMBER_TOO_BIG:
        case STATUS_ERR_TIME_FORMAT:
        case STATUS_ERR_TIME_ORDER:
            printf("Ошибка во входных данных (строка %zu): %s\n", line, status_message(status));
            break;
        case STATUS_OK:
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_HALLS_INVALID:
        case STATUS_ERR_ID_INVALID:
        case STATUS_ERR_TIME_INVALID:
        case STATUS_ERR_DUPLICATE_ID:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_INPUT_EMPTY:
        case STATUS_ERR_NOT_ENOUGH_LINES:
        case STATUS_ERR_READ_FAILED:
            printf("Ошибка: %s\n", status_message(status));
            break;
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    (void) argv;

    if (argc != 1) {
        printf("Ошибка: программа не принимает аргументов, данные читаются из стандартного ввода\n");
        return 1;
    }

    Talk *talks = NULL;
    size_t count = 0;
    long long halls = 0;
    size_t line = 0;
    Status status = read_input(stdin, &talks, &count, &halls, &line);
    if (status != STATUS_OK) {
        print_input_error(status, line);
        return 1;
    }

    size_t answer = 0;
    status = max_talks(talks, count, halls, &answer);
    free(talks);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 1;
    }

    printf("%zu\n", answer);
    return 0;
}
