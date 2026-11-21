#include "liver_list.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define START_HISTORY_CAPACITY 4


static Node *create_node(const Liver *data)
{
    Node *node = (Node *) malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    if (liver_copy(data, &node->data) != STATUS_OK) {
        free(node);
        return NULL;
    }
    node->next = NULL;
    return node;
}


static void free_node(Node *node)
{
    if (node == NULL) {
        return;
    }
    liver_free(&node->data);
    free(node);
}


static bool is_older(const Liver *first, const Liver *second)
{
    int order = 0;
    if (date_compare(&first->birth, &second->birth, &order) != STATUS_OK) {
        return false;
    }
    return order < 0;
}


static bool same_birth(const Liver *first, const Liver *second)
{
    int order = 0;
    if (date_compare(&first->birth, &second->birth, &order) != STATUS_OK) {
        return false;
    }
    return order == 0;
}


static size_t sorted_position(const LiverList *list, const Liver *liver)
{
    size_t position = 0;
    const Node *current = list->head;
    while (current != NULL && !is_older(&current->data, liver)) {
        position++;
        current = current->next;
    }
    return position;
}


static void link_at(LiverList *list, Node *node, const size_t position)
{
    if (position == 0) {
        node->next = list->head;
        list->head = node;
    } else {
        Node *previous = list->head;
        for (size_t i = 1; i < position; i++) {
            previous = previous->next;
        }
        node->next = previous->next;
        previous->next = node;
    }
    list->count++;
}


static Node *unlink_at(LiverList *list, const size_t position)
{
    Node *node = NULL;
    if (position == 0) {
        node = list->head;
        list->head = node->next;
    } else {
        Node *previous = list->head;
        for (size_t i = 1; i < position; i++) {
            previous = previous->next;
        }
        node = previous->next;
        previous->next = node->next;
    }
    node->next = NULL;
    list->count--;
    return node;
}


static Status reserve_history(LiverList *list)
{
    if (list->history_count < list->history_capacity) {
        return STATUS_OK;
    }
    size_t new_capacity = START_HISTORY_CAPACITY;
    if (list->history_capacity > 0) {
        if (list->history_capacity > SIZE_MAX / 2 / sizeof(Change)) {
            return STATUS_ERR_SIZE_OVERFLOW;
        }
        new_capacity = list->history_capacity * 2;
    }
    Change *grown = (Change *) realloc(list->history, new_capacity * sizeof(Change));
    if (grown == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    list->history = grown;
    list->history_capacity = new_capacity;
    return STATUS_OK;
}


static void record_change(LiverList *list, const ChangeKind kind, const size_t old_position,
                          const size_t new_position, Node *old_node)
{
    Change change;
    change.kind = kind;
    change.old_position = old_position;
    change.new_position = new_position;
    change.old_node = old_node;
    list->history[list->history_count] = change;
    list->history_count++;
}


static void clear_history(LiverList *list)
{
    for (size_t i = 0; i < list->history_count; i++) {
        free_node(list->history[i].old_node);
        list->history[i].old_node = NULL;
    }
    list->history_count = 0;
}


void list_init(LiverList *list)
{
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->count = 0;
    list->history = NULL;
    list->history_count = 0;
    list->history_capacity = 0;
}


void list_free(LiverList *list)
{
    if (list == NULL) {
        return;
    }
    while (list->head != NULL) {
        Node *next = list->head->next;
        free_node(list->head);
        list->head = next;
    }
    list->count = 0;
    clear_history(list);
    free(list->history);
    list->history = NULL;
    list->history_capacity = 0;
}


Status list_insert_loaded(LiverList *list, const Liver *liver)
{
    if (list == NULL || liver == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Node *node = create_node(liver);
    if (node == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    link_at(list, node, sorted_position(list, liver));
    return STATUS_OK;
}


Status list_add(LiverList *list, const Liver *liver, size_t *out_position)
{
    if (list == NULL || liver == NULL || out_position == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const Status status = reserve_history(list);
    if (status != STATUS_OK) {
        return status;
    }
    Node *node = create_node(liver);
    if (node == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    const size_t position = sorted_position(list, liver);
    link_at(list, node, position);
    record_change(list, CHANGE_ADD, 0, position, NULL);
    *out_position = position;
    return STATUS_OK;
}


Status list_remove(LiverList *list, const size_t position)
{
    if (list == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (position >= list->count) {
        return STATUS_ERR_BAD_NUMBER;
    }
    const Status status = reserve_history(list);
    if (status != STATUS_OK) {
        return status;
    }

    Node *node = unlink_at(list, position);
    record_change(list, CHANGE_REMOVE, position, 0, node);
    return STATUS_OK;
}


Status list_edit(LiverList *list, const size_t position, const Liver *new_data, size_t *out_new_position)
{
    if (list == NULL || new_data == NULL || out_new_position == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (position >= list->count) {
        return STATUS_ERR_BAD_NUMBER;
    }
    const Status status = reserve_history(list);
    if (status != STATUS_OK) {
        return status;
    }
    Node *new_node = create_node(new_data);
    if (new_node == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }

    Node *old_node = unlink_at(list, position);
    size_t new_position = position;
    if (!same_birth(&old_node->data, new_data)) {
        new_position = sorted_position(list, new_data);
    }
    link_at(list, new_node, new_position);
    record_change(list, CHANGE_EDIT, position, new_position, old_node);
    *out_new_position = new_position;
    return STATUS_OK;
}


Status list_get(const LiverList *list, const size_t position, const Liver **out_liver)
{
    if (list == NULL || out_liver == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (position >= list->count) {
        return STATUS_ERR_BAD_NUMBER;
    }
    const Node *current = list->head;
    for (size_t i = 0; i < position; i++) {
        current = current->next;
    }
    *out_liver = &current->data;
    return STATUS_OK;
}


static bool matches(const Liver *liver, const SearchQuery *query, const double epsilon)
{
    if (query->surname != NULL && strcmp(liver->surname, query->surname) != 0) {
        return false;
    }
    if (query->name != NULL && strcmp(liver->name, query->name) != 0) {
        return false;
    }
    if (query->patronymic != NULL && strcmp(liver->patronymic, query->patronymic) != 0) {
        return false;
    }
    if (query->use_birth) {
        int order = 0;
        if (date_compare(&liver->birth, &query->birth, &order) != STATUS_OK || order != 0) {
            return false;
        }
    }
    if (query->gender != '\0' && liver->gender != query->gender) {
        return false;
    }
    if (query->use_income && fabs(liver->income - query->income) > epsilon) {
        return false;
    }
    return true;
}


Status list_search(const LiverList *list, const SearchQuery *query, const double epsilon, size_t *out_positions,
                   const size_t capacity, size_t *out_found)
{
    if (list == NULL || query == NULL || out_positions == NULL || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (isnan(epsilon) || isinf(epsilon) || epsilon <= 0.0) {
        return STATUS_ERR_BAD_EPSILON;
    }
    if (capacity < list->count) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }
    size_t found = 0;
    size_t position = 0;
    for (const Node *current = list->head; current != NULL; current = current->next) {
        if (matches(&current->data, query, epsilon)) {
            out_positions[found] = position;
            found++;
        }
        position++;
    }
    *out_found = found;
    return STATUS_OK;
}


Status list_undo(LiverList *list, size_t *out_undone)
{
    if (list == NULL || out_undone == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const size_t to_undo = list->history_count / 2;
    if (to_undo == 0) {
        return STATUS_ERR_NOTHING_TO_UNDO;
    }

    for (size_t done = 0; done < to_undo; done++) {
        Change *change = &list->history[list->history_count - 1 - done];
        if (change->kind == CHANGE_ADD) {
            free_node(unlink_at(list, change->new_position));
        } else if (change->kind == CHANGE_REMOVE) {
            link_at(list, change->old_node, change->old_position);
            change->old_node = NULL;
        } else {
            free_node(unlink_at(list, change->new_position));
            link_at(list, change->old_node, change->old_position);
            change->old_node = NULL;
        }
    }

    clear_history(list);
    *out_undone = to_undo;
    return STATUS_OK;
}
