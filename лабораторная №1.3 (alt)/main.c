#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "logic.h"
#include "parsing.h"
#include "status.h"

static void print_status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            break;
        case STATUS_ERR_NULL_ARGUMENT:
            printf("внутренняя ошибка: передан пустой указатель\n");
            break;
        case STATUS_ERR_FLAG_FORMAT:
            printf("неверный формат флага: нужен '-' или '/' и одна буква\n");
            break;
        case STATUS_ERR_FLAG_UNKNOWN:
            printf("неизвестный флаг: допустимы q, m, t\n");
            break;
        case STATUS_ERR_NUMBER_INVALID:
            printf("не является корректным числом (дробная часть через точку)\n");
            break;
        case STATUS_ERR_NUMBER_OUT_OF_RANGE:
            printf("число вне диапазона double (по модулю больше 1.8e308 или слишком близко к нулю)\n");
            break;
        case STATUS_ERR_NUMBER_TOO_PRECISE:
            printf("больше 15 значащих цифр: тип double хранит только 15, число исказилось бы\n");
            break;
        case STATUS_ERR_INTEGER_OUT_OF_RANGE:
            printf("целое число вне диапазона long long (от -9223372036854775808 до 9223372036854775807)\n");
            break;
        case STATUS_ERR_NUMBER_NOT_FINITE:
            printf("число должно быть конечным (inf и nan недопустимы)\n");
            break;
        case STATUS_ERR_EPS_NOT_POSITIVE:
            printf("эпсилон должен быть строго больше 0\n");
            break;
        case STATUS_ERR_EPS_TOO_LARGE:
            printf("эпсилон должен быть меньше 1\n");
            break;
        case STATUS_ERR_ZERO_NOT_ALLOWED:
            printf("числа должны быть ненулевыми\n");
            break;
        case STATUS_ERR_VALUE_OUT_OF_RANGE:
            printf("значение вне допустимого диапазона\n");
            break;
        case STATUS_ERR_COMPUTE_OVERFLOW:
            printf("переполнение при вычислениях\n");
            break;
    }
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

static int decimals_for(const double x, const int digits)
{
    const double magnitude = fabs(x);
    int integer_digits = 1;

    if (magnitude >= 1.0) {
        integer_digits = (int) floor(log10(magnitude)) + 1;
    }

    const int max_decimals = 15 - integer_digits;
    if (max_decimals < 0) {
        return 0;
    }
    if (digits < max_decimals) {
        return digits;
    }
    return max_decimals;
}

static void print_real(const double x, const double eps, const int digits)
{
    double shown = x;
    if (fabs(x) <= eps) {
        shown = 0.0;
    }

    if (fabs(shown) >= 1e15) {
        printf("%.*e", digits < 14 ? digits : 14, shown);
    } else {
        printf("%.*f", decimals_for(shown, digits), shown);
    }
}

static void print_input(const double x)
{
    printf("%.15g", x);
}

static Status parse_real_param(const char *arg, const int position, double *out)
{
    const Status status = parse_double(arg, out);

    if (status != STATUS_OK) {
        printf("Ошибка в параметре %d (\"%s\"): ", position, arg);
        print_status_message(status);
    }
    return status;
}

static Status parse_integer_param(const char *arg, const int position, long long *out)
{
    const Status status = parse_long_long(arg, out);

    if (status != STATUS_OK) {
        printf("Ошибка в параметре %d (\"%s\"): ", position, arg);
        print_status_message(status);
    }
    return status;
}

static void print_solution(const QuadraticSolution *s, const double eps, const int digits)
{
    switch (s->kind) {
        case QUAD_TWO_ROOTS:
            printf("x1 = ");
            print_real(s->x1, eps, digits);
            printf(", x2 = ");
            print_real(s->x2, eps, digits);
            break;
        case QUAD_ONE_DOUBLE_ROOT:
            printf("один корень кратности 2: x = ");
            print_real(s->x1, eps, digits);
            break;
        case QUAD_NO_REAL_ROOTS:
            printf("вещественных корней нет (D < 0)");
            break;
        case QUAD_LINEAR_ONE_ROOT:
            printf("a = 0 с точностью eps, уравнение линейное: x = ");
            print_real(s->x1, eps, digits);
            break;
        case QUAD_NO_SOLUTIONS:
            printf("a = 0 и b = 0 с точностью eps, c != 0: решений нет");
            break;
        case QUAD_INFINITE_SOLUTIONS:
            printf("a, b и c равны 0 с точностью eps: решение - любое x");
            break;
    }
}

static Status run_quadratic(char *const argv[])
{
    double eps;
    double a;
    double b;
    double c;

    Status status = parse_real_param(argv[2], 1, &eps);
    if (status != STATUS_OK) {
        return status;
    }
    status = parse_real_param(argv[3], 2, &a);
    if (status != STATUS_OK) {
        return status;
    }
    status = parse_real_param(argv[4], 3, &b);
    if (status != STATUS_OK) {
        return status;
    }
    status = parse_real_param(argv[5], 4, &c);
    if (status != STATUS_OK) {
        return status;
    }

    Coefficients perms[MAX_PERMUTATIONS];
    size_t count = 0;

    status = build_unique_permutations(a, b, c, eps, perms, MAX_PERMUTATIONS, &count);
    if (status != STATUS_OK) {
        printf("Ошибка: ");
        print_status_message(status);
        return status;
    }

    const int digits = precision_from_eps(eps);
    printf("Уникальных перестановок: %zu\n", count);

    for (size_t i = 0; i < count; i++) {
        printf("%zu) (", i + 1);
        print_input(perms[i].a);
        printf(")*x^2 + (");
        print_input(perms[i].b);
        printf(")*x + (");
        print_input(perms[i].c);
        printf(") = 0: ");

        QuadraticSolution solution;
        status = solve_quadratic(&perms[i], eps, &solution);

        if (status == STATUS_OK) {
            print_solution(&solution, eps, digits);
            printf("\n");
        } else if (status == STATUS_ERR_COMPUTE_OVERFLOW) {
            printf("не удалось решить: переполнение при вычислениях\n");
        } else {
            printf("\n");
            printf("Ошибка: ");
            print_status_message(status);
            return status;
        }
    }
    return STATUS_OK;
}

static Status run_multiple(char *const argv[])
{
    long long first;
    long long second;

    Status status = parse_integer_param(argv[2], 1, &first);
    if (status != STATUS_OK) {
        return status;
    }
    status = parse_integer_param(argv[3], 2, &second);
    if (status != STATUS_OK) {
        return status;
    }

    bool is_multiple = false;
    status = check_multiple(first, second, &is_multiple);
    if (status != STATUS_OK) {
        printf("Ошибка: ");
        print_status_message(status);
        return status;
    }

    if (is_multiple) {
        printf("%lld кратно %lld\n", first, second);
    } else {
        printf("%lld не кратно %lld\n", first, second);
    }
    return STATUS_OK;
}

static Status run_triangle(char *const argv[])
{
    double eps;
    double sides[3];

    Status status = parse_real_param(argv[2], 1, &eps);
    if (status != STATUS_OK) {
        return status;
    }
    for (int i = 0; i < 3; i++) {
        status = parse_real_param(argv[3 + i], 2 + i, &sides[i]);
        if (status != STATUS_OK) {
            return status;
        }
    }

    TriangleVerdict verdict;
    int index = -1;

    status = check_right_triangle(sides[0], sides[1], sides[2], eps, &verdict, &index);
    if (status != STATUS_OK) {
        printf("Ошибка: ");
        print_status_message(status);
        return status;
    }

    switch (verdict) {
        case TRIANGLE_RIGHT:
            printf("Могут: гипотенуза ");
            print_input(sides[index]);
            printf(", катеты ");
            print_input(sides[(index + 1) % 3]);
            printf(" и ");
            print_input(sides[(index + 2) % 3]);
            printf("\n");
            break;
        case TRIANGLE_NOT_RIGHT:
            printf("Не могут: теорема Пифагора не выполняется ни при каком выборе гипотенузы\n");
            break;
        case TRIANGLE_BAD_SIDE:
            printf("Не могут: длина стороны (параметр %d) должна быть больше eps\n", index + 2);
            break;
    }
    return STATUS_OK;
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc < 2) {
        printf("Ошибка: не указан флаг. Использование: program <flag> <параметры>\n");
        return 1;
    }

    Flag flag;
    Status status = parse_flag(argv[1], &flag);
    if (status != STATUS_OK) {
        printf("Ошибка в флаге \"%s\": ", argv[1]);
        print_status_message(status);
        return 1;
    }

    int needed = 0;
    status = required_argc(flag, &needed);
    if (status != STATUS_OK) {
        printf("Ошибка: ");
        print_status_message(status);
        return 1;
    }
    if (argc != needed) {
        printf("Ошибка: для флага %s нужно параметров: %d, передано: %d\n",
               argv[1], needed - 2, argc - 2);
        return 1;
    }

    switch (flag) {
        case FLAG_QUADRATIC:
            status = run_quadratic(argv);
            break;
        case FLAG_MULTIPLE:
            status = run_multiple(argv);
            break;
        case FLAG_TRIANGLE:
            status = run_triangle(argv);
            break;
    }

    if (status != STATUS_OK) {
        return 1;
    }
    return 0;
}
