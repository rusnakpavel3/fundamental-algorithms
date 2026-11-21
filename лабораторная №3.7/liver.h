#ifndef LIVER_H
#define LIVER_H

#include <stdbool.h>

#include "date.h"
#include "status.h"

#define MAX_INCOME_INTEGER_DIGITS 12
#define MAX_INCOME_FRACTION_DIGITS 2

typedef struct {
    char *surname;
    char *name;
    char *patronymic;
    Date birth;
    char gender;
    double income;
} Liver;

typedef struct {
    const char *surname;
    const char *name;
    const char *patronymic;
    Date birth;
    char gender;
    double income;
} LiverFields;

Status check_name(const char *text, bool allow_empty);
Status parse_gender(const char *text, char *out_gender);
Status parse_income(const char *text, double *out_income);
Status parse_birth_date(const char *text, const Date *today, Date *out_date);

Status liver_create(const LiverFields *fields, const Date *today, Liver *out_liver);
Status liver_copy(const Liver *source, Liver *out_copy);
void liver_free(Liver *liver);
Status liver_parse_line(char *line, const Date *today, Liver *out_liver);

#endif
