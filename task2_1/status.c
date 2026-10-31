#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "неверное количество аргументов командной строки";
        case STATUS_ERR_FLAG_FORMAT:
            return "неверный формат флага";
        case STATUS_ERR_FLAG_UNKNOWN:
            return "неизвестный флаг";
        case STATUS_ERR_NUMBER_INVALID:
            return "некорректная запись числа";
        case STATUS_ERR_NUMBER_OUT_OF_RANGE:
            return "число вне допустимого диапазона";
        case STATUS_ERR_EMPTY_COLLECTION:
            return "не передано ни одной строки";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "переполнение размера результата";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
    }
    return "неизвестный статус";
}
