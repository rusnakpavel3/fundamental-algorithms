#include "datetime.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define SECONDS_PER_DAY 86400LL


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static int two_digits(const char *text, const size_t position)
{
    return (text[position] - '0') * 10 + (text[position + 1] - '0');
}


static bool matches_pattern(const char *text, const char *pattern, const char separator)
{
    if (strlen(text) != strlen(pattern)) {
        return false;
    }
    for (size_t i = 0; pattern[i] != '\0'; i++) {
        if (pattern[i] == separator && text[i] != separator) {
            return false;
        }
        if (pattern[i] != separator && !is_digit(text[i])) {
            return false;
        }
    }
    return true;
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


static long long days_before_year(const int year)
{
    const long long previous = year - 1;
    const long long leap_years = previous / 4 - previous / 100 + previous / 400;
    return previous * 365 + leap_years;
}


static long long days_since_start(const int day, const int month, const int year)
{
    long long days = days_before_year(year) - days_before_year(MIN_YEAR);
    for (int m = 1; m < month; m++) {
        days += days_in_month(m, year);
    }
    return days + day - 1;
}


Status datetime_parse(const char *date_text, const char *time_text, DateTime *out_datetime)
{
    if (date_text == NULL || time_text == NULL || out_datetime == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (!matches_pattern(date_text, "dd.MM.yyyy", '.') || !matches_pattern(time_text, "hh:mm:ss", ':')) {
        return STATUS_ERR_BAD_DATETIME_FORMAT;
    }

    DateTime datetime;
    datetime.day = two_digits(date_text, 0);
    datetime.month = two_digits(date_text, 3);
    datetime.year = two_digits(date_text, 6) * 100 + two_digits(date_text, 8);
    datetime.hour = two_digits(time_text, 0);
    datetime.minute = two_digits(time_text, 3);
    datetime.second = two_digits(time_text, 6);

    if (datetime.year < MIN_YEAR || datetime.month < 1 || datetime.month > 12) {
        return STATUS_ERR_BAD_DATETIME;
    }
    if (datetime.day < 1 || datetime.day > days_in_month(datetime.month, datetime.year)) {
        return STATUS_ERR_BAD_DATETIME;
    }
    if (datetime.hour > 23 || datetime.minute > 59 || datetime.second > 59) {
        return STATUS_ERR_BAD_DATETIME;
    }

    datetime.seconds = days_since_start(datetime.day, datetime.month, datetime.year) * SECONDS_PER_DAY
                       + datetime.hour * 3600LL + datetime.minute * 60LL + datetime.second;
    *out_datetime = datetime;
    return STATUS_OK;
}
