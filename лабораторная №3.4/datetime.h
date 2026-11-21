#ifndef DATETIME_H
#define DATETIME_H

#include "status.h"

#define DATETIME_TEXT_LENGTH 19

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} DateTime;

Status datetime_parse(const char *text, DateTime *out_time);
Status datetime_compare(const DateTime *first, const DateTime *second, int *out_result);
Status datetime_now(DateTime *out_time);

#endif
