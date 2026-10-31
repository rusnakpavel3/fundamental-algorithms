#ifndef SOURCE_H
#define SOURCE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "status.h"

typedef struct {
    FILE *file;
    const char *text;
    size_t position;
    size_t consumed;
} Source;

void source_from_file(Source *source, FILE *file);
void source_from_text(Source *source, const char *text);
int source_get(Source *source);
void source_unget(Source *source, const int c);
int source_skip_spaces(Source *source);
bool source_has_error(const Source *source);
Status source_end_status(const Source *source);
Status source_scan(Source *source, const char *spec, int *consumed, ...);

#endif
