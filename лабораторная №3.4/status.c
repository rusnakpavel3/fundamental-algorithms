#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан нулевой указатель";
        case STATUS_ERR_ALLOC_FAILED:
            return "не хватило памяти";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_STRING_TOO_LONG:
            return "строка слишком длинная";
        case STATUS_ERR_EMPTY_FIELD:
            return "поле не может быть пустым";
        case STATUS_ERR_BAD_NUMBER:
            return "нужно натуральное число (1, 2, 3, ...), не больше 9 цифр";
        case STATUS_ERR_BAD_INDEX:
            return "индекс - ровно 6 цифр";
        case STATUS_ERR_BAD_ID:
            return "почтовый идентификатор - ровно 14 цифр";
        case STATUS_ERR_BAD_WEIGHT:
            return "вес - неотрицательное число вида 1.25, не больше 15 символов";
        case STATUS_ERR_BAD_TIME_FORMAT:
            return "время должно быть в формате dd:MM:yyyy hh:mm:ss";
        case STATUS_ERR_BAD_DATE:
            return "такой даты или времени не существует";
        case STATUS_ERR_DEADLINE_BEFORE_CREATION:
            return "время вручения раньше времени создания";
        case STATUS_ERR_DUPLICATE_ID:
            return "письмо с таким идентификатором уже есть";
        case STATUS_ERR_NOT_FOUND:
            return "письма с таким идентификатором нет";
        case STATUS_ERR_ALREADY_DELIVERED:
            return "письмо уже вручено";
        case STATUS_ERR_TIME_UNAVAILABLE:
            return "не удалось получить системное время";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения";
        case STATUS_ERR_LINE_TOO_LONG:
            return "слишком длинная строка ввода";
        case STATUS_ERR_INPUT_EMPTY:
            return "пустой ввод";
    }
    return "неизвестная ошибка";
}
