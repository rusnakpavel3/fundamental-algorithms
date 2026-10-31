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
            return "неизвестный флаг (допустимы только a и d)";
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
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "переполнение размера при выделении памяти";
        case STATUS_ERR_NUL_BYTE:
            return "во входном файле встретился нулевой байт";
        case STATUS_ERR_LINE_FIELD_COUNT:
            return "в строке должно быть ровно 4 поля: id имя фамилия зарплата";
        case STATUS_ERR_ID_INVALID:
            return "id должен быть целым неотрицательным числом (только цифры)";
        case STATUS_ERR_ID_OUT_OF_RANGE:
            return "id слишком велик (максимум 9223372036854775807)";
        case STATUS_ERR_NAME_INVALID:
            return "имя должно состоять только из латинских букв";
        case STATUS_ERR_SURNAME_INVALID:
            return "фамилия должна состоять только из латинских букв";
        case STATUS_ERR_SALARY_INVALID:
            return "зарплата не является корректным вещественным числом";
        case STATUS_ERR_SALARY_NEGATIVE:
            return "зарплата не может быть отрицательной";
        case STATUS_ERR_SALARY_OUT_OF_RANGE:
            return "зарплата вне диапазона типа double";
        case STATUS_ERR_SALARY_TOO_PRECISE:
            return "в записи зарплаты больше 15 значащих цифр";
    }
    return "неизвестный статус";
}
