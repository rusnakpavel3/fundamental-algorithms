#include "file_io.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "parser.h"
#include "same_file.h"

static Status read_number(FILE *in, NumberParser *parser, bool *out_found)
{
    if (in == NULL || parser == NULL || out_found == NULL) {
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
    Status status = parser_reset(parser);

    while (status == STATUS_OK && c != EOF && !isspace(c)) {
        status = parser_feed(parser, c);
        c = fgetc(in);
    }

    if (status == STATUS_OK && ferror(in)) {
        return STATUS_ERR_READ_FAILED;
    }
    return status;
}

static Status write_result(FILE *out, const NumberResult *result)
{
    const char *sign = "";
    if (result->negative) {
        sign = "-";
    }

    if (fprintf(out, "%s %d %s%llu\n", result->text, result->base, sign, result->value) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    return STATUS_OK;
}

Status convert_file(const char *in_path, const char *out_path,
                    unsigned long long *out_bad_number)
{
    if (in_path == NULL || out_path == NULL || out_bad_number == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    *out_bad_number = 0;

    FILE *in = fopen(in_path, "r");
    if (in == NULL) {
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }
    bool same = false;
    Status status = file_matches_path(in, out_path, &same);
    if (status != STATUS_OK || same) {
        fclose(in);
        if (status == STATUS_OK) {
            status = STATUS_ERR_SAME_FILE;
        }
        return status;
    }

    FILE *out = fopen(out_path, "w");
    if (out == NULL) {
        fclose(in);
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }

    unsigned long long number_index = 0;
    bool found = true;

    while (status == STATUS_OK) {
        NumberParser parser;
        status = read_number(in, &parser, &found);
        if (found) {
            number_index++;
        }
        if (status != STATUS_OK || !found) {
            break;
        }

        NumberResult result;
        status = parser_finish(&parser, &result);
        if (status == STATUS_OK) {
            status = write_result(out, &result);
        }
    }

    if (status == STATUS_ERR_NUMBER_INVALID || status == STATUS_ERR_NUMBER_TOO_BIG) {
        *out_bad_number = number_index;
    }

    fclose(in);
    if (fclose(out) == EOF && status == STATUS_OK) {
        status = STATUS_ERR_CLOSE_FAILED;
    }
    return status;
}
