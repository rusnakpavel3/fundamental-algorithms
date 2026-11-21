#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "calculator.h"
#include "polynomial.h"
#include "script.h"
#include "status.h"


static void print_polynomial(const Polynomial *poly)
{
    if (poly->head == NULL) {
        printf("0");
        return;
    }
    for (const Term *term = poly->head; term != NULL; term = term->next) {
        long long magnitude = term->coef;
        if (term->coef < 0) {
            magnitude = -term->coef;
            printf("-");
        } else if (term != poly->head) {
            printf("+");
        }
        if (magnitude != 1 || term->degree == 0) {
            printf("%lld", magnitude);
        }
        if (term->degree >= 1) {
            printf("x");
        }
        if (term->degree >= 2) {
            printf("^%d", term->degree);
        }
    }
}


static void print_outcome(const size_t line, const CommandKind kind, const Outcome *outcome)
{
    printf("строка %zu: %s(", line, command_name(kind));
    print_polynomial(&outcome->left);
    if (outcome->has_right) {
        printf(", ");
        print_polynomial(&outcome->right);
    }
    if (outcome->has_point) {
        printf(", %lld", outcome->point);
    }
    printf(") = ");
    if (outcome->is_value) {
        printf("%lld", outcome->value);
    } else {
        print_polynomial(&outcome->result);
    }
    printf("\n");
}


static bool is_fatal(const Status status)
{
    return status == STATUS_ERR_ALLOC_FAILED || status == STATUS_ERR_SIZE_OVERFLOW
           || status == STATUS_ERR_NULL_ARGUMENT;
}


static Status execute_one(char *text, const size_t line, Polynomial *accumulator)
{
    Command command;
    Status status = parse_command(text, &command);
    if (status != STATUS_OK) {
        return status;
    }
    Outcome outcome;
    status = run_command(&command, accumulator, &outcome);
    if (status != STATUS_OK) {
        return status;
    }
    print_outcome(line, command.kind, &outcome);
    outcome_free(&outcome);
    return STATUS_OK;
}


static bool run_script(char *text, size_t *out_done, size_t *out_failed)
{
    Cursor cursor;
    cursor_init(&cursor, text);
    Polynomial accumulator;
    poly_init(&accumulator);
    bool ok = true;
    size_t done = 0;
    size_t failed = 0;
    while (ok) {
        char *command = NULL;
        size_t line = 0;
        bool found = false;
        Status status = next_command(&cursor, &command, &line, &found);
        if (status == STATUS_OK && !found) {
            break;
        }
        if (status == STATUS_OK) {
            status = execute_one(command, line, &accumulator);
        }
        if (status == STATUS_OK) {
            done++;
        } else {
            printf("строка %zu: ошибка: %s\n", line, status_message(status));
            failed++;
            if (is_fatal(status) || status == STATUS_ERR_MISSING_SEMICOLON) {
                ok = !is_fatal(status);
                break;
            }
        }
    }
    printf("Сумматор в конце: ");
    print_polynomial(&accumulator);
    printf("\n");
    poly_free(&accumulator);
    *out_done = done;
    *out_failed = failed;
    return ok;
}


static void print_load_error(const Status status, const size_t line, const char *path)
{
    switch (status) {
        case STATUS_OK:
            break;
        case STATUS_ERR_NESTED_COMMENT:
        case STATUS_ERR_UNEXPECTED_BRACKET:
        case STATUS_ERR_UNCLOSED_COMMENT:
            printf("Ошибка в файле %s, строка %zu: %s\n", path, line, status_message(status));
            break;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_ARGUMENT_COUNT:
        case STATUS_ERR_FILE_OPEN_FAILED:
        case STATUS_ERR_READ_FAILED:
        case STATUS_ERR_NUL_BYTE:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_MISSING_SEMICOLON:
        case STATUS_ERR_UNKNOWN_COMMAND:
        case STATUS_ERR_EXPECTED_OPEN_PAREN:
        case STATUS_ERR_EXPECTED_CLOSE_PAREN:
        case STATUS_ERR_EXTRA_TEXT:
        case STATUS_ERR_WRONG_ARGUMENT_COUNT:
        case STATUS_ERR_EMPTY_ARGUMENT:
        case STATUS_ERR_BAD_POLYNOMIAL:
        case STATUS_ERR_BAD_VARIABLE:
        case STATUS_ERR_DEGREE_TOO_LARGE:
        case STATUS_ERR_NUMBER_TOO_LONG:
        case STATUS_ERR_BAD_NUMBER:
        case STATUS_ERR_COEFFICIENT_OVERFLOW:
        case STATUS_ERR_VALUE_OVERFLOW:
        case STATUS_ERR_DIVISION_BY_ZERO:
            printf("Ошибка: %s: %s\n", status_message(status), path);
            break;
    }
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    if (argc != 2) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ARGUMENT_COUNT));
        printf("Пример: lab3_task8 commands.txt\n");
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        print_load_error(STATUS_ERR_FILE_OPEN_FAILED, 0, argv[1]);
        return 1;
    }
    char *text = NULL;
    Status status = read_text(file, &text);
    fclose(file);
    if (status != STATUS_OK) {
        print_load_error(status, 0, argv[1]);
        return 1;
    }
    size_t line = 0;
    status = strip_comments(text, &line);
    if (status != STATUS_OK) {
        print_load_error(status, line, argv[1]);
        free(text);
        return 1;
    }

    size_t done = 0;
    size_t failed = 0;
    const bool ok = run_script(text, &done, &failed);
    free(text);
    printf("Команд выполнено: %zu, с ошибкой: %zu\n", done, failed);
    if (!ok) {
        return 1;
    }
    return 0;
}
