#ifndef DATE_H
#define DATE_H

#include "status.h"

#define DATE_TEXT_LENGTH 10

typedef struct {
    int day;
    int month;
    int year;
} Date;

Status date_check(const Date *date);
Status date_parse(const char *text, Date *out_date);
Status date_compare(const Date *first, const Date *second, int *out_result);
Status date_today(Date *out_date);

#endif
