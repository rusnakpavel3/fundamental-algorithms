#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "bracket_tree.h"
#include "line_reader.h"
#include "same_file.h"
#include "status.h"
#include "tree_print.h"


static Status process_line(FILE *output, const char *line, const size_t number, size_t *out_position)
{
    Node *root = NULL;
    Status status = tree_parse(line, &root, out_position);
    if (status != STATUS_OK) {
        return status;
    }

    if (fprintf(output, "Строка %zu: %s\n", number, line) < 0) {
        status = STATUS_ERR_WRITE_FAILED;
    }
    if (status == STATUS_OK) {
        status = tree_write(output, root);
    }
    if (status == STATUS_OK && fputs("\n", output) == EOF) {
        status = STATUS_ERR_WRITE_FAILED;
    }

    tree_free(root);
    return status;
}


static Status process_lines(FILE *input, FILE *output, size_t *out_line_number, size_t *out_position,
                            size_t *out_trees)
{
    size_t capacity = 64;
    char *line = (char *) malloc(capacity);
    if (line == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    Status status = STATUS_OK;
    bool has_line = true;
    size_t number = 0;
    while (status == STATUS_OK && has_line) {
        number++;
        *out_line_number = number;
        status = read_line(input, &line, &capacity, &has_line);
        if (status == STATUS_OK && has_line) {
            status = process_line(output, line, number, out_position);
            if (status == STATUS_OK) {
                (*out_trees)++;
            }
        }
    }

    if (status == STATUS_OK && *out_trees == 0) {
        status = STATUS_ERR_INPUT_EMPTY;
    }
    free(line);
    return status;
}


static Status run(const char *input_path, const char *output_path, size_t *out_line_number,
                  size_t *out_position, size_t *out_trees)
{
    FILE *input = fopen(input_path, "rb");
    if (input == NULL) {
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }

    bool same = false;
    Status status = file_matches_path(input, output_path, &same);
    if (status == STATUS_OK && same) {
        status = STATUS_ERR_SAME_FILE;
    }

    FILE *output = NULL;
    if (status == STATUS_OK) {
        output = fopen(output_path, "wb");
        if (output == NULL) {
            status = STATUS_ERR_OUTPUT_OPEN_FAILED;
        }
    }

    if (status == STATUS_OK) {
        status = process_lines(input, output, out_line_number, out_position, out_trees);
    }

    if (output != NULL && fclose(output) == EOF && status == STATUS_OK) {
        status = STATUS_ERR_CLOSE_FAILED;
    }
    fclose(input);
    return status;
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 3) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ARGUMENT_COUNT));
        printf("Пример: lab3_task10 input.txt output.txt\n");
        return 1;
    }

    size_t line_number = 0;
    size_t position = 0;
    size_t trees = 0;
    const Status status = run(argv[1], argv[2], &line_number, &position, &trees);

    switch (status) {
        case STATUS_OK:
            printf("Готово: построено деревьев: %zu, результат записан в %s\n", trees, argv[2]);
            return 0;
        case STATUS_ERR_EXPECTED_NAME:
        case STATUS_ERR_EXPECTED_SEPARATOR:
        case STATUS_ERR_TRAILING_TEXT:
        case STATUS_ERR_TOO_DEEP:
            printf("Ошибка в строке %zu, позиция %zu: %s\n", line_number, position, status_message(status));
            break;
        case STATUS_ERR_EMPTY_LINE:
        case STATUS_ERR_NUL_BYTE:
        case STATUS_ERR_READ_FAILED:
            printf("Ошибка в строке %zu: %s\n", line_number, status_message(status));
            break;
        case STATUS_ERR_INPUT_OPEN_FAILED:
            printf("Ошибка: %s: %s\n", status_message(status), argv[1]);
            break;
        case STATUS_ERR_OUTPUT_OPEN_FAILED:
            printf("Ошибка: %s: %s\n", status_message(status), argv[2]);
            break;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_ARGUMENT_COUNT:
        case STATUS_ERR_SAME_FILE:
        case STATUS_ERR_FILE_INFO_FAILED:
        case STATUS_ERR_WRITE_FAILED:
        case STATUS_ERR_CLOSE_FAILED:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_INPUT_EMPTY:
            printf("Ошибка: %s\n", status_message(status));
            break;
    }

    if (trees > 0) {
        printf("Обработка остановлена. Деревья строк до ошибки (%zu шт.) записаны в %s\n", trees, argv[2]);
    }
    return 1;
}
