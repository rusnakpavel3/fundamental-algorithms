#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_NUMBER_INVALID:
            return "не является корректным числом (дробная часть через точку)";
        case STATUS_ERR_NUMBER_OUT_OF_RANGE:
            return "число вне диапазона типа double";
        case STATUS_ERR_NUMBER_TOO_PRECISE:
            return "больше 15 значащих цифр: тип double хранит только 15, число исказилось бы";
        case STATUS_ERR_NUMBER_NOT_FINITE:
            return "число должно быть конечным (inf и nan недопустимы)";
        case STATUS_ERR_EPS_NOT_POSITIVE:
            return "эпсилон должен быть строго больше 0";
        case STATUS_ERR_EPS_TOO_LARGE:
            return "эпсилон должен быть меньше 1";
        case STATUS_ERR_VALUE_OUT_OF_RANGE:
            return "значение вне допустимого диапазона";
        case STATUS_ERR_COMPUTE_OVERFLOW:
            return "переполнение при вычислениях";
        case STATUS_ERR_NOT_CONVERGED:
            return "не сошлось за допустимое число шагов";
        case STATUS_ERR_SERIES_DIVERGES:
            return "ряд расходится при данном x (x вне области сходимости)";
        case STATUS_ERR_PRECISION_UNREACHABLE:
            return "точность eps недостижима в типе double (ошибка округления больше eps)";
    }
    return "неизвестный статус";
}
