#include "status.h"


const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "внутренняя ошибка: передан пустой указатель";
        case STATUS_ERR_ARGUMENT_COUNT:
            return "нужно указать путь к файлу и хотя бы один разделитель";
        case STATUS_ERR_SEPARATOR_INVALID:
            return "разделитель должен быть одним символом или \\t, \\s, \\\\";
        case STATUS_ERR_FILE_OPEN_FAILED:
            return "не удалось открыть файл (нет такого файла или нет доступа)";
        case STATUS_ERR_READ_FAILED:
            return "ошибка чтения";
        case STATUS_ERR_NUL_BYTE:
            return "файл содержит нулевой байт - это не текстовый файл в UTF-8";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком большой размер данных";
        case STATUS_ERR_COUNT_OVERFLOW:
            return "счётчик слов переполнен";
        case STATUS_ERR_TREE_TOO_DEEP:
            return "дерево слишком глубокое (больше 1000 уровней): похоже, слова идут по порядку";
        case STATUS_ERR_TREE_EMPTY:
            return "дерево пустое: в файле нет слов";
        case STATUS_ERR_INPUT_EMPTY:
            return "введена пустая строка";
        case STATUS_ERR_NUMBER_INVALID:
            return "нужно положительное целое число";
        case STATUS_ERR_LINE_TOO_LONG:
            return "введённая строка слишком длинная";
        case STATUS_ERR_SAME_FILE:
            return "нельзя записать дерево в исходный текстовый файл";
        case STATUS_ERR_FILE_INFO_FAILED:
            return "не удалось получить сведения о файле";
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
            return "не удалось открыть файл на запись (нет такой папки или нет доступа)";
        case STATUS_ERR_WRITE_FAILED:
            return "ошибка записи в файл";
        case STATUS_ERR_CLOSE_FAILED:
            return "ошибка при закрытии файла (данные могли не записаться)";
        case STATUS_ERR_TREE_FILE_FORMAT:
            return "строка файла дерева должна иметь вид: частота пробел слово";
        case STATUS_ERR_WORD_DUPLICATE:
            return "слово повторяется в файле дерева";
    }
    return "неизвестный статус";
}
