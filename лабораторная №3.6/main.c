#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "console.h"
#include "datetime.h"
#include "record.h"
#include "routes.h"
#include "status.h"
#include "stop_file.h"

#define LINE_SIZE 256
#define DISTANCE_EPSILON 0.000001

typedef struct {
    const RouteList *list;
    const VehicleStats *stats;
    size_t stats_count;
    size_t *indices;
    size_t indices_capacity;
    RouteHit *hits;
    size_t hits_capacity;
} Storage;


static void print_menu(void)
{
    printf("Команды:\n");
    printf("  1 - ТС, проехавшее больше всех рейсов\n");
    printf("  2 - ТС, проехавшее меньше всех рейсов\n");
    printf("  3 - ТС, проехавшее самый длинный путь\n");
    printf("  4 - ТС, проехавшее самый короткий путь\n");
    printf("  5 - ТС с самым длинным маршрутом (рейсом)\n");
    printf("  6 - ТС с самым коротким маршрутом (рейсом)\n");
    printf("  7 - ТС с самой долгой остановкой\n");
    printf("  8 - ТС с самой короткой остановкой\n");
    printf("  9 - ТС с самым большим временем простоя\n");
    printf(" 10 - показать все рейсы\n");
    printf("  0 - выход\n");
}


static void print_datetime(const DateTime *datetime)
{
    printf("%02d.%02d.%04d %02d:%02d:%02d", datetime->day, datetime->month, datetime->year, datetime->hour,
           datetime->minute, datetime->second);
}


static void print_duration(const long long seconds)
{
    printf("%lld с (%lld:%02lld:%02lld)", seconds, seconds / 3600, seconds / 60 % 60, seconds % 60);
}


static const char *mark_name(const StopMark mark)
{
    switch (mark) {
        case MARK_START:
            return "start";
        case MARK_MIDDLE:
            return "middle";
        case MARK_END:
            return "end";
    }
    return "?";
}


static void print_stop(const StopNode *stop)
{
    printf("      %s (%.6g; %.6g): ", stop->point->name, stop->point->x, stop->point->y);
    print_datetime(&stop->arrival);
    printf(" - ");
    print_datetime(&stop->departure);
    printf(", %s\n", mark_name(stop->mark));
}


static void print_route(const RouteNode *route)
{
    printf("  ТС %s: остановок %zu, длина %.3f\n", route->vehicle, route->stop_count, route->length);
    for (const StopNode *stop = route->first; stop != NULL; stop = stop->next) {
        print_stop(stop);
    }
}


static void print_all_routes(const RouteList *list)
{
    printf("Рейсов: %zu\n", list->count);
    for (const RouteNode *route = list->head; route != NULL; route = route->next) {
        print_route(route);
    }
}


static void print_status_error(const Status status)
{
    if (status == STATUS_ERR_NO_DATA) {
        printf("%s\n", status_message(status));
    } else {
        printf("Ошибка: %s\n", status_message(status));
    }
}


static void show_vehicles(const char *title, const Status status, const Storage *storage, const size_t found)
{
    printf("%s:\n", title);
    if (status != STATUS_OK) {
        print_status_error(status);
        return;
    }
    for (size_t i = 0; i < found; i++) {
        const VehicleStats *item = &storage->stats[storage->indices[i]];
        printf("  ТС %s: рейсов %zu, путь %.3f, простой ", item->vehicle, item->routes, item->path);
        print_duration(item->idle);
        printf("\n");
    }
}


static void show_routes(const char *title, const Status status, const Storage *storage, const size_t found)
{
    printf("%s:\n", title);
    if (status != STATUS_OK) {
        print_status_error(status);
        return;
    }
    for (size_t i = 0; i < found; i++) {
        print_route(storage->hits[i].route);
    }
}


static void show_stops(const char *title, const Status status, const Storage *storage, const size_t found)
{
    printf("%s:\n", title);
    if (status != STATUS_OK) {
        print_status_error(status);
        return;
    }
    for (size_t i = 0; i < found; i++) {
        const StopNode *stop = storage->hits[i].stop;
        printf("  ТС %s: остановка ", storage->hits[i].route->vehicle);
        print_duration(stop->departure.seconds - stop->arrival.seconds);
        printf("\n");
        print_stop(stop);
    }
}


static void run_command(const char *command, const Storage *storage)
{
    const VehicleStats *stats = storage->stats;
    const size_t count = storage->stats_count;
    size_t found = 0;
    Status status = STATUS_OK;

    if (strcmp(command, "1") == 0) {
        status = stats_find_routes(stats, count, true, storage->indices, storage->indices_capacity, &found);
        show_vehicles("Больше всех рейсов", status, storage, found);
    } else if (strcmp(command, "2") == 0) {
        status = stats_find_routes(stats, count, false, storage->indices, storage->indices_capacity, &found);
        show_vehicles("Меньше всех рейсов", status, storage, found);
    } else if (strcmp(command, "3") == 0) {
        status = stats_find_path(stats, count, true, DISTANCE_EPSILON, storage->indices,
                                 storage->indices_capacity, &found);
        show_vehicles("Самый длинный путь", status, storage, found);
    } else if (strcmp(command, "4") == 0) {
        status = stats_find_path(stats, count, false, DISTANCE_EPSILON, storage->indices,
                                 storage->indices_capacity, &found);
        show_vehicles("Самый короткий путь", status, storage, found);
    } else if (strcmp(command, "5") == 0) {
        status = routes_find_routes(storage->list, true, DISTANCE_EPSILON, storage->hits, storage->hits_capacity,
                                    &found);
        show_routes("Самый длинный маршрут (рейс)", status, storage, found);
    } else if (strcmp(command, "6") == 0) {
        status = routes_find_routes(storage->list, false, DISTANCE_EPSILON, storage->hits, storage->hits_capacity,
                                    &found);
        show_routes("Самый короткий маршрут (рейс)", status, storage, found);
    } else if (strcmp(command, "7") == 0) {
        status = routes_find_stops(storage->list, true, storage->hits, storage->hits_capacity, &found);
        show_stops("Самая долгая остановка", status, storage, found);
    } else if (strcmp(command, "8") == 0) {
        status = routes_find_stops(storage->list, false, storage->hits, storage->hits_capacity, &found);
        show_stops("Самая короткая остановка", status, storage, found);
    } else if (strcmp(command, "9") == 0) {
        status = stats_find_idle(stats, count, storage->indices, storage->indices_capacity, &found);
        show_vehicles("Самый большой простой", status, storage, found);
    } else if (strcmp(command, "10") == 0) {
        print_all_routes(storage->list);
    } else {
        printf("Неизвестная команда: введите число от 0 до 10\n");
    }
}


static void run_dialog(const Storage *storage)
{
    char command[LINE_SIZE];
    print_menu();
    while (true) {
        printf("> ");
        fflush(stdout);

        bool end = false;
        const Status status = console_read_line(stdin, command, sizeof command, &end);
        if (status == STATUS_ERR_LINE_TOO_LONG) {
            printf("Ошибка: %s\n", status_message(status));
            continue;
        }
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            return;
        }
        if (end) {
            printf("\nВвод закончился - выход\n");
            return;
        }
        if (strcmp(command, "0") == 0) {
            return;
        }
        run_command(command, storage);
    }
}


static void print_file_error(const Status status, const char *path, const size_t line)
{
    switch (status) {
        case STATUS_OK:
            break;
        case STATUS_ERR_NUL_BYTE:
        case STATUS_ERR_BAD_COORDINATES:
        case STATUS_ERR_FIELD_COUNT:
        case STATUS_ERR_BAD_DATETIME_FORMAT:
        case STATUS_ERR_BAD_DATETIME:
        case STATUS_ERR_DEPARTURE_BEFORE_ARRIVAL:
        case STATUS_ERR_BAD_MARK:
        case STATUS_ERR_ROUTE_NOT_STARTED:
        case STATUS_ERR_ROUTE_NOT_FINISHED:
            printf("Ошибка в файле %s, строка %zu: %s\n", path, line, status_message(status));
            break;
        case STATUS_ERR_NULL_ARGUMENT:
        case STATUS_ERR_NO_FILES:
        case STATUS_ERR_FILE_OPEN_FAILED:
        case STATUS_ERR_READ_FAILED:
        case STATUS_ERR_ALLOC_FAILED:
        case STATUS_ERR_SIZE_OVERFLOW:
        case STATUS_ERR_NO_COORDINATES:
        case STATUS_ERR_BAD_POINT:
        case STATUS_ERR_BAD_EPSILON:
        case STATUS_ERR_NO_DATA:
        case STATUS_ERR_BUFFER_TOO_SMALL:
        case STATUS_ERR_LINE_TOO_LONG:
            printf("Ошибка: %s: %s\n", status_message(status), path);
            break;
    }
}


static bool load_files(const size_t file_count, char *paths[], StopPoint *points, RecordArray *records)
{
    for (size_t i = 0; i < file_count; i++) {
        points[i].name = paths[i];
        FILE *file = fopen(paths[i], "r");
        if (file == NULL) {
            print_file_error(STATUS_ERR_FILE_OPEN_FAILED, paths[i], 0);
            return false;
        }
        size_t line = 0;
        const Status status = read_stop_file(file, i, &points[i], records, &line);
        fclose(file);
        if (status != STATUS_OK) {
            print_file_error(status, paths[i], line);
            return false;
        }
    }
    return true;
}


static bool build_routes(const RecordArray *records, const StopPoint *points, const size_t point_count,
                         RouteList *list)
{
    size_t bad = 0;
    const Status status = routes_build(list, records->items, records->count, points, point_count, &bad);
    if (status == STATUS_OK) {
        return true;
    }
    if (bad < records->count && records->items[bad].point < point_count) {
        const Record *record = &records->items[bad];
        printf("ТС %s: ", record->vehicle);
        print_file_error(status, points[record->point].name, record->line);
    } else {
        printf("Ошибка: %s\n", status_message(status));
    }
    return false;
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc < 2) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_NO_FILES));
        printf("Пример: lab3_task6 A.txt B.txt C.txt\n");
        return 1;
    }
    const size_t file_count = (size_t) argc - 1;
    StopPoint *points = (StopPoint *) calloc(file_count, sizeof(StopPoint));
    if (points == NULL) {
        printf("Ошибка: %s\n", status_message(STATUS_ERR_ALLOC_FAILED));
        return 1;
    }

    RecordArray records;
    records_init(&records);
    RouteList list;
    routes_init(&list);
    VehicleStats *stats = NULL;
    size_t stats_count = 0;
    size_t *indices = NULL;
    RouteHit *hits = NULL;

    bool ok = load_files(file_count, &argv[1], points, &records);
    if (ok) {
        const Status status = records_sort(&records);
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            ok = false;
        }
    }
    if (ok) {
        ok = build_routes(&records, points, file_count, &list);
    }
    records_free(&records);
    if (ok && list.count > 0) {
        stats = (VehicleStats *) malloc(list.count * sizeof(VehicleStats));
        indices = (size_t *) malloc(list.count * sizeof(size_t));
        hits = (RouteHit *) malloc(list.stop_total * sizeof(RouteHit));
        if (stats == NULL || indices == NULL || hits == NULL) {
            printf("Ошибка: %s\n", status_message(STATUS_ERR_ALLOC_FAILED));
            ok = false;
        }
    }
    if (ok && list.count > 0) {
        const Status status = routes_vehicle_stats(&list, stats, list.count, &stats_count);
        if (status != STATUS_OK) {
            printf("Ошибка: %s\n", status_message(status));
            ok = false;
        }
    }

    if (ok) {
        printf("Остановок: %zu, рейсов: %zu, транспортных средств: %zu\n", file_count, list.count, stats_count);
        Storage storage;
        storage.list = &list;
        storage.stats = stats;
        storage.stats_count = stats_count;
        storage.indices = indices;
        storage.indices_capacity = list.count;
        storage.hits = hits;
        storage.hits_capacity = list.stop_total;
        run_dialog(&storage);
    }
    free(hits);
    free(indices);
    free(stats);
    routes_free(&list);
    free(points);
    if (!ok) {
        return 1;
    }
    return 0;
}
