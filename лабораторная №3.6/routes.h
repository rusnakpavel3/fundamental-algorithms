#ifndef ROUTES_H
#define ROUTES_H

#include <stdbool.h>
#include <stddef.h>

#include "datetime.h"
#include "record.h"
#include "status.h"

typedef struct StopNode {
    const StopPoint *point;
    DateTime arrival;
    DateTime departure;
    StopMark mark;
    struct StopNode *next;
} StopNode;

typedef struct RouteNode {
    char *vehicle;
    StopNode *first;
    StopNode *last;
    size_t stop_count;
    double length;
    struct RouteNode *next;
} RouteNode;

typedef struct {
    RouteNode *head;
    RouteNode *last;
    size_t count;
    size_t stop_total;
} RouteList;

typedef struct {
    const char *vehicle;
    size_t routes;
    double path;
    long long idle;
} VehicleStats;

typedef struct {
    const RouteNode *route;
    const StopNode *stop;
} RouteHit;

void routes_init(RouteList *list);
void routes_free(RouteList *list);
Status routes_build(RouteList *list, const Record *records, size_t count, const StopPoint *points,
                    size_t point_count, size_t *out_bad_record);
Status routes_vehicle_stats(const RouteList *list, VehicleStats *out_stats, size_t capacity, size_t *out_count);

Status stats_find_routes(const VehicleStats *stats, size_t count, bool want_max, size_t *out_indices,
                         size_t capacity, size_t *out_found);
Status stats_find_path(const VehicleStats *stats, size_t count, bool want_max, double epsilon,
                       size_t *out_indices, size_t capacity, size_t *out_found);
Status stats_find_idle(const VehicleStats *stats, size_t count, size_t *out_indices, size_t capacity,
                       size_t *out_found);

Status routes_find_routes(const RouteList *list, bool longest, double epsilon, RouteHit *out_hits,
                          size_t capacity, size_t *out_found);
Status routes_find_stops(const RouteList *list, bool longest, RouteHit *out_hits, size_t capacity,
                         size_t *out_found);

#endif
