#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан нулевой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "нужно ровно 2 аргумента: файл со студентами и трассировочный файл";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл со студентами";
        case STATUS_ERR_FILE_INFO_FAILED:
            return "не удалось получить сведения о файле";
        case STATUS_ERR_SAME_FILE:
            return "трассировочный файл совпадает с файлом студентов";
        case STATUS_ERR_TRACE_OPEN_FAILED:
            return "не удалось открыть трассировочный файл";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения";
        case STATUS_ERR_WRITE_FAILED:
            return "ошибка записи в трассировочный файл";
        case STATUS_ERR_CLOSE_FAILED:
            return "ошибка закрытия трассировочного файла";
        case STATUS_ERR_NUL_BYTE:
            return "в строке нулевой байт";
        case STATUS_ERR_ALLOC_FAILED:
            return "не хватило памяти";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_FIELD_COUNT:
            return "в строке должно быть 9 полей: id имя фамилия группа и 5 оценок";
        case STATUS_ERR_BAD_ID:
            return "id - целое неотрицательное число, не больше 9 цифр";
        case STATUS_ERR_BAD_FIRST_NAME:
            return "имя должно состоять только из латинских букв";
        case STATUS_ERR_BAD_LAST_NAME:
            return "фамилия должна состоять только из латинских букв";
        case STATUS_ERR_BAD_GRADE:
            return "оценка - целое число от 2 до 5";
        case STATUS_ERR_DUPLICATE_ID:
            return "такой id уже есть";
        case STATUS_ERR_NO_STUDENTS:
            return "в файле нет ни одного студента";
        case STATUS_ERR_NOT_FOUND:
            return "студент не найден";
        case STATUS_ERR_SUM_OVERFLOW:
            return "переполнение при подсчёте суммы оценок";
        case STATUS_ERR_INPUT_EMPTY:
            return "пустой ввод";
        case STATUS_ERR_LINE_TOO_LONG:
            return "слишком длинная строка ввода";
    }
    return "неизвестная ошибка";
}
