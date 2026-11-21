#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_NULL_ARGUMENT,
    STATUS_ERR_ARGUMENT_COUNT,
    STATUS_ERR_FILE_OPEN_FAILED,
    STATUS_ERR_READ_FAILED,
    STATUS_ERR_FILE_TOO_LARGE,
    STATUS_ERR_ALLOC_FAILED,
    STATUS_ERR_SIZE_OVERFLOW
} Status;

const char *status_message(const Status status);

#endif
