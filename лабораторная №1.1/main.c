#include <stdio.h>
#include "status.h"
#include "parsing.h"
#include "task1_logic.h"

#ifdef _WIN32
#include <windows.h>
#endif

static void print_status_message(Status status)
{
    if (status == STATUS_OK) {
        return;
    } else if (status == STATUS_ERR_NULL_ARGUMENT) {
        printf("Внутренняя ошибка: получен пустой указатель.\n");
    } else if (status == STATUS_ERR_ARGC_INVALID) {
        printf("Ошибка: неверное количество аргументов. Использование: program <x> <flag>\n");
    } else if (status == STATUS_ERR_FLAG_FORMAT) {
        printf("Ошибка: флаг должен начинаться с '-' или '/' и содержать один символ действия.\n");
    } else if (status == STATUS_ERR_FLAG_UNKNOWN) {
        printf("Ошибка: неизвестный флаг. Допустимые: -h -p -s -e -a -f\n");
    } else if (status == STATUS_ERR_NUMBER_INVALID) {
        printf("Ошибка: x должно быть целым числом без посторонних символов.\n");
    } else if (status == STATUS_ERR_NUMBER_OVERFLOW) {
        printf("Ошибка: x выходит за пределы допустимого диапазона.\n");
    } else if (status == STATUS_ERR_NUMBER_NEGATIVE) {
        printf("Ошибка: для данного флага x не может быть отрицательным.\n");
    } else if (status == STATUS_ERR_VALUE_OUT_OF_RANGE) {
        printf("Ошибка: x вне допустимого диапазона для данного флага (для -e: от 1 до 10).\n");
    } else if (status == STATUS_ERR_COMPUTE_OVERFLOW) {
        printf("Ошибка: переполнение при вычислении результата.\n");
    } else {
        printf("Ошибка: неизвестный статус выполнения.\n");
    }
}

static void print_multiples_result(const long long *arr, int count, long long x)
{
    if (count == 0) {
        printf("Натуральных чисел от 1 до 100, кратных %lld, не найдено.\n", x);
        return;
    }

    printf("Числа от 1 до 100, кратные %lld:", x);
    for (int i = 0; i < count; i++) {
        printf(" %lld", arr[i]);
    }
    printf("\n");
}

static void print_prime_result(PrimeClass class_, long long x)
{
    if (class_ == NUMBER_PRIME) {
        printf("%lld является простым числом.\n", x);
    } else if (class_ == NUMBER_COMPOSITE) {
        printf("%lld является составным числом.\n", x);
    } else {
        printf("%lld не является ни простым, ни составным числом.\n", x);
    }
}

static void print_hex_result(const int *digits, int count)
{
    static const char hex_chars[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    for (int i = 0; i < count; i++) {
        if (i > 0) {
            printf(" ");
        }
        printf("%c", hex_chars[digits[i]]);
    }
    printf("\n");
}

static void print_power_table_result(long long table[POWER_TABLE_BASES][POWER_TABLE_MAX_EXP],
                                     int used_cols)
{
    for (int base_idx = 0; base_idx < POWER_TABLE_BASES; base_idx++) {
        printf("%2d:", base_idx + 1);
        for (int exp_idx = 0; exp_idx < used_cols; exp_idx++) {
            printf(" %lld", table[base_idx][exp_idx]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32

    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 3) {
        print_status_message(STATUS_ERR_ARGC_INVALID);
        return 1;
    }

    long long x;
    Status status = parse_long_long(argv[1], &x);
    if (status != STATUS_OK) {
        print_status_message(status);
        return 1;
    }

    FlagType flag;
    status = parse_flag(argv[2], &flag);
    if (status != STATUS_OK) {
        print_status_message(status);
        return 1;
    }


    switch (flag) {
        case FLAG_H: {
            long long multiples[MAX_MULTIPLES];
            int count = 0;
            status = find_multiples_of_x(x, multiples, &count);
            if (status == STATUS_OK) {
                print_multiples_result(multiples, count, x);
            }
            break;
        }
        case FLAG_P: {
            PrimeClass class_;
            status = classify_number(x, &class_);
            if (status == STATUS_OK) {
                print_prime_result(class_, x);
            }
            break;
        }
        case FLAG_S: {
            int digits[MAX_HEX_DIGITS];
            int count = 0;
            status = to_hex_digits(x, digits, &count);
            if (status == STATUS_OK) {
                print_hex_result(digits, count);
            }
            break;
        }
        case FLAG_E: {
            long long table[POWER_TABLE_BASES][POWER_TABLE_MAX_EXP];
            int used_cols = 0;
            status = compute_power_table(x, table, &used_cols);
            if (status == STATUS_OK) {
                print_power_table_result(table, used_cols);
            }
            break;
        }
        case FLAG_A: {
            long long sum = 0;
            status = sum_1_to_x(x, &sum);
            if (status == STATUS_OK) {
                printf("Сумма натуральных чисел от 1 до %lld равна %lld.\n", x, sum);
            }
            break;
        }
        case FLAG_F: {
            long long result = 0;
            status = factorial_of_x(x, &result);
            if (status == STATUS_OK) {
                printf("%lld! = %lld\n", x, result);
            }
            break;
        }
    }

    if (status != STATUS_OK) {
        print_status_message(status);
        return 1;
    }

    return 0;
}
