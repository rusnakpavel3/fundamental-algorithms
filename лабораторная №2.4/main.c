#ifdef _WIN32
#include <windows.h>
#endif

#include <stddef.h>
#include <stdio.h>

#include "backup.h"
#include "input.h"
#include "status.h"

static void print_input_error(const Status status, const size_t line)
{
    if (status == STATUS_ERR_ALLOC_FAILED || status == STATUS_ERR_SIZE_OVERFLOW
        || status == STATUS_ERR_READ_FAILED) {
        printf("Ошибка: %s\n", status_message(status));
    } else if (line > 0 && status != STATUS_ERR_TOO_FEW_LINES && status != STATUS_ERR_INPUT_EMPTY) {
        printf("Ошибка во входных данных (строка %zu): %s\n", line, status_message(status));
    } else {
        printf("Ошибка во входных данных: %s\n", status_message(status));
    }
}

static void print_result(const BackupResult *result)
{
    printf("TOTAL_SIZE %lld\n", result->total_size);
    printf("TOTAL_IMPORTANCE %lld\n", result->total_importance);
    printf("FILES %zu\n", result->count);

    for (size_t i = 0; i < result->count; i++) {
        printf("%s\n", result->files[i]->name);
    }
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    FileInfo *files = NULL;
    size_t count = 0;
    long long capacity = 0;
    size_t error_line = 0;

    Status status = read_backup_input(stdin, &files, &count, &capacity, &error_line);
    if (status != STATUS_OK) {
        print_input_error(status, error_line);
        return 1;
    }

    BackupResult result;
    status = select_backup(files, count, capacity, &result);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        free_files(files, count);
        return 1;
    }

    print_result(&result);

    free_backup_result(&result);
    free_files(files, count);
    return 0;
}
