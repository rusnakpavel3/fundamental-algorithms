#ifdef _WIN32
#include <windows.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "big_sum.h"
#include "status.h"

static void finish(const char *label, const Status status, char **result)
{
    if (status == STATUS_OK) {
        printf("%s = %s\n", label, *result);
    } else {
        printf("%s -> ошибка: %s\n", label, status_message(status));
    }

    free(*result);
    *result = NULL;
}

static void demo_correct(void)
{
    char *result = NULL;
    Status status = STATUS_OK;

    printf("--- Корректные данные ---\n");

    status = sum_in_base(&result, 10, 2, "957", "68");
    finish("10: 957 + 68", status, &result);

    status = sum_in_base(&result, 10, 2, "999", "1");
    finish("10: 999 + 1", status, &result);

    status = sum_in_base(&result, 2, 2, "1111", "1");
    finish("2: 1111 + 1", status, &result);

    status = sum_in_base(&result, 16, 2, "FF", "1");
    finish("16: FF + 1", status, &result);

    status = sum_in_base(&result, 16, 2, "ff", "1");
    finish("16: ff + 1", status, &result);

    status = sum_in_base(&result, 16, 3, "FFFF", "FFFF", "2");
    finish("16: FFFF + FFFF + 2", status, &result);

    status = sum_in_base(&result, 36, 2, "Z", "Z");
    finish("36: Z + Z", status, &result);

    status = sum_in_base(&result, 36, 2, "ZZ", "1");
    finish("36: ZZ + 1", status, &result);

    status = sum_in_base(&result, 8, 3, "777", "1", "1");
    finish("8: 777 + 1 + 1", status, &result);

    status = sum_in_base(&result, 7, 3, "6");
    finish("7: 666 + 1", status, &result);

    status = sum_in_base(&result, 10, 5, "1", "2", "3", "4", "5");
    finish("10: 1 + 2 + 3 + 4 + 5", status, &result);

    status = sum_in_base(&result, 10, 2, "99999999999999999999", "1");
    finish("10: 20 девяток + 1", status, &result);
}

static void demo_boundary(void)
{
    char *result = NULL;
    Status status = STATUS_OK;

    printf("--- Граничные случаи ---\n");

    status = sum_in_base(&result, 10, 2, "000", "0");
    finish("10: 000 + 0", status, &result);

    status = sum_in_base(&result, 10, 1, "0012");
    finish("10: одно число 0012", status, &result);

    status = sum_in_base(&result, 10, 2, "0007", "0003");
    finish("10: 0007 + 0003", status, &result);

    status = sum_in_base(&result, 16, 0);
    finish("16: ни одного числа", status, &result);

    status = sum_in_base(&result, 2, 2, "1", "1");
    finish("2: 1 + 1", status, &result);

    status = sum_in_base(&result, 36, 2, "z", "1");
    finish("36: z + 1", status, &result);
}

static void demo_errors(void)
{
    char *result = NULL;
    Status status = STATUS_OK;

    printf("--- Неверные данные ---\n");

    status = sum_in_base(&result, 1, 2, "1", "1");
    finish("основание 1", status, &result);

    status = sum_in_base(&result, 0, 2, "1", "1");
    finish("основание 0", status, &result);

    status = sum_in_base(&result, 37, 2, "1", "1");
    finish("основание 37", status, &result);

    status = sum_in_base(&result, -5, 2, "1", "1");
    finish("основание -5", status, &result);

    status = sum_in_base(&result, 10, -1);
    finish("количество -1", status, &result);

    status = sum_in_base(&result, 10, 2, "1", (const char *) NULL);
    finish("10: второе число NULL", status, &result);

    status = sum_in_base(&result, 10, 2, "", "1");
    finish("10: пустая строка", status, &result);

    status = sum_in_base(&result, 2, 2, "12", "1");
    finish("2: 12 + 1", status, &result);

    status = sum_in_base(&result, 10, 2, "-5", "1");
    finish("10: -5 + 1", status, &result);

    status = sum_in_base(&result, 10, 2, "1 2", "1");
    finish("10: 1 2 + 1", status, &result);

    status = sum_in_base(&result, 16, 2, "1G", "1");
    finish("16: 1G + 1", status, &result);

    status = sum_in_base(&result, 10, 3, "1", "2", "x");
    finish("10: ошибка в третьем числе", status, &result);

    status = sum_in_base(NULL, 10, 1, "1");
    printf("%s -> ошибка: %s\n", "NULL вместо результата", status_message(status));
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
