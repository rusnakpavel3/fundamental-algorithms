#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_INPUT_END:
            return "вход закончился раньше первого преобразования";
        case STATUS_MATCH_FAILURE:
            return "вход не соответствует формату";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель (поток, строка или формат)";
        case STATUS_ERR_FORMAT_INVALID:
            return "неверная спецификация в строке формата";
        case STATUS_ERR_FORMAT_UNSUPPORTED:
            return "спецификация не поддерживается (широкие символы %lc/%ls/%l[, %n с модификатором длины)";
        case STATUS_ERR_SPEC_TOO_LONG:
            return "спецификация в строке формата слишком длинная";
        case STATUS_ERR_POINTER_ARGUMENT_NULL:
            return "указатель для записи результата равен NULL";
        case STATUS_ERR_BASE_OUT_OF_RANGE:
            return "основание системы счисления должно быть от 2 до 36";
        case STATUS_ERR_INPUT_TOO_LONG:
            return "прочитано больше символов, чем помещается в int (для %n)";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения из потока";
    }
    return "неизвестный статус";
}
