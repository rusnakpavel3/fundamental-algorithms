#include "string_ops.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

static Status allocate_string(const size_t length, char **out_text)
{
    if (length == SIZE_MAX) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }

    char *text = malloc(length + 1);
    if (text == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    text[length] = '\0';
    *out_text = text;
    return STATUS_OK;
}

static int char_class(const char c)
{
    if (isdigit((unsigned char) c)) {
        return 0;
    }
    if (isalpha((unsigned char) c)) {
        return 1;
    }
    return 2;
}

static size_t random_below(const size_t bound)
{
    const unsigned long long step = (unsigned long long) RAND_MAX + 1ULL;
    unsigned long long value = 0;
    unsigned long long range = 1;

    while (range < bound) {
        value = value * step + (unsigned long long) rand();
        range *= step;
    }
    return (size_t) (value % bound);
}

Status string_length(const char *text, size_t *out_length)
{
    if (text == NULL || out_length == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    size_t length = 0;
    while (text[length] != '\0') {
        length++;
    }

    *out_length = length;
    return STATUS_OK;
}

Status string_reverse(const char *source, char **out_result)
{
    if (source == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    size_t length = 0;
    Status status = string_length(source, &length);
    if (status != STATUS_OK) {
        return status;
    }

    char *result = NULL;
    status = allocate_string(length, &result);
    if (status != STATUS_OK) {
        return status;
    }

    for (size_t i = 0; i < length; i++) {
        result[i] = source[length - 1 - i];
    }

    *out_result = result;
    return STATUS_OK;
}

Status string_upper_odd(const char *source, char **out_result)
{
    if (source == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    size_t length = 0;
    Status status = string_length(source, &length);
    if (status != STATUS_OK) {
        return status;
    }

    char *result = NULL;
    status = allocate_string(length, &result);
    if (status != STATUS_OK) {
        return status;
    }

    for (size_t i = 0; i < length; i++) {
        if (i % 2 == 1) {
            result[i] = (char) toupper((unsigned char) source[i]);
        } else {
            result[i] = source[i];
        }
    }

    *out_result = result;
    return STATUS_OK;
}

Status string_group_by_class(const char *source, char **out_result)
{
    if (source == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    size_t length = 0;
    Status status = string_length(source, &length);
    if (status != STATUS_OK) {
        return status;
    }

    char *result = NULL;
    status = allocate_string(length, &result);
    if (status != STATUS_OK) {
        return status;
    }

    size_t written = 0;
    for (int wanted = 0; wanted < 3; wanted++) {
        for (size_t i = 0; i < length; i++) {
            if (char_class(source[i]) == wanted) {
                result[written] = source[i];
                written++;
            }
        }
    }

    *out_result = result;
    return STATUS_OK;
}

Status string_concat_random(const char *const *strings, const size_t count,
                            const unsigned int seed, char **out_result)
{
    if (strings == NULL || out_result == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (count == 0) {
        return STATUS_ERR_EMPTY_COLLECTION;
    }

    size_t total = 0;
    for (size_t i = 0; i < count; i++) {
        size_t length = 0;
        const Status status = string_length(strings[i], &length);
        if (status != STATUS_OK) {
            return status;
        }
        if (length > SIZE_MAX - 1 - total) {
            return STATUS_ERR_SIZE_OVERFLOW;
        }
        total += length;
    }

    if (count > SIZE_MAX / sizeof(size_t)) {
        return STATUS_ERR_SIZE_OVERFLOW;
    }
    size_t *order = (size_t*) malloc(count * sizeof *order); //@!!!!!!!!!!!!!!!!!
    if (order == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i < count; i++) {
        order[i] = i;
    }

    srand(seed);
    for (size_t i = count - 1; i > 0; i--) {
        const size_t j = random_below(i + 1);
        const size_t tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    char *result = NULL;
    const Status status = allocate_string(total, &result);
    if (status != STATUS_OK) {
        free(order);
        return status;
    }

    size_t written = 0;
    for (size_t k = 0; k < count; k++) {
        const char *part = strings[order[k]];
        for (size_t j = 0; part[j] != '\0'; j++) {
            result[written] = part[j];
            written++;
        }
    }

    free(order);
    *out_result = result;
    return STATUS_OK;
}
