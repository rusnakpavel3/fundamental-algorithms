#ifdef _WIN32
#include <windows.h>
#endif

#include <stddef.h>
#include <stdio.h>

#include "search.h"
#include "status.h"

static void print_results(const Status status, const FileResult *results, const int file_count)
{
    if (status != STATUS_OK) {
        printf("  ошибка: %s\n", status_message(status));
        return;
    }
    if (results == NULL) {
        printf("  файлов нет\n");
        return;
    }

    for (int i = 0; i < file_count; i++) {
        const FileResult *result = &results[i];
        printf("  файл \"%s\":\n", result->path != NULL ? result->path : "(NULL)");

        if (result->status != STATUS_OK) {
            printf("    ошибка: %s\n", status_message(result->status));
        } else if (result->count == 0) {
            printf("    вхождений нет\n");
        } else {
            for (size_t j = 0; j < result->count; j++) {
                printf("    строка %zu, символ %zu\n", result->matches[j].line, result->matches[j].column);
            }
        }
    }
}

static void finish(const char *label, const Status status, FileResult **results, const int file_count)
{
    printf("%s\n", label);
    print_results(status, *results, file_count);
    free_file_results(*results, file_count);
    *results = NULL;
}

static void demo_correct(void)
{
    FileResult *results = NULL;
    Status status = STATUS_OK;

    printf("--- Корректные данные ---\n");

    status = find_substring(&results, "hello", 1, "test1.txt");
    finish("Подстрока \"hello\":", status, &results, 1);

    status = find_substring(&results, " ", 1, "test1.txt");
    finish("Подстрока из одного пробела:", status, &results, 1);

    status = find_substring(&results, "o", 1, "test1.txt");
    finish("Подстрока \"o\":", status, &results, 1);

    status = find_substring(&results, "d\nhe", 1, "test1.txt");
    finish("Подстрока \"d\\nhe\" (через перевод строки):", status, &results, 1);

    status = find_substring(&results, "\n", 1, "test1.txt");
    finish("Подстрока из одного перевода строки:", status, &results, 1);

    status = find_substring(&results, "a", 3, "test1.txt", "test2.txt", "test3.txt");
    finish("Подстрока \"a\" сразу в трёх файлах:", status, &results, 3);
}

static void demo_boundary(void)
{
    FileResult *results = NULL;
    Status status = STATUS_OK;

    printf("--- Граничные случаи ---\n");

    status = find_substring(&results, "aa", 1, "test2.txt");
    finish("Пересекающиеся вхождения \"aa\" в \"aaaa\":", status, &results, 1);

    status = find_substring(&results, "\t", 1, "test3.txt");
    finish("Подстрока из табуляции:", status, &results, 1);

    status = find_substring(&results, "xyz", 1, "test1.txt");
    finish("Подстроки нет в файле:", status, &results, 1);

    status = find_substring(&results, "hello world\nhello, hello\n!", 1, "test1.txt");
    finish("Подстрока длиннее файла:", status, &results, 1);

    status = find_substring(&results, "hello world\nhello, hello\n", 1, "test1.txt");
    finish("Подстрока равна всему файлу:", status, &results, 1);

    status = find_substring(&results, "hello", 1, "empty.txt");
    finish("Пустой файл:", status, &results, 1);

    status = find_substring(&results, "мир", 1, "test4.txt");
    finish("Русское слово (номер символа считается в байтах UTF-8):", status, &results, 1);

    status = find_substring(&results, "hello", 2, "test1.txt", "test1.txt");
    finish("Один и тот же файл дважды:", status, &results, 2);

    status = find_substring(&results, "hello", 0);
    finish("Ни одного файла:", status, &results, 0);
}

static void demo_errors(void)
{
    FileResult *results = NULL;
    Status status = STATUS_OK;

    printf("--- Неверные данные ---\n");

    status = find_substring(&results, "hello", 3, "test1.txt", "missing.txt", "test2.txt");
    finish("Несуществующий файл среди других (остальные обработаны):", status, &results, 3);

    status = find_substring(&results, "hello", 1, (const char *) NULL);
    finish("Вместо пути NULL:", status, &results, 1);

    status = find_substring(&results, "hello", 1, "");
    finish("Пустой путь:", status, &results, 1);

    status = find_substring(&results, "hello", 1, ".");
    finish("Путь к папке, а не к файлу:", status, &results, 1);

    status = find_substring(&results, "", 1, "test1.txt");
    finish("Пустая подстрока:", status, &results, 1);

    status = find_substring(&results, NULL, 1, "test1.txt");
    finish("Подстрока NULL:", status, &results, 1);

    status = find_substring(&results, "hello", -1);
    finish("Количество файлов -1:", status, &results, -1);

    status = find_substring(NULL, "hello", 1, "test1.txt");
    printf("NULL вместо указателя на результат:\n  ошибка: %s\n", status_message(status));
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    demo_correct();
    demo_boundary();
    demo_errors();

    return 0;
}
