#ifndef STATUS_H
#define STATUS_H

typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_NULL_ARGUMENT,
    STATUS_ERR_POWER_OUT_OF_RANGE,
    STATUS_ERR_BUFFER_TOO_SMALL
} Status;

const char *status_message(const Status status);

#endif
