#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_NULL_ARGUMENT,
    STATUS_ERR_BASE_OUT_OF_RANGE,
    STATUS_ERR_COUNT_NEGATIVE,
    STATUS_ERR_NUMBER_EMPTY,
    STATUS_ERR_NUMBER_INVALID,
    STATUS_ERR_SIZE_OVERFLOW,
    STATUS_ERR_ALLOC_FAILED
} Status;

const char *status_message(const Status status);

#endif
