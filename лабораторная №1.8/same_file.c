#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "same_file.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#ifdef _WIN32

static Status get_identity(FILE *file, BY_HANDLE_FILE_INFORMATION *out_info)
{
    const intptr_t raw = _get_osfhandle(_fileno(file));
    if (raw == -1) {
        return STATUS_ERR_FILE_INFO_FAILED;
    }
    if (!GetFileInformationByHandle((HANDLE) raw, out_info)) {
        return STATUS_ERR_FILE_INFO_FAILED;
    }
    return STATUS_OK;
}

static Status same_identity(FILE *first, FILE *second, bool *out_same)
{
    BY_HANDLE_FILE_INFORMATION a;
    BY_HANDLE_FILE_INFORMATION b;

    Status status = get_identity(first, &a);
    if (status != STATUS_OK) {
        return status;
    }
    status = get_identity(second, &b);
    if (status != STATUS_OK) {
        return status;
    }

    *out_same = a.dwVolumeSerialNumber == b.dwVolumeSerialNumber
             && a.nFileIndexHigh == b.nFileIndexHigh
             && a.nFileIndexLow == b.nFileIndexLow;
    return STATUS_OK;
}

#else

static Status same_identity(FILE *first, FILE *second, bool *out_same)
{
    struct stat a;
    struct stat b;

    if (fstat(fileno(first), &a) != 0 || fstat(fileno(second), &b) != 0) {
        return STATUS_ERR_FILE_INFO_FAILED;
    }

    *out_same = a.st_dev == b.st_dev && a.st_ino == b.st_ino;
    return STATUS_OK;
}

#endif

Status file_matches_path(FILE *opened, const char *path, bool *out_same)
{
    if (opened == NULL || path == NULL || out_same == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    *out_same = false;

    FILE *other = fopen(path, "r");
    if (other == NULL) {
        return STATUS_OK;
    }

    const Status status = same_identity(opened, other, out_same);
    fclose(other);
    return status;
}
