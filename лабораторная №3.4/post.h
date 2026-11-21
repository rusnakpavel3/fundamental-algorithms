#ifndef POST_H
#define POST_H

#include <stdbool.h>
#include <stddef.h>

#include "datetime.h"
#include "my_string.h"
#include "status.h"

#define INDEX_LENGTH 6
#define MAIL_ID_LENGTH 14

typedef struct {
    const char *city;
    const char *street;
    unsigned int house;
    const char *building;
    unsigned int flat;
    const char *index;
} AddressFields;

typedef struct {
    AddressFields to;
    double weight;
    const char *id;
    const char *created;
    const char *deadline;
} MailFields;

typedef struct {
    String city;
    String street;
    unsigned int house;
    String building;
    unsigned int flat;
    String index;
} Address;

typedef struct {
    Address to;
    double weight;
    String id;
    String created_text;
    String deadline_text;
    DateTime created;
    DateTime deadline;
    bool delivered;
} Mail;

typedef struct {
    Address *office;
    Mail *mails;
    size_t count;
    size_t capacity;
} Post;

typedef enum {
    SELECT_DELIVERED,
    SELECT_EXPIRED
} Selection;

Status parse_natural(const char *text, unsigned int *out_value);
Status check_postal_index(const char *text);
Status check_mail_id(const char *text);
Status parse_weight(const char *text, double *out_weight);

Status post_create(const AddressFields *office, Post *out_post);
void post_free(Post *post);
Status post_add(Post *post, const MailFields *fields);
Status post_find(const Post *post, const char *id, size_t *out_index);
Status post_remove(Post *post, const char *id);
Status post_deliver(Post *post, const char *id);
bool mail_is_expired(const Mail *mail, const DateTime *now);
Status post_select(const Post *post, Selection kind, const DateTime *now, Mail **out_list, size_t *out_count);

#endif
