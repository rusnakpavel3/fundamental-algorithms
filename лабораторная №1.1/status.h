#ifndef STATUS_H
#define STATUS_H


typedef enum {
    STATUS_OK = 0,

    /* ошибки разбора аргументов командной строки */
    STATUS_ERR_NULL_ARGUMENT,
    STATUS_ERR_ARGC_INVALID,
    STATUS_ERR_FLAG_FORMAT,
    STATUS_ERR_FLAG_UNKNOWN,
    STATUS_ERR_NUMBER_INVALID,
    STATUS_ERR_NUMBER_OVERFLOW,

    /* ошибки логики */
    STATUS_ERR_NUMBER_NEGATIVE,
    STATUS_ERR_VALUE_OUT_OF_RANGE,
    STATUS_ERR_COMPUTE_OVERFLOW
} Status;

#endif
