#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "console.h"
#include "date.h"
#include "liver.h"
#include "liver_file.h"
#include "liver_list.h"
#include "status.h"

#define LINE_SIZE 256
#define MAX_POSITION_DIGITS 9
#define SEARCH_EPSILON 0.001


static void print_menu(void)
{
    printf("Команды:\n");
    printf("  1 - показать всех жителей\n");
    printf("  2 - найти жителей\n");
    printf("  3 - добавить жителя\n");
    printf("  4 - изменить жителя\n");
    printf("  5 - удалить жителя\n");
    printf("  6 - выгрузить список в файл\n");
    printf("  7 - отменить последние N/2 изменений (Undo)\n");
    printf("  8 - показать команды\n");
    printf("  0 - выход\n");
}


static void print_date(const Date *date)
{
    printf("%02d.%02d.%04d", date->day, date->month, date->year);
}


static void print_liver(const size_t number, const Liver *liver)
{
    printf("  %zu. %s %s", number, liver->surname, liver->name);
    if (liver->patronymic[0] != '\0') {
        printf(" %s", liver->patronymic);
    }
    printf(", ");
    print_date(&liver->birth);
    printf(", пол %c, доход %.2f\n", liver->gender, liver->income);
}


static void print_all(const LiverList *list)
{
    printf("Жителей: %zu (от младших к старшим)\n", list->count);
    size_t number = 1;
    for (const Node *current = list->head; current != NULL; current = current->next) {
        print_liver(number, &current->data);
        number++;
    }
}


static bool read_answer(const char *prompt, char *buffer, const size_t size)
{
    while (true) {
        printf("%s", prompt);
        fflush(stdout);
        bool end = false;
        const Status status = console_read_line(stdin, buffer, size, &end);
        if (status == STATUS_ERR_LINE_TOO_LONG) {
            printf("Ошибка: %s\n", status_message(status));
            continue;
        }
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return false;
        }
        return !end;
    }
}


static bool ask_name(const char *prompt, const bool allow_empty, const Status error, char *buffer,
                     const size_t size)
{
    while (read_answer(prompt, buffer, size)) {
        if (check_name(buffer, allow_empty) == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(error));
    }
    return false;
}


static bool ask_birth_date(const char *prompt, const Date *today, Date *out_date)
{
    char text[LINE_SIZE];
    while (read_answer(prompt, text, sizeof text)) {
        const Status status = parse_birth_date(text, today, out_date);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_gender(const char *prompt, char *out_gender)
{
    char text[LINE_SIZE];
    while (read_answer(prompt, text, sizeof text)) {
        const Status status = parse_gender(text, out_gender);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_income(const char *prompt, double *out_income)
{
    char text[LINE_SIZE];
    while (read_answer(prompt, text, sizeof text)) {
        const Status status = parse_income(text, out_income);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_position(const LiverList *list, size_t *out_position)
{
    char text[LINE_SIZE];
    if (!read_answer("Номер жителя: ", text, sizeof text)) {
        return false;
    }
    const size_t length = strlen(text);
    bool digits_only = length > 0 && length <= MAX_POSITION_DIGITS;
    for (size_t i = 0; i < length && digits_only; i++) {
        if (text[i] < '0' || text[i] > '9') {
            digits_only = false;
        }
    }
    unsigned long number = 0;
    if (digits_only) {
        char *end = NULL;
        number = strtoul(text, &end, 10);
        if (end == text || *end != '\0') {
            digits_only = false;
        }
    }
    if (!digits_only || number == 0 || number > list->count) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_BAD_NUMBER));
        return false;
    }
    *out_position = (size_t) number - 1;
    return true;
}


static bool get_today(Date *today)
{
    const Status status = date_today(today);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return false;
    }
    return true;
}


static void print_changes(const LiverList *list)
{
    printf("Изменений с последней отмены: %zu\n", list->history_count);
}


static bool ask_search_name(const char *prompt, const Status error, char *buffer, const size_t size,
                            const char **out_value)
{
    if (!read_answer(prompt, buffer, size)) {
        return false;
    }
    if (buffer[0] == '\0') {
        *out_value = NULL;
        return true;
    }
    if (check_name(buffer, false) != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(error));
        return false;
    }
    *out_value = buffer;
    return true;
}


static void command_search(const LiverList *list)
{
    if (list->count == 0) {
        printf("Список пуст\n");
        return;
    }
    char surname[LINE_SIZE];
    char name[LINE_SIZE];
    char patronymic[LINE_SIZE];
    char text[LINE_SIZE];
    SearchQuery query;
    query.surname = NULL;
    query.name = NULL;
    query.patronymic = NULL;
    query.use_birth = false;
    query.gender = '\0';
    query.use_income = false;
    query.income = 0.0;

    printf("Пустой ответ - параметр не важен\n");
    if (!ask_search_name("Фамилия: ", STATUS_ERR_BAD_SURNAME, surname, sizeof surname, &query.surname)
        || !ask_search_name("Имя: ", STATUS_ERR_BAD_NAME, name, sizeof name, &query.name)
        || !ask_search_name("Отчество: ", STATUS_ERR_BAD_PATRONYMIC, patronymic, sizeof patronymic,
                            &query.patronymic)) {
        return;
    }
    if (!read_answer("Дата рождения (dd.MM.yyyy): ", text, sizeof text)) {
        return;
    }
    if (text[0] != '\0') {
        const Status status = date_parse(text, &query.birth);
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return;
        }
        query.use_birth = true;
    }
    if (!read_answer("Пол (M/W): ", text, sizeof text)) {
        return;
    }
    if (text[0] != '\0') {
        const Status status = parse_gender(text, &query.gender);
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return;
        }
    }
    if (!read_answer("Средний доход: ", text, sizeof text)) {
        return;
    }
    if (text[0] != '\0') {
        const Status status = parse_income(text, &query.income);
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return;
        }
        query.use_income = true;
    }

    size_t *positions = (size_t *) malloc(list->count * sizeof(size_t));
    if (positions == NULL) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ALLOC_FAILED));
        return;
    }
    size_t found = 0;
    Status status = list_search(list, &query, SEARCH_EPSILON, positions, list->count, &found);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
    } else if (found == 0) {
        printf("Никого не найдено\n");
    } else {
        printf("Найдено: %zu\n", found);
        for (size_t i = 0; i < found && status == STATUS_OK; i++) {
            const Liver *liver = NULL;
            status = list_get(list, positions[i], &liver);
            if (status == STATUS_OK) {
                print_liver(positions[i] + 1, liver);
            } else {
                printf("Ошибка: %s\n", status_message(status));
            }
        }
    }
    free(positions);
}


static void command_add(LiverList *list)
{
    Date today;
    if (!get_today(&today)) {
        return;
    }
    char surname[LINE_SIZE];
    char name[LINE_SIZE];
    char patronymic[LINE_SIZE];
    LiverFields fields;
    if (!ask_name("Фамилия: ", false, STATUS_ERR_BAD_SURNAME, surname, sizeof surname)
        || !ask_name("Имя: ", false, STATUS_ERR_BAD_NAME, name, sizeof name)
        || !ask_name("Отчество (можно пусто): ", true, STATUS_ERR_BAD_PATRONYMIC, patronymic, sizeof patronymic)
        || !ask_birth_date("Дата рождения (dd.MM.yyyy): ", &today, &fields.birth)
        || !ask_gender("Пол (M/W): ", &fields.gender)
        || !ask_income("Средний доход: ", &fields.income)) {
        printf("Добавление отменено\n");
        return;
    }
    fields.surname = surname;
    fields.name = name;
    fields.patronymic = patronymic;

    Liver liver;
    Status status = liver_create(&fields, &today, &liver);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    size_t position = 0;
    status = list_add(list, &liver, &position);
    liver_free(&liver);
    if (status != STATUS_OK) {
        printf("Ошибка: %s. Житель не добавлен\n", status_message(status));
        return;
    }
    printf("Житель добавлен под номером %zu\n", position + 1);
    print_changes(list);
}


static void command_edit(LiverList *list)
{
    if (list->count == 0) {
        printf("Список пуст\n");
        return;
    }
    Date today;
    size_t position = 0;
    if (!get_today(&today) || !ask_position(list, &position)) {
        return;
    }
    const Liver *current = NULL;
    Status status = list_get(list, position, &current);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Сейчас:\n");
    print_liver(position + 1, current);
    printf("Что изменить: 1 - фамилия, 2 - имя, 3 - отчество, 4 - дата рождения, 5 - пол, 6 - доход\n");

    char choice[LINE_SIZE];
    if (!read_answer("Номер поля: ", choice, sizeof choice)) {
        return;
    }

    LiverFields fields;
    fields.surname = current->surname;
    fields.name = current->name;
    fields.patronymic = current->patronymic;
    fields.birth = current->birth;
    fields.gender = current->gender;
    fields.income = current->income;

    char text[LINE_SIZE];
    bool entered = false;
    if (strcmp(choice, "1") == 0) {
        entered = ask_name("Новая фамилия: ", false, STATUS_ERR_BAD_SURNAME, text, sizeof text);
        fields.surname = text;
    } else if (strcmp(choice, "2") == 0) {
        entered = ask_name("Новое имя: ", false, STATUS_ERR_BAD_NAME, text, sizeof text);
        fields.name = text;
    } else if (strcmp(choice, "3") == 0) {
        entered = ask_name("Новое отчество (Enter - без отчества): ", true, STATUS_ERR_BAD_PATRONYMIC, text,
                           sizeof text);
        fields.patronymic = text;
    } else if (strcmp(choice, "4") == 0) {
        entered = ask_birth_date("Новая дата рождения (dd.MM.yyyy): ", &today, &fields.birth);
    } else if (strcmp(choice, "5") == 0) {
        entered = ask_gender("Новый пол (M/W): ", &fields.gender);
    } else if (strcmp(choice, "6") == 0) {
        entered = ask_income("Новый средний доход: ", &fields.income);
    } else {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_BAD_FIELD));
        return;
    }
    if (!entered) {
        printf("Изменение отменено\n");
        return;
    }

    Liver liver;
    status = liver_create(&fields, &today, &liver);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    size_t new_position = 0;
    status = list_edit(list, position, &liver, &new_position);
    liver_free(&liver);
    if (status != STATUS_OK) {
        printf("Ошибка: %s. Житель не изменён\n", status_message(status));
        return;
    }
    printf("Житель изменён, теперь его номер %zu\n", new_position + 1);
    print_changes(list);
}


static void command_remove(LiverList *list)
{
    if (list->count == 0) {
        printf("Список пуст\n");
        return;
    }
    size_t position = 0;
    if (!ask_position(list, &position)) {
        return;
    }
    const Status status = list_remove(list, position);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Житель удалён\n");
    print_changes(list);
}


static void command_export(const LiverList *list, const char *input_path)
{
    char path[LINE_SIZE];
    if (!read_answer("Файл для выгрузки: ", path, sizeof path)) {
        return;
    }
    if (path[0] == '\0') {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_INPUT_EMPTY));
        return;
    }
    const Status status = export_livers(list, input_path, path);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Выгружено жителей: %zu в %s\n", list->count, path);
}


static void command_undo(LiverList *list)
{
    const size_t total = list->history_count;
    size_t undone = 0;
    const Status status = list_undo(list, &undone);
    if (status != STATUS_OK) {
        printf("%s (изменений с последней отмены: %zu)\n", status_message(status), total);
        return;
    }
    printf("Изменений было %zu, отменено последних %zu\n", total, undone);
    print_all(list);
}


static void run_dialog(LiverList *list, const char *input_path)
{
    char command[LINE_SIZE];
    print_menu();
    while (true) {
        printf("> ");
        fflush(stdout);

        bool end = false;
        const Status status = console_read_line(stdin, command, sizeof command, &end);
        if (status == STATUS_ERR_LINE_TOO_LONG) {
            printf("Ошибка: %s\n", status_message(status));
            continue;
        }
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return;
        }
        if (end) {
            printf("\nВвод закончился - выход\n");
            return;
        }

        if (strcmp(command, "0") == 0) {
            return;
        } else if (strcmp(command, "1") == 0) {
            print_all(list);
        } else if (strcmp(command, "2") == 0) {
            command_search(list);
        } else if (strcmp(command, "3") == 0) {
            command_add(list);
        } else if (strcmp(command, "4") == 0) {
            command_edit(list);
        } else if (strcmp(command, "5") == 0) {
            command_remove(list);
        } else if (strcmp(command, "6") == 0) {
            command_export(list, input_path);
        } else if (strcmp(command, "7") == 0) {
            command_undo(list);
        } else if (strcmp(command, "8") == 0) {
            print_menu();
        } else {
            printf("Неизвестная команда: введите число от 0 до 8\n");
        }
    }
}


static void print_load_error(const Status status, const size_t line, const char *path)
{
    switch (status) {
        case STATUS_OK:
            break;
        case STATUS_ERR_NUL_BYTE:
        case STATUS_ERR_FIELD_COUNT:
        case STATUS_ERR_BAD_SURNAME:
        case STATUS_ERR_BAD_NAME:
        case STATUS_ERR_BAD_PATRONYMIC:
        case STATUS_ERR_BAD_DATE_FORMAT:
        case STATUS_ERR_BAD_DATE:
        case STATUS_ERR_DATE_IN_FUTURE:
        case STATUS_ERR_BAD_GENDER:
        case STATUS_ERR_BAD_INCOME:
            printf("Ошибка в файле %s, строка %zu: %s\n", path, line, status_message(status));
            break;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_ARGUMENT_COUNT:
        case STATUS_ERR_FILE_OPEN_FAILED:
        case STATUS_ERR_SAME_FILE:
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
        case STATUS_ERR_READ_FAILED:
        case STATUS_ERR_WRITE_FAILED:
        case STATUS_ERR_CLOSE_FAILED:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_BAD_EPSILON:
        case STATUS_ERR_BUFFER_TOO_SMALL:
        case STATUS_ERR_TIME_UNAVAILABLE:
        case STATUS_ERR_BAD_NUMBER:
        case STATUS_ERR_BAD_FIELD:
        case STATUS_ERR_NOTHING_TO_UNDO:
        case STATUS_ERR_INPUT_EMPTY:
        case STATUS_ERR_LINE_TOO_LONG:
            printf("Ошибка: %s: %s\n", status_message(status), path);
            break;
    }
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc != 2) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ARGUMENT_COUNT));
        printf("Пример: lab3_task7 livers.txt\n");
        return 1;
    }

    Date today;
    Status status = date_today(&today);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        print_load_error(STATUS_ERR_FILE_OPEN_FAILED, 0, argv[1]);
        return 1;
    }
    LiverList list;
    list_init(&list);
    size_t line = 0;
    status = read_livers(input, &today, &list, &line);
    fclose(input);
    if (status != STATUS_OK) {
        print_load_error(status, line, argv[1]);
        list_free(&list);
        return 1;
    }

    printf("Загружено жителей: %zu\n", list.count);
    run_dialog(&list, argv[1]);
    list_free(&list);
    return 0;
}
