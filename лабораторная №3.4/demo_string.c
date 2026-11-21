#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "my_string.h"
#include "status.h"


static void print_string(const char *label, const String *string)
{
    printf("%s\"%s\" (длина %d)\n", label, string->data, string->length);
}


static void print_compare(const char *name, const String *first, const String *second, const int result)
{
    const char *sign = "=";
    if (result < 0) {
        sign = "<";
    } else if (result > 0) {
        sign = ">";
    }
    printf("  %s: \"%s\" %s \"%s\"\n", name, first->data, sign, second->data);
}


static void demo_compare(const char *first_text, const char *second_text)
{
    String first;
    String second;
    Status status = string_create(first_text, &first);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    status = string_create(second_text, &second);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        string_clear(&first);
        return;
    }

    int order = 0;
    int lex = 0;
    string_compare_order(&first, &second, &order);
    string_compare_lex(&first, &second, &lex);
    print_compare("порядок (длина, затем алфавит)", &first, &second, order);
    print_compare("эквивалентность (алфавит)     ", &first, &second, lex);
    if (lex == 0) {
        printf("  строки эквивалентны\n");
    }

    string_clear(&first);
    string_clear(&second);
}


static void demo_create_and_clear(void)
{
    printf("1. Создание и очистка\n");
    String hello;
    String empty;
    if (string_create("hello", &hello) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        return;
    }
    if (string_create("", &empty) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        string_clear(&hello);
        return;
    }
    print_string("  создана: ", &hello);
    print_string("  создана: ", &empty);

    String broken;
    const Status status = string_create(NULL, &broken);
    printf("  создание из NULL: %s\n", status_message(status));

    string_clear(&hello);
    if (hello.data == NULL) {
        printf("  после очистки: data = NULL, длина %d\n", hello.length);
    }
    string_clear(&empty);
}


static void demo_copy(void)
{
    printf("3. Копирование в существующую строку\n");
    String destination;
    String source;
    if (string_create("xyz", &destination) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        return;
    }
    if (string_create("hello", &source) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        string_clear(&destination);
        return;
    }
    print_string("  было:  ", &destination);
    const Status status = string_copy(&destination, &source);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
    } else {
        print_string("  стало: ", &destination);
    }
    string_clear(&destination);
    string_clear(&source);
}


static void demo_copy_new(void)
{
    printf("4. Копирование в новую строку в динамической памяти\n");
    String original;
    if (string_create("original", &original) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        return;
    }
    String *copy = NULL;
    const Status status = string_copy_new(&original, &copy);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        string_clear(&original);
        return;
    }
    original.data[0] = 'O';
    print_string("  оригинал (изменили первую букву): ", &original);
    print_string("  копия (не изменилась):            ", copy);

    string_clear(copy);
    free(copy);
    string_clear(&original);
}


static void demo_concat(void)
{
    printf("5. Конкатенация\n");
    String first;
    String second;
    if (string_create("ab", &first) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        return;
    }
    if (string_create("cd", &second) != STATUS_OK) {
        printf("Ошибка: не удалось создать строку\n");
        string_clear(&first);
        return;
    }
    Status status = string_concat(&first, &second);
    if (status == STATUS_OK) {
        print_string("  \"ab\" + \"cd\" = ", &first);
    }
    if (status == STATUS_OK) {
        status = string_concat(&first, &first);
    }
    if (status == STATUS_OK) {
        print_string("  та же строка + она же = ", &first);
    }
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
    }
    string_clear(&first);
    string_clear(&second);
}


int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    demo_create_and_clear();

    printf("2. Сравнения\n");
    demo_compare("b", "ab");
    demo_compare("abd", "abc");
    demo_compare("abc", "abc");
    demo_compare("", "a");

    demo_copy();
    demo_copy_new();
    demo_concat();
    return 0;
}
