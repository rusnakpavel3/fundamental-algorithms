#ifdef _WIN32
#include <windows.h>
#endif

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "paths.h"
#include "status.h"
#include "tokens.h"

static void print_usage(void)
{
    fprintf(stderr,
            "Использование:\n"
            "  program -r <file1> <file2> <выходной файл>\n"
            "      лексемы из file1 и file2 поочерёдно\n"
            "  program -a <входной файл> <выходной файл>\n"
            "      каждая 10-я лексема - строчные буквы и коды по основанию 4,\n"
            "      каждая 2-я - строчные буквы, каждая 5-я - коды по основанию 8\n"
            "Вместо '-' можно использовать '/'.\n");
}

static void print_error(const char *context, const Status status)
{
    fprintf(stderr, "Ошибка: %s: %s\n", context, status_message(status));
}

static Status parse_flag(const char *str, char *out_letter)
{
    if (str == NULL || out_letter == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (str[0] != '-' && str[0] != '/') {
        return STATUS_ERR_FLAG_FORMAT;
    }
    if (strlen(str) != 2) {
        return STATUS_ERR_FLAG_FORMAT;
    }

    switch (str[1]) {
        case 'r':
        case 'a':
            *out_letter = str[1];
            return STATUS_OK;
        default:
            return STATUS_ERR_FLAG_UNKNOWN;
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        fprintf(stderr, "Ошибка: не указан флаг\n");
        print_usage();
        return 1;
    }

    char letter = '\0';
    Status status = parse_flag(argv[1], &letter);
    if (status != STATUS_OK) {
        print_error("флаг", status);
        print_usage();
        return 1;
    }

    int needed = 4;
    if (letter == 'r') {
        needed = 5;
    }
    if (argc != needed) {
        fprintf(stderr, "Ошибка: для флага %s нужно путей к файлам: %d, передано: %d\n",
                argv[1], needed - 2, argc - 2);
        print_usage();
        return 1;
    }

    const char *out_path = argv[argc - 1];
    status = validate_path(out_path);
    if (status != STATUS_OK) {
        print_error("путь к выходному файлу", status);
        return 1;
    }

    for (int i = 2; i < argc - 1; i++) {
        status = validate_path(argv[i]);
        if (status != STATUS_OK) {
            fprintf(stderr, "Ошибка: путь к входному файлу №%d: %s\n", i - 1, status_message(status));
            return 1;
        }

    }

    if (letter == 'r') {
        status = merge_files(argv[2], argv[3], out_path);
    } else {
        status = transform_file(argv[2], out_path);
    }

    if (status != STATUS_OK) {
        print_error("обработка файлов", status);
        return 1;
    }
    return 0;
}
