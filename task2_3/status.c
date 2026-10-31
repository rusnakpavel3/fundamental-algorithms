#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_SUBSTRING_EMPTY:
            return "подстрока не может быть пустой";
        case STATUS_ERR_COUNT_NEGATIVE:
            return "количество файлов не может быть отрицательным";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком много данных: размер массива не помещается в size_t";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_PATH_NULL:
            return "вместо пути к файлу передан NULL";
        case STATUS_ERR_PATH_EMPTY:
            return "путь к файлу - пустая строка";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл (нет такого файла или нет доступа)";
        case STATUS_ERR_FILE_READ_FAILED:
            return "ошибка чтения файла (например, это папка)";
        case STATUS_ERR_FILE_TOO_LARGE:
            return "файл слишком большой: размер не помещается в size_t";
    }
    return "неизвестный статус";
}
