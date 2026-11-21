#ifndef DATETIME_H
#define DATETIME_H

#include "status.h"

#define MIN_YEAR 1900

typedef struct {
    int day;
    int month;
    int year;
    int hour;
    int minute;
    int second;
    long long seconds;
} DateTime;

Status datetime_parse(const char *date_text, const char *time_text, DateTime *out_datetime);

#endif
