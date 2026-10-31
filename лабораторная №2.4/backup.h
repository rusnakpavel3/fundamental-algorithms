#ifndef BACKUP_H
#define BACKUP_H

#include <stddef.h>

#include "status.h"

typedef struct {
    char *name;
    long long size;
    long long importance;
} FileInfo;

typedef struct {
    const FileInfo **files;
    size_t count;
    long long total_size;
    long long total_importance;
} BackupResult;

Status select_backup(const FileInfo *files, const size_t count, const long long capacity, BackupResult *out_result);
void free_backup_result(BackupResult *result);

#endif
