#include "file_io.h"

#include <stdbool.h>
#include <stdio.h>

#include "same_file.h"

static Status write_buffer(FILE *out, const char *buffer, const size_t length)
{
    if (fwrite(buffer, 1, length, out) != length) {
        return STATUS_ERR_WRITE_FAILED;
    }
    return STATUS_OK;
}

static Status pump(FILE *in, FILE *out, Processor *processor)
{
    char buffer[PROCESSOR_OUTPUT_MAX];
    size_t length = 0;
    Status status;
    int c;

    while ((c = fgetc(in)) != EOF) {
        status = processor_feed(processor, c, buffer, sizeof buffer, &length);
        if (status != STATUS_OK) {
            return status;
        }
        status = write_buffer(out, buffer, length);
        if (status != STATUS_OK) {
            return status;
        }
    }

    if (ferror(in)) {
        return STATUS_ERR_READ_FAILED;
    }

    status = processor_finish(processor, buffer, sizeof buffer, &length);
    if (status != STATUS_OK) {
        return status;
    }
    return write_buffer(out, buffer, length);
}

Status process_file(const char *input_path, const char *output_path, const Action action)
{
    if (input_path == NULL || output_path == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    Processor processor;
    Status status = processor_init(&processor, action);
    if (status != STATUS_OK) {
        return status;
    }

    FILE *in = fopen(input_path, "r");
    if (in == NULL) {
        return STATUS_ERR_INPUT_OPEN_FAILED;
    }

    bool same = false;
    status = file_matches_path(in, output_path, &same);
    if (status != STATUS_OK || same) {
        fclose(in);
        if (status == STATUS_OK) {
            status = STATUS_ERR_SAME_FILE;
        }
        return status;
    }

    FILE *out = fopen(output_path, "w");
    if (out == NULL) {
        fclose(in);
        return STATUS_ERR_OUTPUT_OPEN_FAILED;
    }

    status = pump(in, out, &processor);

    fclose(in);
    if (fclose(out) == EOF && status == STATUS_OK) {
        status = STATUS_ERR_CLOSE_FAILED;
    }
    return status;
}
