#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель на буфер результата";
        case STATUS_ERR_POWER_OUT_OF_RANGE:
            return "показатель r должен быть от 1 до 5";
        case STATUS_ERR_BUFFER_TOO_SMALL:
            return "буфер результата слишком мал";
    }
    return "неизвестный статус";
}
