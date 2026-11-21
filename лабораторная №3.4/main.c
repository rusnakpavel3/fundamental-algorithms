#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "console.h"
#include "datetime.h"
#include "post.h"
#include "status.h"

#define LINE_SIZE 256

typedef struct {
    char city[LINE_SIZE];
    char street[LINE_SIZE];
    char building[LINE_SIZE];
    char index[LINE_SIZE];
} AddressInput;

typedef struct {
    AddressInput to;
    char id[LINE_SIZE];
    char created[LINE_SIZE];
    char deadline[LINE_SIZE];
} MailInput;


static void print_menu(void)
{
    printf("Команды:\n");
    printf("  1 - добавить письмо\n");
    printf("  2 - удалить письмо по идентификатору\n");
    printf("  3 - найти письмо по идентификатору\n");
    printf("  4 - отметить письмо врученным\n");
    printf("  5 - все доставленные письма\n");
    printf("  6 - все письма с истёкшим сроком доставки\n");
    printf("  7 - показать все письма\n");
    printf("  8 - показать команды\n");
    printf("  0 - выход\n");
}


static bool ask_text(const char *prompt, char *buffer, const size_t size, const bool allow_empty)
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
        if (end) {
            return false;
        }
        if (!allow_empty && buffer[0] == '\0') {
            printf("Ошибка: %s\n", status_message(STATUS_ERR_EMPTY_FIELD));
            continue;
        }
        return true;
    }
}


static bool ask_natural(const char *prompt, unsigned int *out_value)
{
    char text[LINE_SIZE];
    while (ask_text(prompt, text, sizeof text, false)) {
        const Status status = parse_natural(text, out_value);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_index(const char *prompt, char *buffer, const size_t size)
{
    while (ask_text(prompt, buffer, size, false)) {
        const Status status = check_postal_index(buffer);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_weight(double *out_weight)
{
    char text[LINE_SIZE];
    while (ask_text("Вес, кг: ", text, sizeof text, false)) {
        const Status status = parse_weight(text, out_weight);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_datetime(const char *prompt, char *buffer, const size_t size)
{
    while (ask_text(prompt, buffer, size, false)) {
        DateTime parsed;
        const Status status = datetime_parse(buffer, &parsed);
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_new_id(const Post *post, char *buffer, const size_t size)
{
    while (ask_text("Почтовый идентификатор (14 цифр): ", buffer, size, false)) {
        Status status = check_mail_id(buffer);
        size_t index = 0;
        if (status == STATUS_OK && post_find(post, buffer, &index) == STATUS_OK) {
            status = STATUS_ERR_DUPLICATE_ID;
        }
        if (status == STATUS_OK) {
            return true;
        }
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


static bool ask_address(AddressInput *input, AddressFields *out_fields)
{
    if (!ask_text("Город: ", input->city, sizeof input->city, false)) {
        return false;
    }
    if (!ask_text("Улица: ", input->street, sizeof input->street, false)) {
        return false;
    }
    if (!ask_natural("Номер дома: ", &out_fields->house)) {
        return false;
    }
    if (!ask_text("Корпус (можно пусто): ", input->building, sizeof input->building, true)) {
        return false;
    }
    if (!ask_natural("Номер квартиры: ", &out_fields->flat)) {
        return false;
    }
    if (!ask_index("Индекс (6 цифр): ", input->index, sizeof input->index)) {
        return false;
    }
    out_fields->city = input->city;
    out_fields->street = input->street;
    out_fields->building = input->building;
    out_fields->index = input->index;
    return true;
}


static void print_address(const Address *address)
{
    printf("%s, %s, ул. %s, д. %u", address->index.data, address->city.data, address->street.data, address->house);
    if (address->building.length > 0) {
        printf(", корп. %s", address->building.data);
    }
    printf(", кв. %u", address->flat);
}


static const char *mail_state(const Mail *mail, const DateTime *now)
{
    if (mail->delivered) {
        return "вручено";
    }
    if (mail_is_expired(mail, now)) {
        return "срок доставки истёк";
    }
    return "в пути";
}


static void print_mail_full(const Mail *mail, const DateTime *now)
{
    printf("Письмо %s\n", mail->id.data);
    printf("  получатель: ");
    print_address(&mail->to);
    printf("\n");
    printf("  вес: %.3f кг\n", mail->weight);
    printf("  создано: %s\n", mail->created_text.data);
    printf("  срок вручения: %s\n", mail->deadline_text.data);
    printf("  состояние: %s\n", mail_state(mail, now));
}


static void print_mail_short(const Mail *mail, const DateTime *now)
{
    printf("  %s  индекс %s  создано %s  срок %s  %s\n", mail->id.data, mail->to.index.data,
           mail->created_text.data, mail->deadline_text.data, mail_state(mail, now));
}


static bool get_now(DateTime *now)
{
    const Status status = datetime_now(now);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return false;
    }
    return true;
}


static void command_add(Post *post)
{
    MailInput input;
    MailFields fields;
    printf("Адрес получателя\n");
    if (!ask_address(&input.to, &fields.to)) {
        printf("Добавление отменено\n");
        return;
    }
    if (!ask_weight(&fields.weight)
        || !ask_new_id(post, input.id, sizeof input.id)
        || !ask_datetime("Время создания (dd:MM:yyyy hh:mm:ss): ", input.created, sizeof input.created)
        || !ask_datetime("Время вручения (dd:MM:yyyy hh:mm:ss): ", input.deadline, sizeof input.deadline)) {
        printf("Добавление отменено\n");
        return;
    }
    fields.id = input.id;
    fields.created = input.created;
    fields.deadline = input.deadline;

    const Status status = post_add(post, &fields);
    if (status != STATUS_OK) {
        printf("Ошибка: %s. Письмо не добавлено\n", status_message(status));
        return;
    }
    printf("Письмо добавлено. Писем в отделении: %zu\n", post->count);
}


static void command_remove(Post *post)
{
    char id[LINE_SIZE];
    if (!ask_text("Идентификатор: ", id, sizeof id, false)) {
        return;
    }
    const Status status = post_remove(post, id);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Письмо удалено. Писем в отделении: %zu\n", post->count);
}


static void command_find(const Post *post)
{
    char id[LINE_SIZE];
    if (!ask_text("Идентификатор: ", id, sizeof id, false)) {
        return;
    }
    DateTime now;
    if (!get_now(&now)) {
        return;
    }
    size_t index = 0;
    const Status status = post_find(post, id, &index);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    print_mail_full(&post->mails[index], &now);
}


static void command_deliver(Post *post)
{
    char id[LINE_SIZE];
    if (!ask_text("Идентификатор: ", id, sizeof id, false)) {
        return;
    }
    const Status status = post_deliver(post, id);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Письмо отмечено врученным\n");
}


static void command_select(const Post *post, const Selection kind, const char *title)
{
    DateTime now;
    if (!get_now(&now)) {
        return;
    }
    Mail *list = NULL;
    size_t count = 0;
    const Status status = post_select(post, kind, &now, &list, &count);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("%s: %zu (от старых к новым)\n", title, count);
    for (size_t i = 0; i < count; i++) {
        print_mail_short(&list[i], &now);
    }
    free(list);
}


static void command_show_all(const Post *post)
{
    DateTime now;
    if (!get_now(&now)) {
        return;
    }
    printf("Отделение: ");
    print_address(post->office);
    printf("\nПисем: %zu (по индексу получателя, затем по идентификатору)\n", post->count);
    for (size_t i = 0; i < post->count; i++) {
        print_mail_short(&post->mails[i], &now);
    }
}


static void run_dialog(Post *post)
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
            command_add(post);
        } else if (strcmp(command, "2") == 0) {
            command_remove(post);
        } else if (strcmp(command, "3") == 0) {
            command_find(post);
        } else if (strcmp(command, "4") == 0) {
            command_deliver(post);
        } else if (strcmp(command, "5") == 0) {
            command_select(post, SELECT_DELIVERED, "Доставленные письма");
        } else if (strcmp(command, "6") == 0) {
            command_select(post, SELECT_EXPIRED, "Письма с истёкшим сроком доставки");
        } else if (strcmp(command, "7") == 0) {
            command_show_all(post);
        } else if (strcmp(command, "8") == 0) {
            print_menu();
        } else {
            printf("Неизвестная команда: введите число от 0 до 8\n");
        }
    }
}


int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    printf("Адрес почтового отделения\n");
    AddressInput input;
    AddressFields fields;
    if (!ask_address(&input, &fields)) {
        printf("\nАдрес отделения не введён - выход\n");
        return 1;
    }

    Post post;
    const Status status = post_create(&fields, &post);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return 1;
    }

    run_dialog(&post);
    post_free(&post);
    return 0;
}
