#include "tokens.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "transform.h"
#include "same_file.h"

static Status copy_token(FILE *in, FILE *out, const bool need_space,
                         const TokenRule *rule, bool *out_found)
{
    if (in == NULL || out == NULL || rule == NULL || out_found == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_found = false;

    int c = fgetc(in);
    while (c != EOF && isspace(c)) {
        c = fgetc(in);
    }

    if (c == EOF) {
        if (ferror(in)) {
            return STATUS_ERR_READ_FAILED;
        }
        return STATUS_OK;
    }

    *out_found = true;
    if (need_space && fputc(' ', out) == EOF) {
        return STATUS_ERR_WRITE_FAILED;
    }

    char buffer[TRANSFORM_OUTPUT_MAX];
    size_t length = 0;

    while (c != EOF && !isspace(c)) {
        const Status status = transform_char(c, rule, buffer, sizeof buffer, &length);
        if (status != STATUS_OK) {
            return status;
        }
        if (fwrite(buffer, 1, length, out) != length) {
            return STATUS_ERR_WRITE_FAILED;
        }
        c = fgetc(in);
    }

    if (ferror(in)) {
        return STATUS_ERR_READ_FAILED;
    }
    return STATUS_OK;
}

static Status close_output(FILE *out, const Status status)
{
    if (fclose(out) == EOF && status == STATUS_OK) {
        return STATUS_ERR_CLOSE_FAILED;
    }
    return status;
}

Status merge_files(const char *path1, const char *path2, const char *out_path)
{
    if (path1 == NULL || path2 == NULL || out_path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    FILE *in1 = fopen(path1, "r");
    if (in1 == NULL) {
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }
    FILE *in2 = fopen(path2, "r");
    if (in2 == NULL) {
        fclose(in1);
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }
    bool same1 = false;
    bool same2 = false;
    Status check = file_matches_path(in1, out_path, &same1);
    if (check == STATUS_OK) {
        check = file_matches_path(in2, out_path, &same2);
    }
    if (check != STATUS_OK || same1 || same2) {
        fclose(in1);
        fclose(in2);
        if (check == STATUS_OK) {
            check = STATUS_ERR_SAME_FILE;
        }
        return check;
    }

    FILE *out = fopen(out_path, "w");
    if (out == NULL) {
        fclose(in1);
        fclose(in2);
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }

    const TokenRule as_is = {false, 0};
    bool more1 = true;
    bool more2 = true;
    bool need_space = false;
    Status status = STATUS_OK;

    while (status == STATUS_OK && (more1 || more2)) {
        if (more1) {
            status = copy_token(in1, out, need_space, &as_is, &more1);
            if (more1) {
                need_space = true;
            }
        }
        if (status == STATUS_OK && more2) {
            status = copy_token(in2, out, need_space, &as_is, &more2);
            if (more2) {
                need_space = true;
            }
        }
    }

    fclose(in1);
    fclose(in2);
    return close_output(out, status);
}

Status transform_file(const char *in_path, const char *out_path)
{
    if (in_path == NULL || out_path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    FILE *in = fopen(in_path, "r");
    if (in == NULL) {
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }
    bool same = false;
    Status check = file_matches_path(in, out_path, &same);
    if (check != STATUS_OK || same) {
        fclose(in);
        if (check == STATUS_OK) {
            check = STATUS_ERR_SAME_FILE;
        }
        return check;
    }

    FILE *out = fopen(out_path, "w");
    if (out == NULL) {
        fclose(in);
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }

    int number = 1;
    bool found = true;
    bool need_space = false;
    Status status = STATUS_OK;

    while (status == STATUS_OK && found) {
        TokenRule rule;
        status = token_rule(number, &rule);
        if (status != STATUS_OK) {
            break;
        }

        status = copy_token(in, out, need_space, &rule, &found);
        if (found) {
            need_space = true;
        }
        number = number % 10 + 1;
    }

    fclose(in);
    return close_output(out, status);
}
