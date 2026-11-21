#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "нужно указать ровно один аргумент - путь к файлу";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл (нет такого файла или нет доступа)";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения файла";
        case STATUS_ERR_FILE_TOO_LARGE:
            return "файл слишком большой: счётчик байтов переполнен";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "размер закодированных данных не помещается в счётчик";
    }
    return "неизвестный статус";
}
