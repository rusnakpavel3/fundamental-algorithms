#ifdef _WIN32
#include <windows.h>
#endif

#include <stdbool.h>
#include <stdio.h>

#include "file_io.h"
#include "paths.h"
#include "status.h"

static void print_usage(void)
{
    fprintf(stderr,
            "Использование:\n"
            "  program <входной файл> <выходной файл>\n"
            "Во входном файле - числа в системах счисления [2..36], разделённые\n"
            "пробелами, табуляциями и переводами строк. Для каждого числа в выходной\n"
            "файл пишется строка: число без ведущих нулей, минимальное основание,\n"
            "значение в десятичной системе.\n");
}

static void print_error(const char *context, const Status status)
{
    fprintf(stderr, "Ошибка: %s: %s\n", context, status_message(status));
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 3) {
        fprintf(stderr, "Ошибка: нужно путей к файлам: 2, передано: %d\n", argc - 1);
        print_usage();
        return 1;
    }

    Status status = validate_path(argv[1]);
    if (status != STATUS_OK) {
        print_error("путь к входному файлу", status);
        return 1;
    }
    status = validate_path(argv[2]);
    if (status != STATUS_OK) {
        print_error("путь к выходному файлу", status);
        return 1;
    }

    unsigned long long bad_number = 0;
    status = convert_file(argv[1], argv[2], &bad_number);

    if (status == STATUS_ERR_NUMBER_INVALID || status == STATUS_ERR_NUMBER_TOO_BIG) {
        fprintf(stderr, "Ошибка в числе №%llu: %s\n", bad_number, status_message(status));
        return 1;
    } else if (status != STATUS_OK) {
        print_error("обработка файлов", status);
        return 1;
    }
    return 0;
}
