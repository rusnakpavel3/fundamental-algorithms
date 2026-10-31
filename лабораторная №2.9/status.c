#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_BASE_OUT_OF_RANGE:
            return "основание системы счисления должно быть не меньше 2";
        case STATUS_ERR_EPSILON_OUT_OF_RANGE:
            return "эпсилон должен быть конечным числом из промежутка [1e-15; 1e-12]";
        case STATUS_ERR_COUNT_NEGATIVE:
            return "количество дробей не может быть отрицательным";
        case STATUS_ERR_FRACTION_NOT_FINITE:
            return "дробь не является конечным числом (nan или бесконечность)";
        case STATUS_ERR_FRACTION_OUT_OF_RANGE:
            return "дробь должна лежать в интервале (0; 1)";
        case STATUS_ERR_NOT_CONVERGED:
            return "не удалось подобрать обыкновенную дробь p/q с точностью эпсилон";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком много дробей: размер массива не помещается в size_t";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
    }
    return "неизвестный статус";
}
