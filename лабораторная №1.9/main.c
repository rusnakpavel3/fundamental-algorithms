#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "arrays.h"
#include "parsing.h"
#include "status.h"

#define FIXED_SIZE 20
#define SHOW_ROWS 10

static void print_array(const char *label, const int *array, const size_t size)
{
    printf("%s:", label);
    for (size_t i = 0; i < size; i++) {
        printf(" %d", array[i]);
    }
    printf("\n");
}

static int run_part1(const int a, const int b)
{
    int array[FIXED_SIZE];

    Status status = fill_random(array, FIXED_SIZE, a, b);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 0;
    }

    printf("=== массив из %d элементов в диапазоне [%d..%d] ===\n", FIXED_SIZE, a, b);
    print_array("До     ", array, FIXED_SIZE);

    size_t min_index = 0;
    size_t max_index = 0;
    status = swap_min_max(array, FIXED_SIZE, &min_index, &max_index);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 0;
    }

    print_array("После  ", array, FIXED_SIZE);
    printf("Минимум был на позиции %zu, максимум - на позиции %zu; они поменялись местами.\n\n",
           min_index, max_index);
    return 1;
}

static int run_part2(void)
{
    int a_size_int = 0;
    int b_size_int = 0;

    Status status = random_int(10, 10000, &a_size_int);
    if (status == STATUS_OK) {
        status = random_int(10, 10000, &b_size_int);
    }
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 0;
    }

    const size_t a_size = (size_t) a_size_int;
    const size_t b_size = (size_t) b_size_int;

    int *a = malloc(a_size * sizeof(int));
    int *b = malloc(b_size * sizeof(int));
    if (a == NULL || b == NULL) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ALLOC_FAILED));
        free(a);
        free(b);
        return 0;
    }

    status = fill_random(a, a_size, -1000, 1000);
    if (status == STATUS_OK) {
        status = fill_random(b, b_size, -1000, 1000);
    }

    int *c = NULL;
    if (status == STATUS_OK) {
        status = build_sum_with_nearest(a, a_size, b, b_size, &c);
    }

    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        free(a);
        free(b);
        return 0;
    }

    printf("=== размер A = %zu, размер B = %zu, размер C = %zu ===\n", a_size, b_size, a_size);
    printf("Первые %d элементов (ближайший из B = C[i] - A[i]):\n", SHOW_ROWS);
    printf("%6s %8s %14s %8s\n", "i", "A[i]", "ближайший в B", "C[i]");
    for (size_t i = 0; i < SHOW_ROWS; i++) {
        printf("%6zu %8d %14d %8d\n", i, a[i], c[i] - a[i], c[i]);
    }

    free(a);
    free(b);
    free(c);
    return 1;
}

int main(const int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 3) {
        printf("Ошибка: нужно 2 параметра, передано %d. Использование: program <a> <b>\n", argc - 1);
        return 1;
    }

    int a = 0;
    int b = 0;

    Status status = parse_int(argv[1], &a);
    if (status != STATUS_OK) {
        printf("Ошибка в параметре a (\"%s\"): %s\n", argv[1], status_message(status));
        return 1;
    }
    status = parse_int(argv[2], &b);
    if (status != STATUS_OK) {
        printf("Ошибка в параметре b (\"%s\"): %s\n", argv[2], status_message(status));
        return 1;
    }
    if (a > b) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_RANGE_INVALID));
        return 1;
    }

    srand((unsigned int) time(NULL));

    if (!run_part1(a, b)) {
        return 1;
    }
    if (!run_part2()) {
        return 1;
    }
    return 0;
}
