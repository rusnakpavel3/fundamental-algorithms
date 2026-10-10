#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_NUMBER_INVALID:
            return "недопустимая запись числа (разрешены 0-9, A-Z, a-z и знак '-' в начале)";
        case STATUS_ERR_NUMBER_TOO_BIG:
            return "значение не помещается в unsigned long long (больше 18446744073709551615)";
        case STATUS_ERR_PATH_EMPTY:
            return "путь к файлу - пустая строка";
        case STATUS_ERR_PATH_NO_FILENAME:
            return "в пути нет имени файла (путь указывает на папку)";
        case STATUS_ERR_SAME_FILE:
            return "входной и выходной файлы совпадают";
        case STATUS_ERR_FILE_INFO_FAILED:
            return "не удалось получить сведения о файле, чтобы сравнить входной и выходной файлы";
        case STATUS_ERR_INPUT_OPEN_FAILED:
            return "не удалось открыть входной файл (нет такого файла или нет доступа)";
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
            return "не удалось открыть выходной файл на запись (нет такой папки или нет доступа)";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения входного файла";
        case STATUS_ERR_WRITE_FAILED:
            return "ошибка записи в выходной файл";
        case STATUS_ERR_CLOSE_FAILED:
            return "ошибка при закрытии выходного файла (данные не записаны)";
    }
    return "неизвестный статус";
}
