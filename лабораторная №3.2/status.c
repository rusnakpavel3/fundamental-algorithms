#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_HALLS_INVALID:
            return "количество залов должно быть положительным";
        case STATUS_ERR_ID_INVALID:
            return "идентификатор заявки должен быть положительным";
        case STATUS_ERR_TIME_INVALID:
            return "время вне суток (допустимо от 0 до 1439 минут)";
        case STATUS_ERR_TIME_ORDER:
            return "время начала должно быть строго меньше времени окончания";
        case STATUS_ERR_DUPLICATE_ID:
            return "идентификаторы заявок повторяются";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком много заявок: размер массива не помещается в память";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_INPUT_EMPTY:
            return "нет входных данных";
        case STATUS_ERR_HEADER_FORMAT:
            return "в первой строке должно быть два числа: n и k";
        case STATUS_ERR_LINE_FORMAT:
            return "в строке заявки должно быть три поля: id start_time finish_time";
        case STATUS_ERR_LINE_TOO_LONG:
            return "строка слишком длинная (больше 127 символов)";
        case STATUS_ERR_NUL_CHARACTER:
            return "в строке есть нулевой байт";
        case STATUS_ERR_NUMBER_INVALID:
            return "число должно состоять только из цифр (без знака)";
        case STATUS_ERR_NUMBER_NOT_POSITIVE:
            return "число должно быть положительным";
        case STATUS_ERR_NUMBER_TOO_BIG:
            return "число слишком большое";
        case STATUS_ERR_TIME_FORMAT:
            return "время должно быть в формате HH:MM (часы 00-23, минуты 00-59)";
        case STATUS_ERR_NOT_ENOUGH_LINES:
            return "заявок меньше, чем указано в первой строке";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения стандартного ввода";
    }
    return "неизвестный статус";
}
