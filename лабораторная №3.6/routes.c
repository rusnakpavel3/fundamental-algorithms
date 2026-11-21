#include "routes.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>


static Status copy_text(const char *text, char **out_copy)
{
    const size_t length = strlen(text);
    char *copy = (char *) malloc(length + 1);
    if (copy == NULL) {
        return STATUS_ERR_ALLOC_FAILED;
    }
    for (size_t i = 0; i <= length; i++) {
        copy[i] = text[i];
    }
    *out_copy = copy;
    return STATUS_OK;
}


static double distance(const StopPoint *first, const StopPoint *second)
{
    const double dx = second->x - first->x;
    const double dy = second->y - first->y;
    return sqrt(dx * dx + dy * dy);
}


static long long stop_seconds(const StopNode *stop)
{
    return stop->departure.seconds - stop->arrival.seconds;
}


static RouteNode *create_route(const char *vehicle)
{
    RouteNode *route = (RouteNode *) malloc(sizeof(RouteNode));
    if (route == NULL) {
        return NULL;
    }
    route->vehicle = NULL;
    if (copy_text(vehicle, &route->vehicle) != STATUS_OK) {
        free(route);
        return NULL;
    }
    route->first = NULL;
    route->last = NULL;
    route->stop_count = 0;
    route->length = 0.0;
    route->next = NULL;
    return route;
}


static StopNode *create_stop(const Record *record, const StopPoint *point)
{
    StopNode *stop = (StopNode *) malloc(sizeof(StopNode));
    if (stop == NULL) {
        return NULL;
    }
    stop->point = point;
    stop->arrival = record->arrival;
    stop->departure = record->departure;
    stop->mark = record->mark;
    stop->next = NULL;
    return stop;
}


static void append_route(RouteList *list, RouteNode *route)
{
    if (list->last == NULL) {
        list->head = route;
    } else {
        list->last->next = route;
    }
    list->last = route;
    list->count++;
}


static void append_stop(RouteList *list, RouteNode *route, StopNode *stop)
{
    if (route->last == NULL) {
        route->first = stop;
    } else {
        route->length += distance(route->last->point, stop->point);
        route->last->next = stop;
    }
    route->last = stop;
    route->stop_count++;
    list->stop_total++;
}


static void free_route(RouteNode *route)
{
    StopNode *stop = route->first;
    while (stop != NULL) {
        StopNode *next = stop->next;
        free(stop);
        stop = next;
    }
    free(route->vehicle);
    free(route);
}


void routes_init(RouteList *list)
{
    if (list == NULL) {
        return;
    }
    list->head = NULL;
    list->last = NULL;
    list->count = 0;
    list->stop_total = 0;
}


void routes_free(RouteList *list)
{
    if (list == NULL) {
        return;
    }
    while (list->head != NULL) {
        RouteNode *next = list->head->next;
        free_route(list->head);
        list->head = next;
    }
    routes_init(list);
}


Status routes_build(RouteList *list, const Record *records, const size_t count, const StopPoint *points,
                    const size_t point_count, size_t *out_bad_record)
{
    if (list == NULL || out_bad_record == NULL || (records == NULL && count > 0)
        || (points == NULL && point_count > 0)) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_bad_record = 0;
    RouteNode *open = NULL;
    size_t open_start = 0;

    for (size_t i = 0; i < count; i++) {
        const Record *record = &records[i];
        if (record->point >= point_count) {
            *out_bad_record = i;
            return STATUS_ERR_BAD_POINT;
        }
        if (open != NULL && strcmp(open->vehicle, record->vehicle) != 0) {
            *out_bad_record = open_start;
            return STATUS_ERR_ROUTE_NOT_FINISHED;
        }

        if (record->mark == MARK_START) {
            if (open != NULL) {
                *out_bad_record = open_start;
                return STATUS_ERR_ROUTE_NOT_FINISHED;
            }
            open = create_route(record->vehicle);
            if (open == NULL) {
                *out_bad_record = i;
                return STATUS_ERR_ALLOC_FAILED;
            }
            append_route(list, open);
            open_start = i;
        } else if (open == NULL) {
            *out_bad_record = i;
            return STATUS_ERR_ROUTE_NOT_STARTED;
        }

        StopNode *stop = create_stop(record, &points[record->point]);
        if (stop == NULL) {
            *out_bad_record = i;
            return STATUS_ERR_ALLOC_FAILED;
        }
        append_stop(list, open, stop);
        if (record->mark == MARK_END) {
            open = NULL;
        }
    }

    if (open != NULL) {
        *out_bad_record = open_start;
        return STATUS_ERR_ROUTE_NOT_FINISHED;
    }
    return STATUS_OK;
}


Status routes_vehicle_stats(const RouteList *list, VehicleStats *out_stats, const size_t capacity, size_t *out_count)
{
    if (list == NULL || out_stats == NULL || out_count == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    size_t count = 0;
    for (const RouteNode *route = list->head; route != NULL; route = route->next) {
        if (count == 0 || strcmp(out_stats[count - 1].vehicle, route->vehicle) != 0) {
            if (count == capacity) {
                return STATUS_ERR_BUFFER_TOO_SMALL;
            }
            out_stats[count].vehicle = route->vehicle;
            out_stats[count].routes = 0;
            out_stats[count].path = 0.0;
            out_stats[count].idle = 0;
            count++;
        }
        VehicleStats *current = &out_stats[count - 1];
        current->routes++;
        current->path += route->length;
        for (const StopNode *stop = route->first; stop != NULL; stop = stop->next) {
            current->idle += stop_seconds(stop);
        }
    }
    *out_count = count;
    return STATUS_OK;
}


static Status check_epsilon(const double epsilon)
{
    if (isnan(epsilon) || isinf(epsilon) || epsilon <= 0.0) {
        return STATUS_ERR_BAD_EPSILON;
    }
    return STATUS_OK;
}


static bool is_better(const double value, const double best, const bool want_max, const double epsilon)
{
    if (want_max) {
        return value > best + epsilon;
    }
    return value < best - epsilon;
}


static bool is_tie(const double value, const double best, const double epsilon)
{
    return fabs(value - best) <= epsilon;
}


static Status check_stats_args(const VehicleStats *stats, const size_t count, const size_t *out_indices,
                               const size_t capacity, const size_t *out_found)
{
    if ((stats == NULL && count > 0) || (out_indices == NULL && capacity > 0) || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (count == 0) {
        return STATUS_ERR_NO_DATA;
    }
    if (capacity < count) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }
    return STATUS_OK;
}


Status stats_find_routes(const VehicleStats *stats, const size_t count, const bool want_max, size_t *out_indices,
                         const size_t capacity, size_t *out_found)
{
    const Status status = check_stats_args(stats, count, out_indices, capacity, out_found);
    if (status != STATUS_OK) {
        return status;
    }

    size_t best = stats[0].routes;
    for (size_t i = 1; i < count; i++) {
        if (want_max && stats[i].routes > best) {
            best = stats[i].routes;
        }
        if (!want_max && stats[i].routes < best) {
            best = stats[i].routes;
        }
    }

    size_t found = 0;
    for (size_t i = 0; i < count; i++) {
        if (stats[i].routes == best) {
            out_indices[found] = i;
            found++;
        }
    }
    *out_found = found;
    return STATUS_OK;
}


Status stats_find_path(const VehicleStats *stats, const size_t count, const bool want_max, const double epsilon,
                       size_t *out_indices, const size_t capacity, size_t *out_found)
{
    Status status = check_epsilon(epsilon);
    if (status != STATUS_OK) {
        return status;
    }
    status = check_stats_args(stats, count, out_indices, capacity, out_found);
    if (status != STATUS_OK) {
        return status;
    }

    double best = stats[0].path;
    for (size_t i = 1; i < count; i++) {
        if (is_better(stats[i].path, best, want_max, epsilon)) {
            best = stats[i].path;
        }
    }

    size_t found = 0;
    for (size_t i = 0; i < count; i++) {
        if (is_tie(stats[i].path, best, epsilon)) {
            out_indices[found] = i;
            found++;
        }
    }
    *out_found = found;
    return STATUS_OK;
}


Status stats_find_idle(const VehicleStats *stats, const size_t count, size_t *out_indices, const size_t capacity,
                       size_t *out_found)
{
    const Status status = check_stats_args(stats, count, out_indices, capacity, out_found);
    if (status != STATUS_OK) {
        return status;
    }

    long long best = stats[0].idle;
    for (size_t i = 1; i < count; i++) {
        if (stats[i].idle > best) {
            best = stats[i].idle;
        }
    }

    size_t found = 0;
    for (size_t i = 0; i < count; i++) {
        if (stats[i].idle == best) {
            out_indices[found] = i;
            found++;
        }
    }
    *out_found = found;
    return STATUS_OK;
}


Status routes_find_routes(const RouteList *list, const bool longest, const double epsilon, RouteHit *out_hits,
                          const size_t capacity, size_t *out_found)
{
    if (list == NULL || (out_hits == NULL && capacity > 0) || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    const Status status = check_epsilon(epsilon);
    if (status != STATUS_OK) {
        return status;
    }
    if (list->count == 0) {
        return STATUS_ERR_NO_DATA;
    }
    if (capacity < list->count) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    double best = list->head->length;
    for (const RouteNode *route = list->head; route != NULL; route = route->next) {
        if (is_better(route->length, best, longest, epsilon)) {
            best = route->length;
        }
    }

    size_t found = 0;
    for (const RouteNode *route = list->head; route != NULL; route = route->next) {
        if (is_tie(route->length, best, epsilon)) {
            out_hits[found].route = route;
            out_hits[found].stop = NULL;
            found++;
        }
    }
    *out_found = found;
    return STATUS_OK;
}


Status routes_find_stops(const RouteList *list, const bool longest, RouteHit *out_hits, const size_t capacity,
                         size_t *out_found)
{
    if (list == NULL || (out_hits == NULL && capacity > 0) || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    if (list->count == 0) {
        return STATUS_ERR_NO_DATA;
    }
    if (capacity < list->stop_total) {
        return STATUS_ERR_BUFFER_TOO_SMALL;
    }

    long long best = stop_seconds(list->head->first);
    for (const RouteNode *route = list->head; route != NULL; route = route->next) {
        for (const StopNode *stop = route->first; stop != NULL; stop = stop->next) {
            const long long value = stop_seconds(stop);
            if (longest && value > best) {
                best = value;
            }
            if (!longest && value < best) {
                best = value;
            }
        }
    }

    size_t found = 0;
    for (const RouteNode *route = list->head; route != NULL; route = route->next) {
        for (const StopNode *stop = route->first; stop != NULL; stop = stop->next) {
            if (stop_seconds(stop) == best) {
                out_hits[found].route = route;
                out_hits[found].stop = stop;
                found++;
            }
        }
    }
    *out_found = found;
    return STATUS_OK;
}
