#ifdef _WIN32
#include <windows.h>
#endif

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "over_printf.h"
#include "status.h"

static void report(const int result)
{
    const Status status = over_result_status(result);

    if (status != STATUS_OK) {
        printf("ошибка: %s\n", status_message(status));
    }
}

static void demo_standard(void)
{
    int count = 0;

    printf("--- Стандартные спецификаторы ---\n");

    report(overfprintf(stdout, "%d %s %.3f\n", 42, "abc", 3.14159));
    report(overfprintf(stdout, "[%5d] [%-5d] [%05d] [%+d]\n", 42, 42, 42, 42));
    report(overfprintf(stdout, "[%*d] [%.*f]\n", -6, 7, 2, 2.5));
    report(overfprintf(stdout, "%lld %#x %hhd %zu\n", LLONG_MIN, 255, 300, sizeof(double)));
    report(overfprintf(stdout, "%e %Lf %c%s\n", 12345.678, 1.5L, 'w', "ide"));
    report(overfprintf(stdout, "100%%, abc%n", &count));
    printf(" <- %%n записал %d\n", count);
}

static void demo_custom(void)
{
    char buffer[128];

    printf("--- Свои спецификаторы ---\n");

    report(overfprintf(stdout, "%%Ro: %Ro %Ro %Ro\n", 1994, 3999, 4));
    report(overfprintf(stdout, "%%Zr: %Zr %Zr %Zr %Zr\n", 0u, 1u, 10u, 100u));
    report(overfprintf(stdout, "%%Cv/%%CV: %Cv %CV %Cv %Cv\n", 255, 16, 255, 16, -255, 16, 100, 37));
    report(overfprintf(stdout, "%%to/%%TO: %to %TO %to\n", "zz", 36, "7FFFFFFFFFFFFFFF", 16, "-zz", 36));
    report(overfprintf(stdout, "%%mi 1:     %mi\n", 1));
    report(overfprintf(stdout, "%%mi -2:    %mi\n", -2));
    report(overfprintf(stdout, "%%mu max:   %mu\n", UINT32_MAX));
    report(overfprintf(stdout, "%%md 1.0:   %md\n", 1.0));
    report(overfprintf(stdout, "%%mf 1.0f:  %mf\n", 1.0f));
    report(overfprintf(stdout, "смесь: %d = %Ro = %Zr = %Cv (2)\n", 14, 14, 14u, 14, 2));

    const int result = oversprintf(buffer, "%Ro-%Zr-%CV", 14, 4u, 3054, 16);
    report(result);
    if (result >= 0) {
        printf("oversprintf: \"%s\", длина %d\n", buffer, result);
    }
}

static void demo_boundary(void)
{
    printf("--- Граничные случаи ---\n");

    report(overfprintf(stdout, "%%Zr UINT_MAX: %Zr\n", UINT_MAX));
    report(overfprintf(stdout, "%%Cv INT_MIN (2): %Cv\n", INT_MIN, 2));
    report(overfprintf(stdout, "%%to LLONG_MIN (16): %to\n", "-8000000000000000", 16));
    report(overfprintf(stdout, "%%mf -0.0f: %mf\n", -0.0f));
    report(overfprintf(stdout, "пустой формат вернул %d\n", overfprintf(stdout, "")));
}

static void demo_errors(void)
{
    char buffer[64] = "старое содержимое";
    signed char count = 0;

    printf("--- Неверные данные ---\n");

    printf("поток NULL: ");
    report(overfprintf(NULL, "%d", 1));
    printf("%% в конце: ");
    report(overfprintf(stdout, "abc%"));
    printf("%%y: ");
    report(overfprintf(stdout, "%y", 1));
    printf("%%#d: ");
    report(overfprintf(stdout, "%#d", 1));
    printf("%%10Ro: ");
    report(overfprintf(stdout, "%10Ro", 5));
    printf("%%s с NULL: ");
    report(overfprintf(stdout, "%s", (char *) NULL));
    printf("%%Ro 0: ");
    report(overfprintf(stdout, "%Ro", 0));
    printf("%%Ro 4000: ");
    report(overfprintf(stdout, "%Ro", 4000));
    printf("%%to \"FF\" (16): ");
    report(overfprintf(stdout, "%to", "FF", 16));
    printf("%%to \"\" (10): ");
    report(overfprintf(stdout, "%to", "", 10));
    printf("%%TO 2^63 (16): ");
    report(overfprintf(stdout, "%TO", "8000000000000000", 16));
    printf("%%mf 1e300: ");
    report(overfprintf(stdout, "%mf", 1e300));
    printf("%%ls: ");
    report(overfprintf(stdout, "%ls", L"abc"));
    printf("%%hhn: ");
    report(overfprintf(stdout, "abc%hhn", &count));
    printf("ошибка в конце строки: ");
    report(overfprintf(stdout, "начало %d, потом %Ro", 1, 0));

    printf("oversprintf с ошибкой: ");
    report(oversprintf(buffer, "%Ro", -5));
    printf("строка-приёмник после ошибки: \"%s\"\n", buffer);
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    demo_standard();
    demo_custom();
    demo_boundary();
    demo_errors();

    return 0;
}
