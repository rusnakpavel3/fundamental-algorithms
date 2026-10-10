#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_BASE_INVALID:
            return "основание должно быть целым числом от 2 до 36";
        case STATUS_ERR_NUMBER_INVALID:
            return "недопустимый символ (разрешены цифры 0-9, прописные буквы A-Z и знак '-' в начале)";
        case STATUS_ERR_NUMBER_LOWERCASE:
            return "цифры больше 9 нужно вводить прописными буквами";
        case STATUS_ERR_DIGIT_OUT_OF_BASE:
            return "цифра недопустима в системе с этим основанием";
        case STATUS_ERR_NUMBER_TOO_BIG:
            return "число не помещается в long long";
        case STATUS_ERR_SUM_OVERFLOW:
            return "не помещается в long long (по модулю больше 9223372036854775807)";
        case STATUS_ERR_VALUE_OUT_OF_RANGE:
            return "значение вне допустимого диапазона";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения ввода";
    }
    return "неизвестный статус";
}
