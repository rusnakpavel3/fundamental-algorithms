#ifdef _WIN32
#include <windows.h>
#endif

#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "bisection.h"
#include "power.h"
#include "status.h"
#include "variadic.h"

#define KAPREKAR_CAPACITY 32

static void print_error(const Status status)
{
    printf("ошибка - %s\n", status_message(status));
}

static int decimals_from_eps(const double eps)
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

static void show_convex(const char *label, const Status status, const bool convex)
{
    printf("  %s: ", label);
    if (status == STATUS_OK) {
        printf("%s\n", convex ? "выпуклый" : "невыпуклый");
    } else {
        print_error(status);
    }
}

static void show_number(const char *label, const Status status, const double value)
{
    printf("  %s: ", label);
    if (status == STATUS_OK) {
        printf("%.15g\n", value);
    } else {
        print_error(status);
    }
}

static void show_kaprekar(const char *label, const Status status,
                          const char *const found[], const size_t found_count,
                          const size_t bad_index)
{
    printf("  %s: ", label);

    if (status == STATUS_OK) {
        if (found_count == 0) {
            printf("чисел Капрекара нет\n");
            return;
        }
        for (size_t i = 0; i < found_count; i++) {
            printf("%s%s", i > 0 ? ", " : "", found[i]);
        }
        printf("\n");
    } else if (status == STATUS_ERR_NUMBER_INVALID || status == STATUS_ERR_NUMBER_TOO_BIG) {
        printf("ошибка в строке №%zu - %s\n", bad_index + 1, status_message(status));
    } else {
        print_error(status);
    }
}

static void show_root(const char *label, const Status status, const double root, const double eps)
{
    printf("  %s, eps = %.15g: ", label, eps);
    if (status == STATUS_OK) {
        printf("x = %.*f\n", decimals_from_eps(eps), root);
    } else {
        print_error(status);
    }
}

static void demo_convex(void)
{
    const double eps = 1e-9;
    bool convex = false;
    Status status;

    printf("6.1. Выпуклость многоугольника (eps = %.15g)\n", eps);

    status = is_convex_polygon(&convex, eps, 4, 0.0, 0.0, 4.0, 0.0, 4.0, 4.0, 0.0, 4.0);
    show_convex("квадрат (0;0) (4;0) (4;4) (0;4), против часовой", status, convex);

    status = is_convex_polygon(&convex, eps, 4, 0.0, 0.0, 0.0, 4.0, 4.0, 4.0, 4.0, 0.0);
    show_convex("тот же квадрат по часовой", status, convex);

    status = is_convex_polygon(&convex, eps, 5, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 2.0, 2.0, 2.0, 0.0);
    show_convex("прямоугольник с вершиной (0;1) на стороне", status, convex);

    status = is_convex_polygon(&convex, eps, 3, 0.0, 0.0, 1e300, 0.0, 0.0, 1e300);
    show_convex("треугольник с катетами 1e300", status, convex);

    status = is_convex_polygon(&convex, eps, 3, 0.0, 0.0, 1e-300, 0.0, 0.0, 1e-300);
    show_convex("треугольник с катетами 1e-300", status, convex);

    status = is_convex_polygon(&convex, eps, 5, 0.0, 0.0, 4.0, 0.0, 4.0, 4.0, 2.0, 1.0, 0.0, 4.0);
    show_convex("пятиугольник с вершиной (2;1), вдавленной внутрь", status, convex);

    const double pi = acos(-1.0);
    double sx[5];
    double sy[5];
    for (int i = 0; i < 5; i++) {
        const double angle = pi / 2.0 + 4.0 * pi * (double) i / 5.0;
        sx[i] = cos(angle);
        sy[i] = sin(angle);
    }
    status = is_convex_polygon(&convex, eps, 5, sx[0], sy[0], sx[1], sy[1], sx[2], sy[2],
                               sx[3], sy[3], sx[4], sy[4]);
    show_convex("пятиконечная звезда (повороты в одну сторону, но два оборота)", status, convex);

    status = is_convex_polygon(&convex, eps, 2, 0.0, 0.0, 1.0, 1.0);
    show_convex("2 вершины", status, convex);

    status = is_convex_polygon(&convex, eps, 4, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0);
    show_convex("вершина (1;0) повторена дважды", status, convex);

    status = is_convex_polygon(&convex, eps, 4, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    show_convex("последняя вершина повторяет первую", status, convex);

    status = is_convex_polygon(&convex, eps, 3, 0.0, 0.0, 1.0, 1.0, 2.0, 2.0);
    show_convex("все вершины на одной прямой", status, convex);

    status = is_convex_polygon(&convex, eps, 4, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 1.0, 1.0);
    show_convex("разворот назад (0;0) (2;0) (1;0)", status, convex);

    status = is_convex_polygon(&convex, eps, 3, 0.0, 0.0, NAN, 0.0, 0.0, 1.0);
    show_convex("координата nan", status, convex);

    status = is_convex_polygon(&convex, eps, 3, -1e308, 0.0, 1e308, 0.0, 0.0, 1e308);
    show_convex("треугольник с основанием 2e308 (длиннее максимального double)", status, convex);

    status = is_convex_polygon(&convex, eps, 3, 0.0, 0.0, 1.0, 0.0, 0.5, 1e-12);
    show_convex("сплющенный треугольник: высота 1e-12, углы меньше eps", status, convex);

    status = is_convex_polygon(&convex, eps, 3, 0.0, 0.0, 4.9e-324, 0.0, 0.0, 4.9e-324);
    show_convex("треугольник с катетами 4.9e-324 (наименьший double)", status, convex);

    status = is_convex_polygon(&convex, 0.0, 3, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0);
    show_convex("eps = 0", status, convex);

    status = is_convex_polygon(NULL, eps, 3, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0);
    show_convex("пустой указатель на результат", status, convex);

    printf("\n");
}

static void demo_polynomial(void)
{
    double value = 0.0;
    Status status;

    printf("6.2. Значение многочлена (схема Горнера)\n");

    status = polynomial_value(&value, 2.0, 3, 2.0, -3.0, 0.0, 5.0);
    show_number("2x^3 - 3x^2 + 5 при x = 2", status, value);

    status = polynomial_value(&value, 6.0, 3, 1.0, 0.0, -4.0, 9.0);
    show_number("x^3 - 4x + 9 при x = 6", status, value);

    status = polynomial_value(&value, -1.5, 4, 1.0, 0.0, 0.0, 0.0, 0.0);
    show_number("x^4 при x = -1.5", status, value);

    status = polynomial_value(&value, 3.0, 2, 0.0, 1.0, 1.0);
    show_number("0x^2 + x + 1 при x = 3 (старший коэффициент 0)", status, value);

    status = polynomial_value(&value, 123.0, 0, 7.0);
    show_number("многочлен степени 0, равный 7", status, value);

    status = polynomial_value(&value, 1e200, 2, 1.0, 0.0, 0.0);
    show_number("x^2 при x = 1e200", status, value);

    status = polynomial_value(&value, 2.0, -1);
    show_number("степень -1", status, value);

    status = polynomial_value(&value, 2.0, 1, NAN, 1.0);
    show_number("коэффициент nan", status, value);

    status = polynomial_value(&value, INFINITY, 1, 1.0, 1.0);
    show_number("x = inf", status, value);

    printf("\n");
}

static void demo_kaprekar(void)
{
    const char *found[KAPREKAR_CAPACITY];
    size_t count = 0;
    size_t bad = 0;
    Status status;

    printf("6.3. Числа Капрекара\n");

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 14,
                           "1", "9", "45", "55", "99", "297", "703", "999",
                           "4879", "5292", "10", "100", "0", "12");
    show_kaprekar("основание 10: 1 9 45 55 99 297 703 999 4879 5292 10 100 0 12",
                  status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 16, 9,
                           "1", "6", "A", "f", "33", "AB", "FF", "45", "10");
    show_kaprekar("основание 16: 1 6 A f 33 AB FF 45 10", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 2, 6,
                           "1", "11", "110", "111", "1010", "10");
    show_kaprekar("основание 2: 1 11 110 111 1010 10", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 36, 3, "Z", "z", "10");
    show_kaprekar("основание 36: Z z 10", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 2, "0000045", "999999999");
    show_kaprekar("основание 10: 0000045 999999999", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 0);
    show_kaprekar("пустой список", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 2, "4294967295", "4294967296");
    show_kaprekar("основание 10: 4294967295 4294967296", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 3, "45", "12a", "55");
    show_kaprekar("основание 10: 45 12a 55", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 8, 2, "7", "8");
    show_kaprekar("основание 8: 7 8", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 4, "1", "", "-5", " 5");
    show_kaprekar("основание 10: 1, пустая строка, -5, пробел+5", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 10, 2, "9", (const char *) NULL);
    show_kaprekar("основание 10: 9, NULL", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 1, 1, "1");
    show_kaprekar("основание 1", status, found, count, bad);

    status = find_kaprekar(found, KAPREKAR_CAPACITY, &count, &bad, 37, 1, "1");
    show_kaprekar("основание 37", status, found, count, bad);

    status = find_kaprekar(found, 2, &count, &bad, 10, 3, "1", "9", "45");
    show_kaprekar("3 числа, а массив для результата на 2", status, found, count, bad);

    printf("\n");
}

static void demo_mean(void)
{
    double value = 0.0;
    Status status;

    printf("6.4. Среднее геометрическое\n");

    status = geometric_mean(&value, 3, 2.0, 4.0, 8.0);
    show_number("2, 4, 8", status, value);

    status = geometric_mean(&value, 5, 5.8, 4.0, 3.9, 168.25, 61.8);
    show_number("5.8, 4, 3.9, 168.25, 61.8", status, value);

    status = geometric_mean(&value, 1, 9.0);
    show_number("одно число 9", status, value);

    status = geometric_mean(&value, 3, 1e200, 1e200, 1e-200);
    show_number("1e200, 1e200, 1e-200 (произведение переполнило бы double)", status, value);

    status = geometric_mean(&value, 2, 1e-300, 1e-300);
    show_number("1e-300, 1e-300 (произведение обнулилось бы)", status, value);

    status = geometric_mean(&value, 3, 2.0, 0.0, 8.0);
    show_number("2, 0, 8", status, value);

    status = geometric_mean(&value, 2, -2.0, -8.0);
    show_number("-2, -8", status, value);

    status = geometric_mean(&value, 0);
    show_number("ноль чисел", status, value);

    status = geometric_mean(&value, 2, 1.0, NAN);
    show_number("1, nan", status, value);

    status = geometric_mean(&value, 2, 1.0, INFINITY);
    show_number("1, inf", status, value);

    printf("\n");
}

static void show_power(const double x, const int exponent)
{
    double value = 0.0;
    const Status status = fast_power(&value, x, exponent);

    printf("  (%.15g)^(%d): ", x, exponent);
    if (status == STATUS_OK) {
        printf("%.15g\n", value);
    } else {
        print_error(status);
    }
}

static void demo_power(void)
{
    printf("6.5. Быстрое возведение в целую степень (рекурсивно)\n");

    show_power(2.0, 10);
    show_power(1.5, 4);
    show_power(2.0, -3);
    show_power(-2.0, 3);
    show_power(-2.0, 4);
    show_power(3.0, 20);
    show_power(1.1, 10);
    show_power(0.0, 0);
    show_power(0.0, 5);
    show_power(0.0, -1);
    show_power(10.0, 308);
    show_power(10.0, 309);
    show_power(10.0, -307);
    show_power(0.5, 1022);
    show_power(0.5, 1023);
    show_power(2.0, -1023);
    show_power(2.0, INT_MIN);
    show_power(1.0, INT_MIN);
    show_power(-1.0, INT_MAX);
    show_power(-1.0, INT_MIN);
    show_power(NAN, 2);

    printf("\n");
}

static double square_minus_two(double x)
{
    return x * x - 2.0;
}

static double cubic(double x)
{
    return x * x * x - x - 3.0;
}

static double cosine_minus_x(double x)
{
    return cos(x) - x;
}

static double exp_minus_two(double x)
{
    return exp(x) - 2.0;
}

static double sine(double x)
{
    return sin(x);
}

static double identity(double x)
{
    return x;
}

static double square_plus_one(double x)
{
    return x * x + 1.0;
}

static double natural_log(double x)
{
    return log(x);
}

static void demo_bisection(void)
{
    double root = 0.0;
    Status status;

    printf("6.6. Метод дихотомии\n");

    status = bisection(&root, square_minus_two, 0.0, 2.0, 1e-3);
    show_root("x^2 - 2 = 0 на [0; 2]", status, root, 1e-3);

    status = bisection(&root, square_minus_two, 0.0, 2.0, 1e-6);
    show_root("x^2 - 2 = 0 на [0; 2]", status, root, 1e-6);

    status = bisection(&root, square_minus_two, 0.0, 2.0, 1e-12);
    show_root("x^2 - 2 = 0 на [0; 2]", status, root, 1e-12);

    status = bisection(&root, square_minus_two, -2.0, 0.0, 1e-6);
    show_root("x^2 - 2 = 0 на [-2; 0]", status, root, 1e-6);

    status = bisection(&root, cubic, 1.0, 2.0, 1e-9);
    show_root("x^3 - x - 3 = 0 на [1; 2]", status, root, 1e-9);

    status = bisection(&root, cosine_minus_x, 0.0, 1.0, 1e-9);
    show_root("cos x - x = 0 на [0; 1]", status, root, 1e-9);

    status = bisection(&root, exp_minus_two, 0.0, 1.0, 1e-9);
    show_root("e^x - 2 = 0 на [0; 1]", status, root, 1e-9);

    status = bisection(&root, sine, 3.0, 4.0, 1e-12);
    show_root("sin x = 0 на [3; 4]", status, root, 1e-12);

    status = bisection(&root, natural_log, 0.5, 2.0, 1e-6);
    show_root("ln x = 0 на [0.5; 2]", status, root, 1e-6);

    status = bisection(&root, identity, 0.0, 1.0, 1e-6);
    show_root("x = 0 на [0; 1] (корень на границе)", status, root, 1e-6);

    status = bisection(&root, square_plus_one, -1.0, 1.0, 1e-6);
    show_root("x^2 + 1 = 0 на [-1; 1]", status, root, 1e-6);

    status = bisection(&root, natural_log, -1.0, 2.0, 1e-6);
    show_root("ln x = 0 на [-1; 2]", status, root, 1e-6);

    status = bisection(&root, square_minus_two, 2.0, 0.0, 1e-6);
    show_root("x^2 - 2 = 0 на [2; 0]", status, root, 1e-6);

    status = bisection(&root, square_minus_two, 0.0, 2.0, 0.0);
    show_root("x^2 - 2 = 0 на [0; 2]", status, root, 0.0);

    status = bisection(&root, square_minus_two, 1.0, 2.0, 1e-20);
    show_root("x^2 - 2 = 0 на [1; 2]", status, root, 1e-20);

    status = bisection(&root, square_minus_two, NAN, 2.0, 1e-6);
    show_root("x^2 - 2 = 0 на [nan; 2]", status, root, 1e-6);

    status = bisection(&root, NULL, 0.0, 2.0, 1e-6);
    show_root("пустой указатель на функцию", status, root, 1e-6);
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    demo_convex();
    demo_polynomial();
    demo_kaprekar();
    demo_mean();
    demo_power();
    demo_bisection();
    return 0;
}
