#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "polynomial.h"
#include "status.h"

static void print_coefficients(const char *name, const double *coefficients, const int degree)
{
    printf("  %s:", name);
    for (int i = 0; i <= degree; i++) {
        printf(" %.15g", coefficients[i]);
    }
    printf("\n");
}

static void check_point(const double *f, const double *g, const int degree, const double a, const double t)
{
    double f_value = 0.0;
    double g_value = 0.0;
    const double x = a + t;

    Status status = polynomial_value(f, degree, x, &f_value);
    if (status == STATUS_OK) {
        status = polynomial_value(g, degree, t, &g_value);
    }

    if (status == STATUS_OK) {
        printf("  проверка в x = %.15g: f(x) = %.15g, g(x - a) = %.15g\n", x, f_value, g_value);
    } else {
        printf("  проверка в x = %.15g: ошибка: %s\n", x, status_message(status));
    }
}

static void finish(const char *label, const Status status, const double *f, const int degree, const double a,
                   double **g)
{
    printf("%s\n", label);

    if (status != STATUS_OK) {
        printf("  ошибка: %s\n", status_message(status));
    } else {
        print_coefficients("f", f, degree);
        print_coefficients("g", *g, degree);
        check_point(f, *g, degree, a, -1.0);
        check_point(f, *g, degree, a, 2.0);
    }

    free(*g);
    *g = NULL;
}

static void demo_correct(const double epsilon)
{
    double *g = NULL;
    Status status = STATUS_OK;

    printf("--- Корректные данные ---\n");

    const double f1[] = {1.0, 2.0, 3.0};
    status = shift_polynomial(epsilon, 2.0, &g, 2, f1[0], f1[1], f1[2]);
    finish("1 + 2x + 3x^2 по степеням (x - 2):", status, f1, 2, 2.0, &g);

    const double f2[] = {0.0, 0.0, 1.0};
    status = shift_polynomial(epsilon, 1.0, &g, 2, f2[0], f2[1], f2[2]);
    finish("x^2 по степеням (x - 1):", status, f2, 2, 1.0, &g);

    const double f3[] = {0.0, 0.0, 0.0, 1.0};
    status = shift_polynomial(epsilon, -1.0, &g, 3, f3[0], f3[1], f3[2], f3[3]);
    finish("x^3 по степеням (x + 1):", status, f3, 3, -1.0, &g);

    const double f4[] = {1.0, 2.0, 3.0};
    status = shift_polynomial(epsilon, 0.0, &g, 2, f4[0], f4[1], f4[2]);
    finish("1 + 2x + 3x^2 при a = 0 (ничего не меняется):", status, f4, 2, 0.0, &g);

    const double f5[] = {-4.0, 0.5, 0.0, 2.0, -1.0};
    status = shift_polynomial(epsilon, 1.5, &g, 4, f5[0], f5[1], f5[2], f5[3], f5[4]);
    finish("-4 + 0.5x + 2x^3 - x^4 по степеням (x - 1.5):", status, f5, 4, 1.5, &g);
}

static void demo_boundary(const double epsilon)
{
    double *g = NULL;
    Status status = STATUS_OK;

    printf("--- Граничные случаи ---\n");

    const double f1[] = {0.0};
    status = shift_polynomial(epsilon, 7.0, &g, 0, f1[0]);
    finish("Константа 0 (степень 0):", status, f1, 0, 7.0, &g);
    //
    // const double f2[] = {0.01, -0.2, 1.0};
    // status = shift_polynomial(epsilon, 0.1, &g, 2, f2[0], f2[1], f2[2]);
    // finish("(x - 0.1)^2 по степеням (x - 0.1), шум обнулён:", status, f2, 2, 0.1, &g);
    //
    // const double f3[] = {0.0, 1e-8};
    // status = shift_polynomial(epsilon, 0.0, &g, 1, f3[0], f3[1]);
    // finish("Маленький многочлен 1e-8 x (не обнуляется):", status, f3, 1, 0.0, &g);
    //
    // const double f4[] = {0.0, 0.0, 0.0};
    // status = shift_polynomial(epsilon, 3.0, &g, 2, f4[0], f4[1], f4[2]);
    // finish("Нулевой многочлен:", status, f4, 2, 3.0, &g);
    //
    // const double f5[] = {1.0, 1.0, 0.0};
    // status = shift_polynomial(epsilon, 1.0, &g, 2, f5[0], f5[1], f5[2]);
    // finish("Старший коэффициент 0 (1 + x, степень 2):", status, f5, 2, 1.0, &g);
    //
    // const double f6[] = {123456789012345678901234567890.0};
    // status = shift_polynomial(epsilon, 1.0, &g, 0, f6[0]);
    // finish("Коэффициент из 30 цифр (хранится только ~15-17 значащих):", status, f6, 0, 1.0, &g);
    //
    // const double f9[] = {999999999999999999999999999999.0, 2.0};
    // status = shift_polynomial(epsilon, 1.0, &g, 1, f9[0], f9[1]);
    // finish("Коэффициент из 30 девяток (хранится как 1e30) и 2x при a = 1:", status, f9, 1, 1.0, &g);
    //
    // const double f7[] = {1.0, 1e17};
    // status = shift_polynomial(epsilon, 1.0, &g, 1, f7[0], f7[1]);
    // finish("1 + 1e17 x при a = 1 (точно g0 = 100000000000000001, единица теряется):", status, f7, 1, 1.0, &g);
    //
    // const double f8[] = {0.5f, 0.25f};
    // status = shift_polynomial(epsilon, 1.0, &g, 1, 0.5f, 0.25f);
    // finish("Коэффициенты переданы как float:", status, f8, 1, 1.0, &g);
}

static void demo_errors(const double epsilon)
{
    double *g = NULL;
    Status status = STATUS_OK;

    printf("--- Неверные данные ---\n");

    status = shift_polynomial(0.0, 1.0, &g, 1, 1.0, 1.0);
    finish("Эпсилон 0:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(-1e-9, 1.0, &g, 1, 1.0, 1.0);
    finish("Эпсилон -1e-9:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(1.0, 1.0, &g, 1, 1.0, 1.0);
    finish("Эпсилон 1:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial((double) NAN, 1.0, &g, 1, 1.0, 1.0);
    finish("Эпсилон NAN:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, (double) NAN, &g, 1, 1.0, 1.0);
    finish("Точка a = NAN:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, (double) INFINITY, &g, 1, 1.0, 1.0);
    finish("Точка a = INFINITY:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, 1.0, &g, -1);
    finish("Степень -1:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, 1.0, &g, 2, 1.0, (double) NAN, 1.0);
    finish("Коэффициент NAN в середине:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, 1.0, &g, 2, 1.0, 1.0, (double) INFINITY);
    finish("Коэффициент INFINITY:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, 1e200, &g, 2, 0.0, 0.0, 1.0);
    finish("x^2 при a = 1e200 (a^2 не помещается в double):", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, 1e5, &g, 2, 1e300, 0.0, 1e300);
    finish("Огромный коэффициент 1e300 при a = 1e5:", status, NULL, 0, 0.0, &g);

    status = shift_polynomial(epsilon, 1.0, NULL, 1, 1.0, 1.0);
    printf("NULL вместо указателя на результат:\n  ошибка: %s\n", status_message(status));
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    const double epsilon = 1e-9;

    demo_correct(epsilon);
    demo_boundary(epsilon);
    demo_errors(epsilon);

    return 0;
}
