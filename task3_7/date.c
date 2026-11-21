#include "date.h"

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


Status date_check(const Date *date)
{
    if (date == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (date->year < 1900 || date->month < 1 || date->month > 12) {
        return STATUS_ERR_BAD_DATE;
    }
    if (date->day < 1 || date->day > days_in_month(date->month, date->year)) {
        return STATUS_ERR_BAD_DATE;
    }
    return STATUS_OK;
}


Status date_parse(const char *text, Date *out_date)
{
    if (text == NULL || out_date == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const char pattern[] = "dd.MM.yyyy";
    if (strlen(text) != DATE_TEXT_LENGTH) {
        return STATUS_ERR_BAD_DATE_FORMAT;
    }
    for (size_t i = 0; i < DATE_TEXT_LENGTH; i++) {
        if (pattern[i] == '.' && text[i] != '.') {
            return STATUS_ERR_BAD_DATE_FORMAT;
        }
        if (pattern[i] != '.' && !is_digit(text[i])) {
            return STATUS_ERR_BAD_DATE_FORMAT;
        }
    }

    Date date;
    date.day = two_digits(text, 0);
    date.month = two_digits(text, 3);
    date.year = two_digits(text, 6) * 100 + two_digits(text, 8);

    const Status status = date_check(&date);
    if (status != STATUS_OK) {
        return status;
    }
    *out_date = date;
    return STATUS_OK;
}


Status date_compare(const Date *first, const Date *second, int *out_result)
{
    if (first == NULL || second == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const int a[3] = {first->year, first->month, first->day};
    const int b[3] = {second->year, second->month, second->day};
    *out_result = 0;
    for (size_t i = 0; i < 3; i++) {
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


Status date_today(Date *out_date)
{
    if (out_date == NULL) {
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
    out_date->day = local->tm_mday;
    out_date->month = local->tm_mon + 1;
    out_date->year = local->tm_year + 1900;
    return STATUS_OK;
}
