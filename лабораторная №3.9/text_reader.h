#ifndef TEXT_READER_H
#define TEXT_READER_H

#include <stdbool.h>

#include "status.h"
#include "word_tree.h"

#define CHAR_VALUES 256

Status parse_separator(const char *argument, unsigned char *out_separator);
Status read_words(const char *path, const bool separator[CHAR_VALUES], WordNode **out_root, unsigned long long *out_total);

#endif
