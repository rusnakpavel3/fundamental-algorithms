#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "console.h"
#include "same_file.h"
#include "status.h"
#include "student.h"
#include "student_file.h"
#include "trace.h"

#define LINE_SIZE 256


static void print_menu(void)
{
    printf("Команды:\n");
    printf("  1 - найти по id\n");
    printf("  2 - найти по фамилии\n");
    printf("  3 - найти по имени\n");
    printf("  4 - найти по группе\n");
    printf("  5 - сортировать по id\n");
    printf("  6 - сортировать по фамилии\n");
    printf("  7 - сортировать по имени\n");
    printf("  8 - сортировать по группе\n");
    printf("  9 - в трассировочный файл: студент по id\n");
    printf("  10 - в трассировочный файл: студенты выше среднего балла\n");
    printf("  11 - показать всех\n");
    printf("  12 - показать команды\n");
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


static bool ask_id(unsigned long *out_id)
{
    char text[LINE_SIZE];
    if (!ask_line("id: ", text, sizeof text)) {
        return false;
    }
    const Status status = parse_id(text, out_id);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return false;
    }
    return true;
}


static void print_student(const Student *student)
{
    double average = 0.0;
    student_average(student, &average);
    printf("  id %lu: %s %s, группа %s, оценки", student->id, student->last_name, student->first_name,
           student->group);
    for (size_t g = 0; g < GRADE_COUNT; g++) {
        printf(" %u", (unsigned int) student->grades[g]);
    }
    printf(", средний %.2f\n", average);
}


static void print_all(const Student *students, const size_t count)
{
    for (size_t i = 0; i < count; i++) {
        print_student(&students[i]);
    }
}


static void command_find_id(const Student *students, const size_t count)
{
    unsigned long id = 0;
    if (!ask_id(&id)) {
        return;
    }
    size_t index = 0;
    const Status status = find_by_id(students, count, id, &index);
    if (status == STATUS_OK) {
        print_student(&students[index]);
    } else if (status == STATUS_ERR_NOT_FOUND) {
        printf("Студента с id %lu нет\n", id);
    } else {
        printf("Ошибка: %s\n", status_message(status));
    }
}


static void command_find_text(const Student *students, const size_t count, size_t *indices,
                              const TextField field, const char *prompt)
{
    char key[LINE_SIZE];
    if (!ask_line(prompt, key, sizeof key)) {
        return;
    }
    size_t found = 0;
    const Status status = find_by_text(students, count, field, key, indices, &found);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    if (found == 0) {
        printf("Никого не найдено\n");
        return;
    }
    printf("Найдено: %zu\n", found);
    for (size_t i = 0; i < found; i++) {
        print_student(&students[indices[i]]);
    }
}


static void command_sort(Student *students, const size_t count, const SortKey key, const char *title)
{
    const Status status = sort_students(students, count, key);
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Отсортировано %s:\n", title);
    print_all(students, count);
}


static void command_trace_id(const Student *students, const size_t count, FILE *trace)
{
    unsigned long id = 0;
    if (!ask_id(&id)) {
        return;
    }
    size_t index = 0;
    Status status = find_by_id(students, count, id, &index);
    if (status == STATUS_ERR_NOT_FOUND) {
        printf("Студента с id %lu нет, в файл ничего не записано\n", id);
        return;
    }
    if (status == STATUS_OK) {
        status = trace_student(trace, &students[index]);
    }
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Записано в трассировочный файл\n");
}


static void command_trace_above(const Student *students, const size_t count, size_t *indices, FILE *trace)
{
    size_t found = 0;
    double total_average = 0.0;
    Status status = find_above_average(students, count, indices, &found, &total_average);
    if (status == STATUS_OK) {
        status = trace_above_average(trace, students, indices, found, total_average);
    }
    if (status != STATUS_OK) {
        printf("Ошибка: %s\n", status_message(status));
        return;
    }
    printf("Средний балл по всем: %.2f, выше него студентов: %zu. Записано в трассировочный файл\n",
           total_average, found);
}


static void run_dialog(Student *students, const size_t count, size_t *indices, FILE *trace)
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
            command_find_id(students, count);
        } else if (strcmp(command, "2") == 0) {
            command_find_text(students, count, indices, FIELD_LAST_NAME, "Фамилия: ");
        } else if (strcmp(command, "3") == 0) {
            command_find_text(students, count, indices, FIELD_FIRST_NAME, "Имя: ");
        } else if (strcmp(command, "4") == 0) {
            command_find_text(students, count, indices, FIELD_GROUP, "Группа: ");
        } else if (strcmp(command, "5") == 0) {
            command_sort(students, count, SORT_BY_ID, "по id");
        } else if (strcmp(command, "6") == 0) {
            command_sort(students, count, SORT_BY_LAST_NAME, "по фамилии");
        } else if (strcmp(command, "7") == 0) {
            command_sort(students, count, SORT_BY_FIRST_NAME, "по имени");
        } else if (strcmp(command, "8") == 0) {
            command_sort(students, count, SORT_BY_GROUP, "по группе");
        } else if (strcmp(command, "9") == 0) {
            command_trace_id(students, count, trace);
        } else if (strcmp(command, "10") == 0) {
            command_trace_above(students, count, indices, trace);
        } else if (strcmp(command, "11") == 0) {
            print_all(students, count);
        } else if (strcmp(command, "12") == 0) {
            print_menu();
        } else {
            printf("Неизвестная команда: введите число от 0 до 12\n");
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
        case STATUS_ERR_BAD_ID:
        case STATUS_ERR_BAD_FIRST_NAME:
        case STATUS_ERR_BAD_LAST_NAME:
        case STATUS_ERR_BAD_GRADE:
        case STATUS_ERR_DUPLICATE_ID:
            printf("Ошибка в файле %s, строка %zu: %s\n", path, line, status_message(status));
            break;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_ARGUMENT_COUNT:
        case STATUS_ERR_FILE_OPEN_FAILED:
        case STATUS_ERR_FILE_INFO_FAILED:
        case STATUS_ERR_SAME_FILE:
        case STATUS_ERR_TRACE_OPEN_FAILED:
        case STATUS_ERR_READ_FAILED:
        case STATUS_ERR_WRITE_FAILED:
        case STATUS_ERR_CLOSE_FAILED:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_NO_STUDENTS:
        case STATUS_ERR_NOT_FOUND:
        case STATUS_ERR_SUM_OVERFLOW:
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

    if (argc != 3) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ARGUMENT_COUNT));
        printf("Пример: lab3_task5 students.txt trace.txt\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        print_load_error(STATUS_ERR_FILE_OPEN_FAILED, 0, argv[1]);
        return 1;
    }
    bool same = false;
    Status status = file_matches_path(input, argv[2], &same);
    if (status == STATUS_OK && same) {
        status = STATUS_ERR_SAME_FILE;
    }
    if (status != STATUS_OK) {
        fclose(input);
        print_load_error(status, 0, argv[2]);
        return 1;
    }

    Student *students = NULL;
    size_t count = 0;
    size_t line = 0;
    status = read_students(input, &students, &count, &line);
    fclose(input);
    if (status != STATUS_OK) {
        print_load_error(status, line, argv[1]);
        return 1;
    }

    if (count > SIZE_MAX / sizeof(size_t)) {
        students_free(students, count);
        printf("Ошибка: %s\n", status_message(STATUS_ERR_SIZE_OVERFLOW));
        return 1;
    }
    size_t *indices = (size_t *) malloc(count * sizeof(size_t));
    if (indices == NULL) {
        students_free(students, count);
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ALLOC_FAILED));
        return 1;
    }

    FILE *trace = fopen(argv[2], "w");
    if (trace == NULL) {
        free(indices);
        students_free(students, count);
        print_load_error(STATUS_ERR_TRACE_OPEN_FAILED, 0, argv[2]);
        return 1;
    }

    printf("Загружено студентов: %zu\n", count);
    run_dialog(students, count, indices, trace);

    free(indices);
    students_free(students, count);
    if (fclose(trace) == EOF) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_CLOSE_FAILED));
        return 1;
    }
    return 0;
}
