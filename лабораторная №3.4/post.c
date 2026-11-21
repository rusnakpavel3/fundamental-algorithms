#include "post.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUMBER_DIGITS 9
#define MAX_WEIGHT_LENGTH 15
#define START_CAPACITY 4


static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}


static bool all_digits(const char *text)
{
    for (size_t i = 0; text[i] != '\0'; i++) {
        if (!is_digit(text[i])) {
            return false;
        }
    }
    return true;
}


Status parse_natural(const char *text, unsigned int *out_value)
{
    if (text == NULL || out_value == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const size_t length = strlen(text);
    if (length == 0 || length > MAX_NUMBER_DIGITS || !all_digits(text)) {
        return STATUS_ERR_BAD_NUMBER;
    }
    char *end = NULL;
    const unsigned long value = strtoul(text, &end, 10);
    if (end == text || *end != '\0' || value == 0) {
        return STATUS_ERR_BAD_NUMBER;
    }
    *out_value = (unsigned int) value;
    return STATUS_OK;
}


Status check_postal_index(const char *text)
{
    if (text == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (strlen(text) != INDEX_LENGTH || !all_digits(text)) {
        return STATUS_ERR_BAD_INDEX;
    }
    return STATUS_OK;
}


Status check_mail_id(const char *text)
{
    if (text == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (strlen(text) != MAIL_ID_LENGTH || !all_digits(text)) {
        return STATUS_ERR_BAD_ID;
    }
    return STATUS_OK;
}


Status parse_weight(const char *text, double *out_weight)
{
    if (text == NULL || out_weight == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const size_t length = strlen(text);
    if (length == 0 || length > MAX_WEIGHT_LENGTH) {
        return STATUS_ERR_BAD_WEIGHT;
    }
    if (!is_digit(text[0]) || !is_digit(text[length - 1])) {
        return STATUS_ERR_BAD_WEIGHT;
    }
    size_t points = 0;
    for (size_t i = 0; i < length; i++) {
        if (text[i] == '.') {
            points++;
        } else if (!is_digit(text[i])) {
            return STATUS_ERR_BAD_WEIGHT;
        }
    }
    if (points > 1) {
        return STATUS_ERR_BAD_WEIGHT;
    }

    char *end = NULL;
    const double value = strtod(text, &end);
    if (end == text || *end != '\0') {
        return STATUS_ERR_BAD_WEIGHT;
    }
    *out_weight = value;
    return STATUS_OK;
}


static void set_empty(String *string)
{
    string->data = NULL;
    string->length = 0;
}


static void address_clear(Address *address)
{
    string_clear(&address->city);
    string_clear(&address->street);
    string_clear(&address->building);
    string_clear(&address->index);
}


static Status address_create(const AddressFields *fields, Address *out_address)
{
    if (fields->city == NULL || fields->street == NULL || fields->building == NULL || fields->index == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (fields->city[0] == '\0' || fields->street[0] == '\0') {
        return STATUS_ERR_EMPTY_FIELD;
    }
    if (fields->house == 0 || fields->flat == 0) {
        return STATUS_ERR_BAD_NUMBER;
    }
    Status status = check_postal_index(fields->index);
    if (status != STATUS_OK) {
        return status;
    }

    Address address;
    set_empty(&address.city);
    set_empty(&address.street);
    set_empty(&address.building);
    set_empty(&address.index);
    address.house = fields->house;
    address.flat = fields->flat;

    status = string_create(fields->city, &address.city);
    if (status == STATUS_OK) {
        status = string_create(fields->street, &address.street);
    }
    if (status == STATUS_OK) {
        status = string_create(fields->building, &address.building);
    }
    if (status == STATUS_OK) {
        status = string_create(fields->index, &address.index);
    }
    if (status != STATUS_OK) {
        address_clear(&address);
        return status;
    }

    *out_address = address;
    return STATUS_OK;
}


static void mail_clear(Mail *mail)
{
    address_clear(&mail->to);
    string_clear(&mail->id);
    string_clear(&mail->created_text);
    string_clear(&mail->deadline_text);
}


static int compare_by_index_and_id(const void *a, const void *b)
{
    const Mail *first = (const Mail *) a;
    const Mail *second = (const Mail *) b;
    int result = 0;
    string_compare_lex(&first->to.index, &second->to.index, &result);
    if (result == 0) {
        string_compare_lex(&first->id, &second->id, &result);
    }
    return result;
}


static int compare_by_created(const void *a, const void *b)
{
    const Mail *first = (const Mail *) a;
    const Mail *second = (const Mail *) b;
    int result = 0;
    datetime_compare(&first->created, &second->created, &result);
    if (result == 0) {
        string_compare_lex(&first->id, &second->id, &result);
    }
    return result;
}


Status post_create(const AddressFields *office, Post *out_post)
{
    if (office == NULL || out_post == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Address *address = (Address *) malloc(sizeof(Address));
    if (address == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    const Status status = address_create(office, address);
    if (status != STATUS_OK) {
        free(address);
        return status;
    }

    out_post->office = address;
    out_post->mails = NULL;
    out_post->count = 0;
    out_post->capacity = 0;
    return STATUS_OK;
}


void post_free(Post *post)
{
    if (post == NULL) {
        return;
    }
    if (post->office != NULL) {
        address_clear(post->office);
        free(post->office);
        post->office = NULL;
    }
    for (size_t i = 0; i < post->count; i++) {
        mail_clear(&post->mails[i]);
    }
    free(post->mails);
    post->mails = NULL;
    post->count = 0;
    post->capacity = 0;
}


Status post_add(Post *post, const MailFields *fields)
{
    if (post == NULL || fields == NULL || fields->id == NULL || fields->created == NULL || fields->deadline == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (isnan(fields->weight) || isinf(fields->weight) || fields->weight < 0.0) {
        return STATUS_ERR_BAD_WEIGHT;
    }
    Status status = check_mail_id(fields->id);
    if (status != STATUS_OK) {
        return status;
    }
    size_t existing = 0;
    if (post_find(post, fields->id, &existing) == STATUS_OK) {
        return STATUS_ERR_DUPLICATE_ID;
    }

    DateTime created;
    DateTime deadline;
    status = datetime_parse(fields->created, &created);
    if (status != STATUS_OK) {
        return status;
    }
    status = datetime_parse(fields->deadline, &deadline);
    if (status != STATUS_OK) {
        return status;
    }
    int order = 0;
    datetime_compare(&deadline, &created, &order);
    if (order < 0) {
        return STATUS_ERR_DEADLINE_BEFORE_CREATION;
    }

    Mail mail;
    status = address_create(&fields->to, &mail.to);
    if (status != STATUS_OK) {
        return status;
    }
    mail.weight = fields->weight;
    mail.created = created;
    mail.deadline = deadline;
    mail.delivered = false;
    set_empty(&mail.id);
    set_empty(&mail.created_text);
    set_empty(&mail.deadline_text);

    status = string_create(fields->id, &mail.id);
    if (status == STATUS_OK) {
        status = string_create(fields->created, &mail.created_text);
    }
    if (status == STATUS_OK) {
        status = string_create(fields->deadline, &mail.deadline_text);
    }
    if (status != STATUS_OK) {
        mail_clear(&mail);
        return status;
    }

    if (post->count == post->capacity) {
        size_t new_capacity = START_CAPACITY;
        if (post->capacity > 0) {
            if (post->capacity > SIZE_MAX / 2 / sizeof(Mail)) {
                mail_clear(&mail);
                return STATUS_ERR_SIZE_OVERFLOW;
            }
            new_capacity = post->capacity * 2;
        }
        Mail *grown = (Mail *) realloc(post->mails, new_capacity * sizeof(Mail));
        if (grown == NULL) {
            mail_clear(&mail);
            return STATUS_ERR_ALLOC_FAILED;
        }
        post->mails = grown;
        post->capacity = new_capacity;
    }

    post->mails[post->count] = mail;
    post->count++;
    qsort(post->mails, post->count, sizeof(Mail), compare_by_index_and_id);
    return STATUS_OK;
}


Status post_find(const Post *post, const char *id, size_t *out_index)
{
    if (post == NULL || id == NULL || out_index == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    for (size_t i = 0; i < post->count; i++) {
        if (strcmp(post->mails[i].id.data, id) == 0) {
            *out_index = i;
            return STATUS_OK;
        }
    }
    return STATUS_ERR_NOT_FOUND;
}


Status post_remove(Post *post, const char *id)
{
    size_t index = 0;
    const Status status = post_find(post, id, &index);
    if (status != STATUS_OK) {
        return status;
    }
    mail_clear(&post->mails[index]);
    for (size_t i = index; i + 1 < post->count; i++) {
        post->mails[i] = post->mails[i + 1];
    }
    post->count--;
    return STATUS_OK;
}


Status post_deliver(Post *post, const char *id)
{
    size_t index = 0;
    const Status status = post_find(post, id, &index);
    if (status != STATUS_OK) {
        return status;
    }
    if (post->mails[index].delivered) {
        return STATUS_ERR_ALREADY_DELIVERED;
    }
    post->mails[index].delivered = true;
    return STATUS_OK;
}


bool mail_is_expired(const Mail *mail, const DateTime *now)
{
    if (mail == NULL || now == NULL || mail->delivered) {
        return false;
    }
    int order = 0;
    datetime_compare(&mail->deadline, now, &order);
    return order < 0;
}


static bool is_selected(const Mail *mail, const Selection kind, const DateTime *now)
{
    if (kind == SELECT_DELIVERED) {
        return mail->delivered;
    }
    return mail_is_expired(mail, now);
}


Status post_select(const Post *post, const Selection kind, const DateTime *now, Mail **out_list, size_t *out_count)
{
    if (post == NULL || now == NULL || out_list == NULL || out_count == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_list = NULL;
    *out_count = 0;

    size_t found = 0;
    for (size_t i = 0; i < post->count; i++) {
        if (is_selected(&post->mails[i], kind, now)) {
            found++;
        }
    }
    if (found == 0) {
        return STATUS_OK;
    }

    Mail *list = (Mail *) malloc(found * sizeof(Mail));
    if (list == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    size_t filled = 0;
    for (size_t i = 0; i < post->count; i++) {
        if (is_selected(&post->mails[i], kind, now)) {
            list[filled] = post->mails[i];
            filled++;
        }
    }
    qsort(list, found, sizeof(Mail), compare_by_created);

    *out_list = list;
    *out_count = found;
    return STATUS_OK;
}
