#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <stdio.h>

#include "integrals.h"
#include "parsing.h"
#include "series.h"
#include "status.h"

static void print_usage(void)
{
    printf("Использование: program <eps> <x>\n"
           "  eps - точность вычислений, вещественное число, 0 < eps < 1 (например, 1e-6)\n"
           "  x   - точка для сумм рядов, вещественное число\n"
           "        (ряд c сходится при |x| < 1, ряд d - при |x| <= 1)\n"
           "Дробная часть - через точку.\n");
}

static int parse_real_param(const char *arg, const int position, double *out)
{
    const Status status = parse_double(arg, out);

    if (status != STATUS_OK) {
        printf("Ошибка в параметре %d (\"%s\"): %s\n", position, arg, status_message(status));
        return 0;
    }
    return 1;
}

static int precision_from_eps(const double eps)
{
    const int digits = (int) ceil(-log10(eps));

    if (digits < 0) {
        return 0;
    }
    if (digits > 15) {
        return 15;
    }
    return digits;
}

static void print_real(const double x, const double eps, const int digits)
{
    double shown = x;
    if (fabs(x) < eps) {
        shown = 0.0;
    }

    if (fabs(shown) >= 1e15) {
        printf("%.*e", digits, shown);
    } else {
        printf("%.*f", digits, shown);
    }
}

static void print_result_line(const char *label, const Status status, const double value,
                              const long steps, const char *steps_name,
                              const double eps, const int digits)
{
    if (status != STATUS_OK) {
        printf("  %-40s : %s\n", label, status_message(status));
        return;
    }

    printf("  %-40s = ", label);
    print_real(value, eps, digits);
    printf("   (%s: %ld)\n", steps_name, steps);
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    const long max_terms = 10000000L;
    const long max_segments = 134217728L;

    if (argc != 3) {
        printf("Ошибка: неверное количество аргументов: нужно 2, передано %d\n", argc - 1);
        print_usage();
        return 1;
    }

    double eps = 0.0;
    double x = 0.0;

    if (!parse_real_param(argv[1], 1, &eps)) {
        print_usage();
        return 1;
    }
    if (!parse_real_param(argv[2], 2, &x)) {
        print_usage();
        return 1;
    }
    if (eps <= 0.0) {
        printf("Ошибка в параметре 1 (\"%s\"): %s\n", argv[1],
               status_message(STATUS_ERR_EPS_NOT_POSITIVE));
        return 1;
    }
    if (eps >= 1.0) {
        printf("Ошибка в параметре 1 (\"%s\"): %s\n", argv[1],
               status_message(STATUS_ERR_EPS_TOO_LARGE));
        return 1;
    }

    const int digits = precision_from_eps(eps);
    double value = 0.0;
    long steps = 0;
    Status status;

    printf("Точность eps = %s, x = %s (знаков после запятой: %d)\n\n", argv[1], argv[2], digits);

    printf("Суммы рядов:\n");

    status = series_a(x, eps, max_terms, &value, &steps);
    print_result_line("a) x^n / n!", status, value, steps, "членов ряда", eps, digits);

    status = series_b(x, eps, max_terms, &value, &steps);
    print_result_line("b) (-1)^n x^(2n) / (2n)!", status, value, steps, "членов ряда", eps, digits);

    status = series_c(x, eps, max_terms, &value, &steps);
    print_result_line("c) 27^n (n!)^3 x^(2n) / (3n)!", status, value, steps, "членов ряда", eps, digits);

    status = series_d(x, eps, max_terms, &value, &steps);
    print_result_line("d) (-1)^n (2n-1)!! x^(2n) / (2n)!!", status, value, steps, "членов ряда", eps, digits);

    printf("\nИнтегралы на [0; 1]:\n");

    status = integral_a(eps, max_segments, &value, &steps);
    print_result_line("a) ln(1 + x) / x", status, value, steps, "отрезков", eps, digits);

    status = integral_b(eps, max_segments, &value, &steps);
    print_result_line("b) e^(-x^2 / 2)", status, value, steps, "отрезков", eps, digits);

    status = integral_c(eps, max_segments, &value, &steps);
    print_result_line("c) ln(1 / (1 - x))", status, value, steps, "отрезков", eps, digits);

    status = integral_d(eps, max_segments, &value, &steps);
    print_result_line("d) x^x", status, value, steps, "отрезков", eps, digits);

    return 0;
}
