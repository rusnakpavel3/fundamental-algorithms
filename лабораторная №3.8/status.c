#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан нулевой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "нужен ровно 1 аргумент: файл с командами";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл с командами";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения файла";
        case STATUS_ERR_NUL_BYTE:
            return "в файле нулевой байт";
        case STATUS_ERR_ALLOC_FAILED:
            return "не хватило памяти";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_NESTED_COMMENT:
            return "вложенный комментарий [ ... [ ... ] ... ] запрещён";
        case STATUS_ERR_UNEXPECTED_BRACKET:
            return "закрывающая ] без открывающей [";
        case STATUS_ERR_UNCLOSED_COMMENT:
            return "комментарий [ не закрыт до конца файла";
        case STATUS_ERR_MISSING_SEMICOLON:
            return "команда не закончена: нет ;";
        case STATUS_ERR_UNKNOWN_COMMAND:
            return "неизвестная или пустая команда (Add, Sub, Mult, Div, Mod, Eval, Diff, Cmps)";
        case STATUS_ERR_EXPECTED_OPEN_PAREN:
            return "после имени команды нужна (";
        case STATUS_ERR_EXPECTED_CLOSE_PAREN:
            return "нет закрывающей )";
        case STATUS_ERR_EXTRA_TEXT:
            return "после ) до ; лишний текст";
        case STATUS_ERR_WRONG_ARGUMENT_COUNT:
            return "неверное число параметров команды";
        case STATUS_ERR_EMPTY_ARGUMENT:
            return "пустой параметр";
        case STATUS_ERR_BAD_POLYNOMIAL:
            return "неверная запись многочлена (пример: 2x^2-x+1)";
        case STATUS_ERR_BAD_VARIABLE:
            return "переменная многочлена - только строчная x";
        case STATUS_ERR_DEGREE_TOO_LARGE:
            return "степень больше 10000";
        case STATUS_ERR_NUMBER_TOO_LONG:
            return "в числе больше 18 цифр";
        case STATUS_ERR_BAD_NUMBER:
            return "точка в Eval - целое число";
        case STATUS_ERR_COEFFICIENT_OVERFLOW:
            return "переполнение коэффициента";
        case STATUS_ERR_VALUE_OVERFLOW:
            return "переполнение при вычислении значения";
        case STATUS_ERR_DIVISION_BY_ZERO:
            return "деление на нулевой многочлен";
    }
    return "неизвестная ошибка";
}
