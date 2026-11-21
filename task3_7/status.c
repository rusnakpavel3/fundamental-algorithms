#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан нулевой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "нужен ровно 1 аргумент: файл с жителями";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл с жителями";
        case STATUS_ERR_SAME_FILE:
            return "нельзя выгружать во входной файл";
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
            return "не удалось открыть файл для выгрузки";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения";
        case STATUS_ERR_WRITE_FAILED:
            return "ошибка записи в файл";
        case STATUS_ERR_CLOSE_FAILED:
            return "ошибка закрытия файла";
        case STATUS_ERR_NUL_BYTE:
            return "в строке нулевой байт";
        case STATUS_ERR_ALLOC_FAILED:
            return "не хватило памяти";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_FIELD_COUNT:
            return "в строке должно быть 5 полей (без отчества) или 6 (с отчеством)";
        case STATUS_ERR_BAD_SURNAME:
            return "фамилия - непустая, только латинские буквы";
        case STATUS_ERR_BAD_NAME:
            return "имя - непустое, только латинские буквы";
        case STATUS_ERR_BAD_PATRONYMIC:
            return "отчество - только латинские буквы";
        case STATUS_ERR_BAD_DATE_FORMAT:
            return "дата должна быть в формате dd.MM.yyyy";
        case STATUS_ERR_BAD_DATE:
            return "такой даты не существует (или год раньше 1900)";
        case STATUS_ERR_DATE_IN_FUTURE:
            return "дата рождения в будущем";
        case STATUS_ERR_BAD_GENDER:
            return "пол - M или W";
        case STATUS_ERR_BAD_INCOME:
            return "доход - число вида 52000 или 52000.50 (до 12 цифр, до 2 знаков после точки)";
        case STATUS_ERR_BAD_EPSILON:
            return "точность сравнения eps должна быть положительным конечным числом";
        case STATUS_ERR_BUFFER_TOO_SMALL:
            return "массив для результата меньше числа жителей";
        case STATUS_ERR_TIME_UNAVAILABLE:
            return "не удалось получить системное время";
        case STATUS_ERR_BAD_NUMBER:
            return "нет жителя с таким номером";
        case STATUS_ERR_BAD_FIELD:
            return "номер поля - число от 1 до 6";
        case STATUS_ERR_NOTHING_TO_UNDO:
            return "отменить нельзя: лимит N/2 исчерпан или изменений нет";
        case STATUS_ERR_INPUT_EMPTY:
            return "пустой ввод";
        case STATUS_ERR_LINE_TOO_LONG:
            return "слишком длинная строка ввода";
    }
    return "неизвестная ошибка";
}
