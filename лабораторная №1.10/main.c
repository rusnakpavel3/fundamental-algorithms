#ifdef _WIN32
#include <windows.h>
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "numbers.h"
#include "status.h"

#define TOKEN_MAX 128

static Status read_token(char *buffer, const size_t size, bool *out_found, bool *out_too_long)
{
    if (buffer == NULL || out_found == NULL || out_too_long == NULL || size < 2) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_found = false;
    *out_too_long = false;
    buffer[0] = '\0';

    int c = getchar();
    while (c != EOF && isspace(c)) {
        c = getchar();
    }
    if (c == EOF) {
        if (ferror(stdin)) {
            return STATUS_ERR_READ_FAILED;
        }
        return STATUS_OK;
    }

    *out_found = true;
    size_t length = 0;

    while (c != EOF && !isspace(c)) {
        const bool only_zero = (length == 1 && buffer[0] == '0')
                            || (length == 2 && buffer[0] == '-' && buffer[1] == '0');
        if (c == '0' && only_zero) {
            c = getchar();
            continue;
        }
        if (length + 1 < size) {
            buffer[length] = (char) c;
            length++;
        } else {
            *out_too_long = true;
        }
        c = getchar();
    }
    buffer[length] = '\0';

    if (ferror(stdin)) {
        return STATUS_ERR_READ_FAILED;
    }
    return STATUS_OK;
}

static void print_number(const char *label, const long long value, const int base)
{
    const int bases[4] = {9, 18, 27, 36};
    char text[BASE_TEXT_MAX];

    Status status = to_base(value, base, text, sizeof text);
    if (status != STATUS_OK) {
        printf("%s: ошибка - %s\n", label, status_message(status));
        return;
    }
    printf("%s (основание %d): %s\n", label, base, text);

    for (int i = 0; i < 4; i++) {
        status = to_base(value, bases[i], text, sizeof text);
        if (status != STATUS_OK) {
            printf("  ошибка - %s\n", status_message(status));
            return;
        }
        printf("  в системе с основанием %2d: %s\n", bases[i], text);
    }
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    char token[TOKEN_MAX];
    bool found = false;
    bool too_long = false;

    printf("Введите основание системы счисления (от 2 до 36):\n");

    Status status = read_token(token, sizeof token, &found, &too_long);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 1;
    }
    if (!found) {
        printf("Ошибка: основание не введено\n");
        return 1;
    }

    int base = 0;
    status = parse_base(token, &base);
    if (status != STATUS_OK || too_long) {
        printf("Ошибка: \"%s\": %s\n", token, status_message(STATUS_ERR_BASE_INVALID));
        return 1;
    }

    printf("Вводите целые числа в системе с основанием %d (цифры больше 9 - прописными буквами).\n", base);
    printf("Для завершения введите Stop.\n");

    Stats stats;
    stats_init(&stats);
    bool stopped = false;

    while (true) {
        status = read_token(token, sizeof token, &found, &too_long);
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return 1;
        }
        if (!found) {
            break;
        }
        if (too_long) {
            printf("\"%s...\" пропущено: слишком длинная запись (больше %d символов)\n", token, TOKEN_MAX - 1);
            continue;
        }
        if (strcmp(token, "Stop") == 0) {
            stopped = true;
            break;
        }

        long long value = 0;
        status = parse_in_base(token, base, &value);
        if (status == STATUS_OK) {
            status = stats_add(&stats, value);
        }
        if (status != STATUS_OK) {
            printf("\"%s\" пропущено: %s\n", token, status_message(status));
        }
    }

    if (!stopped) {
        printf("Предупреждение: ввод закончился без Stop.\n");
    }
    if (stats.count == 0) {
        printf("Не введено ни одного корректного числа.\n");
        return 0;
    }

    printf("\nВведено чисел: %zu\n", stats.count);
    print_number("Максимальное по модулю", stats.max_abs, base);

    long long sum = 0;
    status = stats_sum(&stats, &sum);
    if (status != STATUS_OK) {
        printf("Сумма: %s\n", status_message(status));
        return 0;
    }
    print_number("Сумма", sum, base);
    return 0;
}
