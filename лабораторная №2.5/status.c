#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель (поток, строка-приёмник или формат)";
        case STATUS_ERR_FORMAT_INVALID:
            return "неверная спецификация в строке формата";
        case STATUS_ERR_FORMAT_UNSUPPORTED:
            return "спецификация не поддерживается (широкие символы %lc/%ls, %n с модификатором длины)";
        case STATUS_ERR_SPEC_TOO_LONG:
            return "спецификация в строке формата слишком длинная";
        case STATUS_ERR_POINTER_ARGUMENT_NULL:
            return "аргумент-указатель для спецификации равен NULL";
        case STATUS_ERR_ROMAN_OUT_OF_RANGE:
            return "римскими цифрами записываются только числа от 1 до 3999";
        case STATUS_ERR_BASE_OUT_OF_RANGE:
            return "основание системы счисления должно быть от 2 до 36";
        case STATUS_ERR_BASE_NUMBER_EMPTY:
            return "в записи числа нет ни одной цифры";
        case STATUS_ERR_BASE_NUMBER_INVALID:
            return "в записи числа есть символ, который не является цифрой этой системы (или буква не того регистра)";
        case STATUS_ERR_BASE_NUMBER_OVERFLOW:
            return "число не помещается в long long";
        case STATUS_ERR_FLOAT_OUT_OF_RANGE:
            return "значение не помещается в тип float";
        case STATUS_ERR_BUFFER_TOO_SMALL:
            return "внутренняя ошибка: буфер для результата слишком мал";
        case STATUS_ERR_STANDARD_FORMAT_FAILED:
            return "стандартное форматирование завершилось ошибкой";
        case STATUS_ERR_OUTPUT_TOO_LONG:
            return "результат слишком длинный";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_WRITE_FAILED:
            return "ошибка записи в поток";
    }
    return "неизвестный статус";
}
