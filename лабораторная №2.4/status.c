#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком много данных: размер массива не помещается в size_t";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения стандартного потока ввода";
        case STATUS_ERR_NUL_BYTE:
            return "во входных данных встретился нулевой байт";
        case STATUS_ERR_INPUT_EMPTY:
            return "входные данные пусты: нет первой строки \"n capacity\"";
        case STATUS_ERR_HEADER_FORMAT:
            return "первая строка должна содержать ровно два числа: n и capacity";
        case STATUS_ERR_COUNT_INVALID:
            return "количество файлов n должно быть положительным целым числом";
        case STATUS_ERR_CAPACITY_INVALID:
            return "вместимость накопителя должна быть неотрицательным целым числом";
        case STATUS_ERR_LINE_FORMAT:
            return "строка файла должна содержать ровно три значения: filename size importance";
        case STATUS_ERR_SIZE_INVALID:
            return "размер файла должен быть положительным целым числом";
        case STATUS_ERR_IMPORTANCE_INVALID:
            return "важность файла должна быть неотрицательным целым числом";
        case STATUS_ERR_NUMBER_INVALID:
            return "значение не является целым числом";
        case STATUS_ERR_NUMBER_TOO_BIG:
            return "число вне диапазона long long: по модулю больше 9223372036854775807";
        case STATUS_ERR_TOO_FEW_LINES:
            return "строк с описаниями файлов меньше, чем указано в n";
        case STATUS_ERR_DUPLICATE_NAME:
            return "имя файла повторяется (имена должны быть уникальными)";
        case STATUS_ERR_INVALID_FILE:
            return "некорректное описание файла: пустое имя, размер <= 0 или важность < 0";
        case STATUS_ERR_IMPORTANCE_OVERFLOW:
            return "суммарная важность всех файлов не помещается в long long";
        case STATUS_ERR_PROBLEM_TOO_LARGE:
            return "задача слишком велика: алгоритму нужно больше 200 МБ памяти ((n + 8) x (min(capacity, сумма размеров) + 1) байт)";
    }
    return "неизвестный статус";
}
