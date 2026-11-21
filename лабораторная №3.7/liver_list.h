#ifndef LIVER_LIST_H
#define LIVER_LIST_H

#include <stdbool.h>
#include <stddef.h>

#include "liver.h"
#include "status.h"

typedef struct Node {
    Liver data;
    struct Node *next;
} Node;

typedef enum {
    CHANGE_ADD,
    CHANGE_REMOVE,
    CHANGE_EDIT
} ChangeKind;

typedef struct {
    ChangeKind kind;
    size_t old_position;
    size_t new_position;
    Node *old_node;
} Change;

typedef struct {
    Node *head;
    size_t count;
    Change *history;
    size_t history_count;
    size_t history_capacity;
} LiverList;

typedef struct {
    const char *surname;
    const char *name;
    const char *patronymic;
    bool use_birth;
    Date birth;
    char gender;
    bool use_income;
    double income;
} SearchQuery;

void list_init(LiverList *list);
void list_free(LiverList *list);
Status list_insert_loaded(LiverList *list, const Liver *liver);
Status list_add(LiverList *list, const Liver *liver, size_t *out_position);
Status list_remove(LiverList *list, size_t position);
Status list_edit(LiverList *list, size_t position, const Liver *new_data, size_t *out_new_position);
Status list_get(const LiverList *list, size_t position, const Liver **out_liver);
Status list_search(const LiverList *list, const SearchQuery *query, double epsilon, size_t *out_positions,
                   size_t capacity, size_t *out_found);
Status list_undo(LiverList *list, size_t *out_undone);

#endif
