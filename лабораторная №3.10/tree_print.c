#include "tree_print.h"

#include <stdbool.h>
#include <string.h>

#define BRANCH "├── "
#define LAST_BRANCH "└── "
#define LINE_PREFIX "│   "
#define EMPTY_PREFIX "    "
#define PREFIX_STEP_SIZE (sizeof(LINE_PREFIX) - 1)


static Status write_children(FILE *file, const Node *parent, char *prefix, const size_t length)
{
    for (const Node *child = parent->first_child; child != NULL; child = child->next_sibling) {
        const bool last = child->next_sibling == NULL;
        if (fprintf(file, "%s%s%s\n", prefix, last ? LAST_BRANCH : BRANCH, child->name) < 0) {
            return STATUS_ERR_WRITE_FAILED;
        }

        if (child->first_child != NULL) {
            const char *step = last ? EMPTY_PREFIX : LINE_PREFIX;
            const size_t step_length = strlen(step);
            memcpy(prefix + length, step, step_length + 1);

            const Status status = write_children(file, child, prefix, length + step_length);
            prefix[length] = '\0';
            if (status != STATUS_OK) {
                return status;
            }
        }
    }
    return STATUS_OK;
}


Status tree_write(FILE *file, const Node *root)
{
    if (file == NULL || root == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }

    char prefix[MAX_TREE_DEPTH * PREFIX_STEP_SIZE + 1];
    prefix[0] = '\0';

    if (fprintf(file, "%s\n", root->name) < 0) {
        return STATUS_ERR_WRITE_FAILED;
    }
    return write_children(file, root, prefix, 0);
}
