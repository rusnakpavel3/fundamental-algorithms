#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "нужно указать два аргумента: входной файл и выходной файл";
        case STATUS_ERR_SAME_FILE:
            return "входной и выходной файл совпадают";
        case STATUS_ERR_FILE_INFO_FAILED:
            return "не удалось получить сведения о файле";
        case STATUS_ERR_INPUT_OPEN_FAILED:
            return "не удалось открыть входной файл (нет такого файла или нет доступа)";
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
            return "не удалось открыть выходной файл (нет такой папки или нет доступа)";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения файла";
        case STATUS_ERR_WRITE_FAILED:
            return "ошибка записи в файл";
        case STATUS_ERR_CLOSE_FAILED:
            return "ошибка при закрытии файла (данные могли не записаться)";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_NUL_BYTE:
            return "строка содержит нулевой байт - это не текстовый файл";
        case STATUS_ERR_INPUT_EMPTY:
            return "входной файл пуст";
        case STATUS_ERR_EMPTY_LINE:
            return "пустая строка - это не скобочное выражение";
        case STATUS_ERR_EXPECTED_NAME:
            return "ожидалось имя узла";
        case STATUS_ERR_EXPECTED_SEPARATOR:
            return "ожидалась ',' или ')'";
        case STATUS_ERR_TRAILING_TEXT:
            return "лишний текст после конца выражения";
        case STATUS_ERR_TOO_DEEP:
            return "слишком глубокая вложенность (больше 1000 уровней)";
    }
    return "неизвестный статус";
}
