#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_BASE_OUT_OF_RANGE:
            return "основание системы счисления должно быть от 2 до 36";
        case STATUS_ERR_COUNT_NEGATIVE:
            return "количество чисел не может быть отрицательным";
        case STATUS_ERR_NUMBER_EMPTY:
            return "число задано пустой строкой";
        case STATUS_ERR_NUMBER_INVALID:
            return "в записи числа есть символ, который не является цифрой этой системы счисления";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "число слишком длинное: размер буфера не помещается в size_t";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
    }
    return "неизвестный статус";
}
