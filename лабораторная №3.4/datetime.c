#include "datetime.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <time.h>


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static int two_digits(const char *text, const size_t position)
{
    return (text[position] - '0') * 10 + (text[position + 1] - '0');
}


static bool is_leap_year(const int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}


static int days_in_month(const int month, const int year)
{
    if (month == 2) {
        if (is_leap_year(year)) {
            return 29;
        }
        return 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 31;
}


Status datetime_parse(const char *text, DateTime *out_time)
{
    if (text == NULL || out_time == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const char pattern[] = "dd:MM:yyyy hh:mm:ss";
    if (strlen(text) != DATETIME_TEXT_LENGTH) {
        return STATUS_ERR_BAD_TIME_FORMAT;
    }
    for (size_t i = 0; i < DATETIME_TEXT_LENGTH; i++) {
        const bool need_digit = pattern[i] != ':' && pattern[i] != ' ';
        if (need_digit && !is_digit(text[i])) {
            return STATUS_ERR_BAD_TIME_FORMAT;
        }
        if (!need_digit && text[i] != pattern[i]) {
            return STATUS_ERR_BAD_TIME_FORMAT;
        }
    }

    DateTime time;
    time.day = two_digits(text, 0);
    time.month = two_digits(text, 3);
    time.year = two_digits(text, 6) * 100 + two_digits(text, 8);
    time.hour = two_digits(text, 11);
    time.minute = two_digits(text, 14);
    time.second = two_digits(text, 17);

    if (time.year < 1900 || time.month < 1 || time.month > 12) {
        return STATUS_ERR_BAD_DATE;
    }
    if (time.day < 1 || time.day > days_in_month(time.month, time.year)) {
        return STATUS_ERR_BAD_DATE;
    }
    if (time.hour > 23 || time.minute > 59 || time.second > 59) {
        return STATUS_ERR_BAD_DATE;
    }

    *out_time = time;
    return STATUS_OK;
}


Status datetime_compare(const DateTime *first, const DateTime *second, int *out_result)
{
    if (first == NULL || second == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const int a[6] = {first->year, first->month, first->day, first->hour, first->minute, first->second};
    const int b[6] = {second->year, second->month, second->day, second->hour, second->minute, second->second};
    *out_result = 0;
    for (size_t i = 0; i < 6; i++) {
        if (a[i] < b[i]) {
            *out_result = -1;
            return STATUS_OK;
        }
        if (a[i] > b[i]) {
            *out_result = 1;
            return STATUS_OK;
        }
    }
    return STATUS_OK;
}


Status datetime_now(DateTime *out_time)
{
    if (out_time == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const time_t seconds = time(NULL);
    if (seconds == (time_t) -1) {
        return STATUS_ERR_TIME_UNAVAILABLE;
    }
    const struct tm *local = localtime(&seconds);
    if (local == NULL) {
        return STATUS_ERR_TIME_UNAVAILABLE;
    }
    out_time->year = local->tm_year + 1900;
    out_time->month = local->tm_mon + 1;
    out_time->day = local->tm_mday;
    out_time->hour = local->tm_hour;
    out_time->minute = local->tm_min;
    out_time->second = local->tm_sec;
    return STATUS_OK;
}
