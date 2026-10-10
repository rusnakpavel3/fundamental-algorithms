#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_NUMBER_INVALID:
            return "не является корректным целым числом";
        case STATUS_ERR_NUMBER_OUT_OF_RANGE:
            return "число вне диапазона типа int";
        case STATUS_ERR_RANGE_INVALID:
            return "левая граница диапазона больше правой (нужно a <= b)";
        case STATUS_ERR_SIZE_INVALID:
            return "размер массива должен быть больше 0";
        case STATUS_ERR_COMPUTE_OVERFLOW:
            return "переполнение при сложении";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
    }
    return "неизвестный статус";
}
