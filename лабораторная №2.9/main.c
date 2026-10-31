#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "fraction.h"
#include "status.h"


static const char *kind_text(const FractionKind kind)
{
    switch (kind) {
        case FRACTION_FINITE:
            return "конечная запись";
        case FRACTION_INFINITE:
            return "бесконечная запись";
    }
    return "неизвестный результат";
}

static void finish(const char *label, const Status status, const double *values, const int count,
                   FractionKind **results)
{
    printf("%s\n", label);

    if (status != STATUS_OK) {
        printf("  ошибка: %s\n", status_message(status));
    } else if (count == 0) {
        printf("  дробей нет\n");
    } else {
        for (int i = 0; i < count; i++) {
            printf("  %.15g -> %s\n", values[i], kind_text((*results)[i]));
        }
    }

    free(*results);
    *results = NULL;
}

static void demo_correct(const double epsilon)
{
    FractionKind *results = NULL;
    Status status = STATUS_OK;

    printf("--- Корректные данные ---\n");

    const double set_10[] = {0.5, 0.123456789, 1.0 / 3, 5.0 / 7};
    status = classify_fractions(&results, epsilon, 10, 4, set_10[0], set_10[1], set_10[2], set_10[3]);
    finish("Основание 10:", status, set_10, 4, &results);

    const double set_2[] = {0.375, 0.1};
    status = classify_fractions(&results, epsilon, 2, 2, set_2[0], set_2[1]);
    finish("Основание 2:", status, set_2, 2, &results);

    const double set_3[] = {1.0 / 3, 0.5};
    status = classify_fractions(&results, epsilon, 3, 2, set_3[0], set_3[1]);
    finish("Основание 3:", status, set_3, 2, &results);

    const double set_7[] = {5.0 / 7, 0.5};
    status = classify_fractions(&results, epsilon, 7, 2, set_7[0], set_7[1]);
    finish("Основание 7:", status, set_7, 2, &results);

    const double set_6[] = {0.375, 1.0 / 3, 0.2};
    status = classify_fractions(&results, epsilon, 6, 3, set_6[0], set_6[1], set_6[2]);
    finish("Основание 6:", status, set_6, 3, &results);
}

static void demo_boundary(const double epsilon)
{
    FractionKind *results = NULL;
    Status status = STATUS_OK;

    printf("--- Граничные случаи ---\n");

    const double set_edges[] = {0.000000001, 0.999999999};
    status = classify_fractions(&results, epsilon, 10, 2, set_edges[0], set_edges[1]);
    finish("Основание 10, самая маленькая и самая большая 9-значные дроби:", status, set_edges, 2, &results);

    const double set_long[] = {1.0 / 1048576};
    status = classify_fractions(&results, epsilon, 2, 1, set_long[0]);
    finish("Основание 2, дробь 1 / 2^20 (20 знаков после точки):", status, set_long, 1, &results);

    status = classify_fractions(&results, epsilon, 10, 0);
    finish("Основание 10, ни одной дроби:", status, NULL, 0, &results);
}

static void demo_errors(const double epsilon)
{
    FractionKind *results = NULL;
    Status status = STATUS_OK;

    printf("--- Неверные данные ---\n");

    status = classify_fractions(&results, epsilon, 10, 1, 1.5);
    finish("Дробь 1.5:", status, NULL, 0, &results);

    status = classify_fractions(&results, epsilon, 10, 1, (double) NAN);
    finish("Дробь NAN:", status, NULL, 0, &results);

    status = classify_fractions(&results, epsilon, 1, 1, 0.5);
    finish("Основание 1:", status, NULL, 0, &results);

    status = classify_fractions(&results, 1e-6, 10, 1, 0.5);
    finish("Эпсилон 1e-6 (слишком грубый):", status, NULL, 0, &results);

	const double set_test[] = {1.0 / 3};
	status = classify_fractions(&results, epsilon, 10, 1, set_test[0]);
	finish("======   Основание 7, дробь 5.0 / 7:", status, set_test, 1, &results);   // !!!!!!!!!!!!!!
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    const double epsilon = 1e-15;

    demo_correct(epsilon);
    demo_boundary(epsilon);
    demo_errors(epsilon);

    return 0;
}
