#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "console.h"
#include "status.h"
#include "text_reader.h"
#include "tree_file.h"
#include "word_tree.h"

#define LINE_SIZE 1024
#define MAX_SHOWN_NODES 100


static void print_menu(void)
{
    printf("Команды:\n");
    printf("  1 - сколько раз встречается слово\n");
    printf("  2 - n самых частых слов\n");
    printf("  3 - самое длинное и самое короткое слово\n");
    printf("  4 - глубина дерева\n");
    printf("  5 - показать дерево\n");
    printf("  6 - сохранить дерево в файл\n");
    printf("  7 - восстановить дерево из файла\n");
    printf("  8 - показать команды\n");
    printf("  0 - выход\n");
}


static bool ask_line(const char *prompt, char *buffer, const size_t size)
{
    printf("%s", prompt);
    fflush(stdout);

    bool end = false;
    const Status status = console_read_line(stdin, buffer, size, &end);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return false;
    }
    if (end) {
        return false;
    }
    if (buffer[0] == '\0') {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_INPUT_EMPTY));
        return false;
    }
    return true;
}


static void print_tree(const WordNode *node, const size_t level)
{
    if (node == NULL) {
        return;
    }
    print_tree(node->right, level + 1);
    for (size_t i = 0; i < level; i++) {
        printf("        ");
    }
    printf("%s (%llu)\n", node->word, node->count);
    print_tree(node->left, level + 1);
}


static void show_tree(const WordNode *root)
{
    size_t size = 0;
    tree_size(root, &size);
    if (size == 0) {
        printf("Дерево пустое\n");
    } else if (size > MAX_SHOWN_NODES) {
        printf("В дереве %zu узлов - показываю только деревья до %d узлов\n", size, MAX_SHOWN_NODES);
    } else {
        printf("Дерево повёрнуто: корень слева, правое поддерево выше, левое ниже\n");
        print_tree(root, 0);
    }
}


static void command_count(const WordNode *root)
{
    char word[LINE_SIZE];
    if (!ask_line("Слово: ", word, sizeof word)) {
        return;
    }
    unsigned long long count = 0;
    const Status status = tree_find(root, word, &count);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("\"%s\" встречается %llu раз(а)\n", word, count);
}


static void command_top(const WordNode *root)
{
    char text[LINE_SIZE];
    if (!ask_line("n: ", text, sizeof text)) {
        return;
    }
    size_t n = 0;
    Status status = parse_count(text, &n);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }

    WordList list;
    status = tree_top_words(root, n, &list);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    if (n > list.total) {
        printf("Разных слов всего %zu, вывожу все\n", list.total);
    }
    for (size_t i = 0; i < list.count; i++) {
        printf("%3zu. %s - %llu\n", i + 1, list.items[i]->word, list.items[i]->count);
    }
    word_list_free(&list);
}


static void command_extremes(const WordNode *root)
{
    const WordNode *longest = NULL;
    const WordNode *shortest = NULL;
    const Status status = tree_extremes(root, &longest, &shortest);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Самое длинное слово: %s (%zu симв.)\n", longest->word, longest->length);
    printf("Самое короткое слово: %s (%zu симв.)\n", shortest->word, shortest->length);
}


static void command_depth(const WordNode *root)
{
    size_t depth = 0;
    tree_depth(root, &depth);
    printf("Глубина дерева (число уровней): %zu\n", depth);
}


static void command_save(const WordNode *root, const char *text_path)
{
    char path[LINE_SIZE];
    if (!ask_line("Файл для сохранения: ", path, sizeof path)) {
        return;
    }
    const Status status = tree_save(root, path, text_path);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    size_t size = 0;
    tree_size(root, &size);
    printf("Дерево сохранено в %s: %zu строк (прямой обход)\n", path, size);
}


static void command_restore(const WordNode *root)
{
    char path[LINE_SIZE];
    if (!ask_line("Файл с деревом: ", path, sizeof path)) {
        return;
    }
    WordNode *restored = NULL;
    size_t line = 0;
    const Status status = tree_load(path, &restored, &line);
    switch (status) {
        case STATUS_OK:
            break;
        case STATUS_ERR_TREE_FILE_FORMAT:
        case STATUS_ERR_WORD_DUPLICATE:
        case STATUS_ERR_COUNT_OVERFLOW:
        case STATUS_ERR_TREE_TOO_DEEP:
            printf("Ошибка в файле дерева (строка %zu): %s\n", line, status_message(status));
            return;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_ARGUMENT_COUNT:
        case STATUS_ERR_SEPARATOR_INVALID:
        case STATUS_ERR_FILE_OPEN_FAILED:
        case STATUS_ERR_READ_FAILED:
        case STATUS_ERR_NUL_BYTE:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_TREE_EMPTY:
        case STATUS_ERR_INPUT_EMPTY:
        case STATUS_ERR_NUMBER_INVALID:
        case STATUS_ERR_LINE_TOO_LONG:
        case STATUS_ERR_SAME_FILE:
        case STATUS_ERR_FILE_INFO_FAILED:
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
        case STATUS_ERR_WRITE_FAILED:
        case STATUS_ERR_CLOSE_FAILED:
            printf("Ошибка: %s\n", status_message(status));
            return;
    }

    size_t size = 0;
    size_t depth = 0;
    bool equal = false;
    tree_size(restored, &size);
    tree_depth(restored, &depth);
    tree_equal(root, restored, &equal);
    printf("Восстановлено узлов: %zu, глубина: %zu\n", size, depth);
    show_tree(restored);
    printf("Структура совпадает с текущим деревом: %s\n", equal ? "да" : "нет");
    tree_free(restored);
    printf("Восстановленное дерево удалено\n");
}


static void run_dialog(const WordNode *root, const char *text_path)
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
            command_count(root);
        } else if (strcmp(command, "2") == 0) {
            command_top(root);
        } else if (strcmp(command, "3") == 0) {
            command_extremes(root);
        } else if (strcmp(command, "4") == 0) {
            command_depth(root);
        } else if (strcmp(command, "5") == 0) {
            show_tree(root);
        } else if (strcmp(command, "6") == 0) {
            command_save(root, text_path);
        } else if (strcmp(command, "7") == 0) {
            command_restore(root);
        } else if (strcmp(command, "8") == 0) {
            print_menu();
        } else {
            printf("Неизвестная команда: введите число от 0 до 8\n");
        }
    }
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 3) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ARGUMENT_COUNT));
        printf("Пример: lab3_task9 text.txt \\s , .\n");
        return 1;
    }

    bool separator[CHAR_VALUES] = {false};
    separator['\n'] = true;
    separator['\r'] = true;
    for (int i = 2; i < argc; i++) {
        unsigned char symbol = 0;
        const Status status = parse_separator(argv[i], &symbol);
        if (status != STATUS_OK) {
            printf("Ошибка в аргументе %d (\"%s\"): %s\n", i, argv[i], status_message(status));
            return 1;
        }
        separator[symbol] = true;
    }

    WordNode *root = NULL;
    unsigned long long total = 0;
    const Status status = read_words(argv[1], separator, &root, &total);
    if (status != STATUS_OK) {
        printf("Ошибка: %s: %s\n", status_message(status), argv[1]);
        return 1;
    }

    size_t distinct = 0;
    tree_size(root, &distinct);
    printf("Файл %s: слов %llu, разных %zu\n", argv[1], total, distinct);
    run_dialog(root, argv[1]);

    tree_free(root);
    return 0;
}
