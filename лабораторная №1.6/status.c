#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_COUNT_INVALID:
            return "недопустимое количество (вершин, чисел или степень многочлена)";
        case STATUS_ERR_NOT_FINITE:
            return "среди чисел есть inf или nan";
        case STATUS_ERR_EPS_INVALID:
            return "недопустимый eps";
        case STATUS_ERR_POLYGON_DEGENERATE:
            return "вырожденный многоугольник (совпадающие соседние вершины, разворот назад или все вершины на одной прямой)";
        case STATUS_ERR_OVERFLOW:
            return "переполнение: результат по модулю больше максимального double";
        case STATUS_ERR_UNDERFLOW:
            return "результат по модулю меньше 2.2e-308: double хранит такие числа с потерей точности или как 0";
        case STATUS_ERR_BASE_INVALID:
            return "основание системы счисления должно быть от 2 до 36";
        case STATUS_ERR_NUMBER_INVALID:
            return "строка не является записью неотрицательного целого числа в этой системе счисления";
        case STATUS_ERR_NUMBER_TOO_BIG:
            return "число больше 4294967295: его квадрат не помещается в 64 бита";
        case STATUS_ERR_CAPACITY_TOO_SMALL:
            return "массив для результата меньше количества чисел";
        case STATUS_ERR_NEGATIVE_NUMBER:
            return "среднее геометрическое определено только для неотрицательных чисел";
        case STATUS_ERR_DIVISION_BY_ZERO:
            return "ноль в отрицательной степени: деление на ноль";
        case STATUS_ERR_INVALID_INTERVAL:
            return "левая граница отрезка должна быть меньше правой";
        case STATUS_ERR_NO_SIGN_CHANGE:
            return "на концах отрезка функция одного знака: корень не гарантирован";
        case STATUS_ERR_FUNCTION_NOT_FINITE:
            return "функция не определена в точке отрезка (получено inf или nan)";
        case STATUS_ERR_PRECISION_UNREACHABLE:
            return "точность eps недостижима: отрезок сузился до соседних чисел double";
    }
    return "неизвестный статус";
}
