#include "liver.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIELDS 6


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static bool is_latin_letter(const char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


static bool is_space(const char c)
{
    return c == ' ' || c == '\t';
}


Status check_name(const char *text, const bool allow_empty)
{
    if (text == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (text[0] == '\0' && !allow_empty) {
        return STATUS_ERR_BAD_NAME;
    }
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (!is_latin_letter(text[i])) {
            return STATUS_ERR_BAD_NAME;
        }
    }
    return STATUS_OK;
}


Status parse_gender(const char *text, char *out_gender)
{
    if (text == NULL || out_gender == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (strcmp(text, "M") != 0 && strcmp(text, "W") != 0) {
        return STATUS_ERR_BAD_GENDER;
    }
    *out_gender = text[0];
    return STATUS_OK;
}


Status parse_income(const char *text, double *out_income)
{
    if (text == NULL || out_income == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t integer_digits = 0;
    size_t i = 0;
    while (is_digit(text[i])) {
        integer_digits++;
        i++;
    }
    if (integer_digits == 0 || integer_digits > MAX_INCOME_INTEGER_DIGITS) {
        return STATUS_ERR_BAD_INCOME;
    }
    if (text[i] == '.') {
        i++;
        size_t fraction_digits = 0;
        while (is_digit(text[i])) {
            fraction_digits++;
            i++;
        }
        if (fraction_digits == 0 || fraction_digits > MAX_INCOME_FRACTION_DIGITS) {
            return STATUS_ERR_BAD_INCOME;
        }
    }
    if (text[i] != '\0') {
        return STATUS_ERR_BAD_INCOME;
    }

    char *end = NULL;
    const double value = strtod(text, &end);
    if (end == text || *end != '\0') {
        return STATUS_ERR_BAD_INCOME;
    }
    *out_income = value;
    return STATUS_OK;
}


Status parse_birth_date(const char *text, const Date *today, Date *out_date)
{
    if (text == NULL || today == NULL || out_date == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Date date;
    Status status = date_parse(text, &date);
    if (status != STATUS_OK) {
        return status;
    }
    int order = 0;
    status = date_compare(&date, today, &order);
    if (status != STATUS_OK) {
        return status;
    }
    if (order > 0) {
        return STATUS_ERR_DATE_IN_FUTURE;
    }
    *out_date = date;
    return STATUS_OK;
}


static Status copy_text(const char *text, char **out_copy)
{
    const size_t length = strlen(text);
    char *copy = (char *) malloc(length + 1);
    if (copy == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i <= length; i++) {
        copy[i] = text[i];
    }
    *out_copy = copy;
    return STATUS_OK;
}


Status liver_create(const LiverFields *fields, const Date *today, Liver *out_liver)
{
    if (fields == NULL || today == NULL || out_liver == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (fields->surname == NULL || fields->name == NULL || fields->patronymic == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (check_name(fields->surname, false) != STATUS_OK) {
        return STATUS_ERR_BAD_SURNAME;
    }
    if (check_name(fields->name, false) != STATUS_OK) {
        return STATUS_ERR_BAD_NAME;
    }
    if (check_name(fields->patronymic, true) != STATUS_OK) {
        return STATUS_ERR_BAD_PATRONYMIC;
    }
    Status status = date_check(&fields->birth);
    if (status != STATUS_OK) {
        return status;
    }
    int order = 0;
    status = date_compare(&fields->birth, today, &order);
    if (status != STATUS_OK) {
        return status;
    }
    if (order > 0) {
        return STATUS_ERR_DATE_IN_FUTURE;
    }
    if (fields->gender != 'M' && fields->gender != 'W') {
        return STATUS_ERR_BAD_GENDER;
    }
    if (isnan(fields->income) || isinf(fields->income) || fields->income < 0.0) {
        return STATUS_ERR_BAD_INCOME;
    }

    Liver liver;
    liver.surname = NULL;
    liver.name = NULL;
    liver.patronymic = NULL;
    liver.birth = fields->birth;
    liver.gender = fields->gender;
    liver.income = fields->income;

    status = copy_text(fields->surname, &liver.surname);
    if (status == STATUS_OK) {
        status = copy_text(fields->name, &liver.name);
    }
    if (status == STATUS_OK) {
        status = copy_text(fields->patronymic, &liver.patronymic);
    }
    if (status != STATUS_OK) {
        liver_free(&liver);
        return status;
    }

    *out_liver = liver;
    return STATUS_OK;
}


Status liver_copy(const Liver *source, Liver *out_copy)
{
    if (source == NULL || out_copy == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Liver copy;
    copy.surname = NULL;
    copy.name = NULL;
    copy.patronymic = NULL;
    copy.birth = source->birth;
    copy.gender = source->gender;
    copy.income = source->income;

    Status status = copy_text(source->surname, &copy.surname);
    if (status == STATUS_OK) {
        status = copy_text(source->name, &copy.name);
    }
    if (status == STATUS_OK) {
        status = copy_text(source->patronymic, &copy.patronymic);
    }
    if (status != STATUS_OK) {
        liver_free(&copy);
        return status;
    }

    *out_copy = copy;
    return STATUS_OK;
}


void liver_free(Liver *liver)
{
    if (liver == NULL) {
        return;
    }
    free(liver->surname);
    free(liver->name);
    free(liver->patronymic);
    liver->surname = NULL;
    liver->name = NULL;
    liver->patronymic = NULL;
}


static size_t split_fields(char *line, char *fields[], const size_t max_fields)
{
    size_t count = 0;
    size_t i = 0;
    while (line[i] != '\0') {
        while (is_space(line[i])) {
            line[i] = '\0';
            i++;
        }
        if (line[i] == '\0') {
            break;
        }
        if (count == max_fields) {
            return max_fields + 1;
        }
        fields[count] = &line[i];
        count++;
        while (line[i] != '\0' && !is_space(line[i])) {
            i++;
        }
    }
    return count;
}


Status liver_parse_line(char *line, const Date *today, Liver *out_liver)
{
    if (line == NULL || today == NULL || out_liver == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    char *fields[MAX_FIELDS];
    const size_t count = split_fields(line, fields, MAX_FIELDS);
    if (count != 5 && count != 6) {
        return STATUS_ERR_FIELD_COUNT;
    }

    LiverFields input;
    size_t next = 0;
    input.surname = fields[next];
    next++;
    input.name = fields[next];
    next++;
    input.patronymic = "";
    if (count == 6) {
        input.patronymic = fields[next];
        next++;
    }

    Status status = parse_birth_date(fields[next], today, &input.birth);
    if (status != STATUS_OK) {
        return status;
    }
    status = parse_gender(fields[next + 1], &input.gender);
    if (status != STATUS_OK) {
        return status;
    }
    status = parse_income(fields[next + 2], &input.income);
    if (status != STATUS_OK) {
        return status;
    }
    return liver_create(&input, today, out_liver);
}
