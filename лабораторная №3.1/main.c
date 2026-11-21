#include <limits.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "convert.h"
#include "status.h"

static void show(const int number, const int power, char *out, const size_t capacity)
{
    const Status status = to_power_of_two_base(number, power, out, capacity);

    switch (status) {
        case STATUS_OK:
            printf("%d при r = %d: %s\n", number, power, out);
            break;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_POWER_OUT_OF_RANGE:
        case STATUS_ERR_BUFFER_TOO_SMALL:
            printf("%d при r = %d: ошибка: %s\n", number, power, status_message(status));
            break;
    }
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    char out[34];

    printf("Корректные:\n");
    for (int power = 1; power <= 5; power++) {
        show(45, power, out, sizeof out);
    }
    show(-45, 4, out, sizeof out);

    printf("\nГраничные:\n");
    show(0, 3, out, sizeof out);
    show(INT_MAX, 4, out, sizeof out);
    show(INT_MIN, 1, out, sizeof out);
    show(INT_MIN, 5, out, sizeof out);
    show(45, 1, out, 7);

    printf("\nОшибки:\n");
    show(45, 1, out, 6);
    show(45, 0, out, sizeof out);
    show(45, 6, out, sizeof out);
    show(45, 4, NULL, sizeof out);

    return 0;
}
