#ifdef _WIN32
#include <windows.h>
#endif

#include <stddef.h>
#include <stdio.h>

#include "file_io.h"
#include "status.h"

static void print_usage(void)
{
    printf("Использование: program <входной файл> <флаг> <выходной файл>\n"
           "Флаг: -a или /a - по возрастанию, -d или /d - по убыванию.\n"
           "Каждая строка файла: id имя фамилия зарплата\n");
}

static Status parse_flag(const char *str, SortOrder *out_order)
{
    if (str[0] != '-' && str[0] != '/') {
        return STATUS_ERR_FLAG_FORMAT;
    }
    if (str[1] == '\0' || str[2] != '\0') {
        return STATUS_ERR_FLAG_FORMAT;
    }

    switch (str[1]) {
        case 'a':
            *out_order = SORT_ASCENDING;
            return STATUS_OK;
        case 'd':
            *out_order = SORT_DESCENDING;
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

    if (argc != 4) {
        printf("Ошибка: %s (передано %d, нужно 3)\n",
               status_message(STATUS_ERR_ARGUMENT_COUNT), argc - 1);
        print_usage();
        return 1;
    }

    SortOrder order = SORT_ASCENDING;
    Status status = parse_flag(argv[2], &order);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        print_usage();
        return 1;
    }

    size_t count = 0;
    size_t bad_line = 0;
    status = sort_employee_file(argv[1], argv[3], order, &count, &bad_line);

    if (status != STATUS_OK) {
        if (bad_line > 0) {
            printf("Ошибка в строке %zu: %s\n", bad_line, status_message(status));
        } else {
            printf("Ошибка: %s\n", status_message(status));
        }
        return 1;
    }

    printf("Готово: отсортировано записей: %zu\n", count);
    return 0;
}
