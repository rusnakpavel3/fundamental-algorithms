#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_EPSILON_OUT_OF_RANGE:
            return "эпсилон должен быть конечным числом из интервала (0; 1)";
        case STATUS_ERR_POINT_NOT_FINITE:
            return "точка a (или x) не является конечным числом (nan или бесконечность)";
        case STATUS_ERR_DEGREE_NEGATIVE:
            return "степень многочлена не может быть отрицательной";
        case STATUS_ERR_COEFFICIENT_NOT_FINITE:
            return "коэффициент многочлена не является конечным числом (nan или бесконечность)";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большая степень: размер массива не помещается в size_t";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_RESULT_OVERFLOW:
            return "переполнение: результат не помещается в double";
    }
    return "неизвестный статус";
}
