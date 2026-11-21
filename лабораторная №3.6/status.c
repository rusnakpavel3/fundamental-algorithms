#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан нулевой указатель";
        case STATUS_ERR_NO_FILES:
            return "нужен хотя бы 1 аргумент: файлы остановок";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения";
        case STATUS_ERR_NUL_BYTE:
            return "в строке нулевой байт";
        case STATUS_ERR_ALLOC_FAILED:
            return "не хватило памяти";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_NO_COORDINATES:
            return "в файле нет строки с координатами остановки";
        case STATUS_ERR_BAD_COORDINATES:
            return "координаты - два числа вида -12.5 (до 6 цифр до точки и до 6 после)";
        case STATUS_ERR_FIELD_COUNT:
            return "в записи должно быть 6 полей: номер, дата и время прибытия, дата и время отправления, маркер";
        case STATUS_ERR_BAD_DATETIME_FORMAT:
            return "дата и время должны быть в формате dd.MM.yyyy hh:mm:ss";
        case STATUS_ERR_BAD_DATETIME:
            return "такой даты или времени не существует (или год раньше 1900)";
        case STATUS_ERR_DEPARTURE_BEFORE_ARRIVAL:
            return "время отправления раньше времени прибытия";
        case STATUS_ERR_BAD_MARK:
            return "маркер - start, middle или end";
        case STATUS_ERR_ROUTE_NOT_STARTED:
            return "остановка middle или end, но рейс не начат (нет start)";
        case STATUS_ERR_ROUTE_NOT_FINISHED:
            return "рейс не закончен: нет end до следующего start или до конца данных";
        case STATUS_ERR_BAD_POINT:
            return "неверный номер остановочного пункта";
        case STATUS_ERR_BAD_EPSILON:
            return "точность сравнения eps должна быть положительным конечным числом";
        case STATUS_ERR_NO_DATA:
            return "нет данных: ни одного рейса";
        case STATUS_ERR_BUFFER_TOO_SMALL:
            return "массив для результата слишком мал";
        case STATUS_ERR_LINE_TOO_LONG:
            return "слишком длинная строка ввода";
    }
    return "неизвестная ошибка";
}
