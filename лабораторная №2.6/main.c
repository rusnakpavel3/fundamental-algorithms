#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>

#include "over_scanf.h"
#include "status.h"

static void report(const char *label, const int result)
{
    const Status status = over_result_status(result);

    if (status == STATUS_OK) {
        printf("%s -> вернула %d", label, result);
    } else if (status == STATUS_INPUT_END) {
        printf("%s -> вернула EOF (%s)", label, status_message(status));
    } else {
        printf("%s -> ошибка: %s", label, status_message(status));
    }
}

static void demo_correct(void)
{
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    unsigned int z1 = 0;
    unsigned int z2 = 0;
    unsigned int z3 = 0;
    unsigned int z4 = 0;
    double x = 0.0;
    char word[16] = "";
    int result = 0;

    printf("--- Корректные данные ---\n");

    result = oversscanf("12 XIV 1011 ff", "%d %Ro %Zr %Cv", &a, &b, &z1, &c, 16);
    report("\"12 XIV 1011 ff\", \"%d %Ro %Zr %Cv\"", result);
    printf(": %d %d %u %d\n", a, b, z1, c);

    result = oversscanf("FF -zz +101", "%CV %Cv %Cv", &a, 16, &b, 36, &c, 2);
    report("\"FF -zz +101\", \"%CV %Cv %Cv\" (16, 36, 2)", result);
    printf(": %d %d %d\n", a, b, c);

    result = oversscanf("11 011 010011 01", "%Zr %Zr %Zr %Zr", &z1, &z2, &z3, &z4);
    report("\"11 011 010011 01\", \"%Zr\" x4", result);
    printf(": %u %u %u %u\n", z1, z2, z3, z4);

    result = oversscanf("MMMCMXCIX I MCMXCIV", "%Ro %Ro %Ro", &a, &b, &c);
    report("\"MMMCMXCIX I MCMXCIV\", \"%Ro\" x3", result);
    printf(": %d %d %d\n", a, b, c);

    result = oversscanf("3.5 abc 0x1F", "%lf %15s %x", &x, word, &d);
    report("\"3.5 abc 0x1F\", \"%lf %15s %x\"", result);
    printf(": %g \"%s\" %d\n", x, word, d);

    result = oversscanf("x=5;y=7", "x=%d;y=%d", &a, &b);
    report("\"x=5;y=7\", \"x=%d;y=%d\"", result);
    printf(": %d %d\n", a, b);

    result = oversscanf("abc 12", "%*s %d%n", &a, &b);
    report("\"abc 12\", \"%*s %d%n\"", result);
    printf(": %d, прочитано символов %d\n", a, b);

    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("не удалось открыть input.txt (в CLion укажи Working directory = папка проекта)\n");
        return;
    }
    result = overfscanf(file, "%Ro %Zr %Cv", &a, &z1, &b, 16);
    report("overfscanf(input.txt), \"%Ro %Zr %Cv\"", result);
    printf(": %d %u %d\n", a, z1, b);
    fclose(file);
}

static void demo_boundary(void)
{
    int a = 0;
    int b = 0;
    unsigned int z = 0;
    char word[16] = "";
    int result = 0;

    printf("--- Граничные случаи ---\n");

    report("\"\", \"%d\"", oversscanf("", "%d", &a));
    printf("\n");
    report("\"   \", \"%Ro\"", oversscanf("   ", "%Ro", &a));
    printf("\n");

    result = oversscanf("-10000000000000000000000000000000", "%Cv", &a, 2);
    report("INT_MIN в двоичной, \"%Cv\"", result);
    printf(": %d\n", a);

    report("\"2147483648\", \"%Cv\" (10) - не помещается в int", oversscanf("2147483648", "%Cv", &a, 10));
    printf("\n");

    result = oversscanf("00100100100010000000100010100010101000010001011", "%Zr", &z);
    report("код UINT_MAX, \"%Zr\"", result);
    printf(": %u\n", z);

    report("код UINT_MAX + 1, \"%Zr\"", oversscanf("10100100100010000000100010100010101000010001011", "%Zr", &z));
    printf("\n");

    result = oversscanf("123", "%Cv", &a, 37);
    report("\"123\", \"%Cv\" (37 -> 10)", result);
    printf(": %d\n", a);

    result = oversscanf("XIVabc", "%Ro%15s", &a, word);
    report("\"XIVabc\", \"%Ro%15s\"", result);
    printf(": %d \"%s\"\n", a, word);

    result = oversscanf("10110", "%Zr%d", &z, &b);
    report("\"10110\", \"%Zr%d\" - конец записи на 11", result);
    printf(": %u %d\n", z, b);

    report("\"1 x\", \"%d %d\"", oversscanf("1 x", "%d %d", &a, &b));
    printf("\n");
}

static void demo_mismatch(void)
{
    static const char *const ROMANS[] = {"IIII", "VX", "MMMM", "xiv", "12"};
    static const char *const CODES[] = {"0001", "1", "101"};
    int a = 0;
    unsigned int z = 0;

    printf("--- Вход не подходит: не ошибка, функция возвращает 0 ---\n");

    for (size_t i = 0; i < sizeof ROMANS / sizeof ROMANS[0]; i++) {
        printf("\"%s\", ", ROMANS[i]);
        report("\"%Ro\"", oversscanf(ROMANS[i], "%Ro", &a));
        printf("\n");
    }
    for (size_t i = 0; i < sizeof CODES / sizeof CODES[0]; i++) {
        printf("\"%s\", ", CODES[i]);
        report("\"%Zr\"", oversscanf(CODES[i], "%Zr", &z));
        printf("\n");
    }
    report("\"FF\", \"%Cv\" (16) - буквы не того регистра", oversscanf("FF", "%Cv", &a, 16));
    printf("\n");
}

static void demo_errors(void)
{
    int a = 0;

    printf("--- Неверные параметры и формат ---\n");

    report("поток NULL", overfscanf(NULL, "%d", &a));
    printf("\n");
    report("строка NULL", oversscanf(NULL, "%d", &a));
    printf("\n");
    report("формат NULL", oversscanf("1", NULL));
    printf("\n");
    report("\"%Ro\" с указателем NULL", oversscanf("XIV", "%Ro", (int *) NULL));
    printf("\n");
    report("\"%y\"", oversscanf("1", "%y", &a));
    printf("\n");
    report("\"%5Ro\"", oversscanf("XIV", "%5Ro", &a));
    printf("\n");
    report("\"abc%\"", oversscanf("xyz", "abc%"));
    printf("\n");
    report("\"%[abc\"", oversscanf("abc", "%[abc", &a));
    printf("\n");
    report("\"%ls\"", oversscanf("abc", "%ls", &a));
    printf("\n");
    report("\"%hhn\"", oversscanf("abc", "%hhn", &a));
    printf("\n");
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    demo_correct();
    demo_boundary();
    demo_mismatch();
    demo_errors();

    return 0;
}
