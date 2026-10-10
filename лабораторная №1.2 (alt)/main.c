#include <math.h>
#include <stdio.h>
#include "status.h"
#include "parsing.h"
#include "constants.h"

#ifdef _WIN32
#include <windows.h>
#endif

typedef Status (*Method)(const double eps, double *out_value, long *out_steps);

static int precision_from_eps(const double eps)
{
    const int digits = (int) ceil(-log10(eps));

    if (digits < 0) {
        return 0;
    }
    if (digits > 14) {
        return 14;
    }
    return digits;
}

static void run_method(const char *label, const Method method, const double eps)
{
    double value = 0.0;
    long steps = 0;
    Status status = method(eps, &value, &steps);

    if (status == STATUS_OK) {
        printf("  %s: %.*f (шагов: %ld)\n", label, precision_from_eps(eps), value, steps);
    } else if (status == STATUS_ERR_NOT_CONVERGED) {
        printf("  %s: не сошлось, такая точность недостижима для этого метода\n", label);
    } else if (status == STATUS_ERR_INVALID_INTERVAL) {
        printf("  %s: ошибка, на интервале нет смены знака\n", label);
    } else if (status == STATUS_ERR_ALLOC_FAILED) {
        printf("  %s: ошибка выделения памяти\n", label);
    } else if (status == STATUS_ERR_EPS_INVALID) {
        printf("  %s: ошибка, недопустимое значение eps\n", label);
    } else if (status == STATUS_ERR_NULL_ARGUMENT) {
        printf("  %s: внутренняя ошибка, пустой указатель\n", label);
    } else {
        printf("  %s: неизвестная ошибка\n", label);
    }
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 2) {
        printf("Ошибка: неверное количество аргументов. Использование: program <eps>\n");
        return 1;
    }

    double eps = 0.0;
    Status status = parse_double(argv[1], &eps);

    if (status == STATUS_ERR_NUMBER_INVALID) {
        printf("Ошибка: eps не является корректным вещественным числом.\n");
        return 1;
    } else if (status == STATUS_ERR_NUMBER_OVERFLOW) {
        printf("Ошибка: eps выходит за диапазон double.\n");
        return 1;
    } else if (status == STATUS_ERR_NUMBER_TOO_PRECISE) {
        printf("Ошибка: в eps больше 15 значащих цифр, тип double хранит только 15.\n");
        return 1;
    } else if (status != STATUS_OK) {
        printf("Ошибка при разборе eps.\n");
        return 1;
    }

    status = validate_eps(eps);
    if (status != STATUS_OK) {
        printf("Ошибка: eps должен быть конечным числом не меньше %g и меньше 1.\n", MIN_EPS);
        return 1;
    }

    printf("eps = %.15g (знаков после запятой: %d)\n", eps, precision_from_eps(eps));

    printf("e:\n");
    run_method("предел", e_limit, eps);
    run_method("ряд", e_series, eps);
    run_method("уравнение", e_equation, eps);

    printf("pi:\n");
    run_method("предел", pi_limit, eps);
    run_method("ряд", pi_series, eps);
    run_method("уравнение", pi_equation, eps);

    printf("ln2:\n");
    run_method("предел", ln2_limit, eps);
    run_method("ряд", ln2_series, eps);
    run_method("уравнение", ln2_equation, eps);

    printf("sqrt2:\n");
    run_method("предел", sqrt2_limit, eps);
    run_method("произведение", sqrt2_product, eps);
    run_method("уравнение", sqrt2_equation, eps);

    printf("gamma:\n");
    run_method("предел", gamma_limit, eps);
    run_method("ряд", gamma_series, eps);
    run_method("уравнение", gamma_equation, eps);

    return 0;
}
