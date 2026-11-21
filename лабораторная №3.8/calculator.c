#include "calculator.h"

#include <stddef.h>

#include "poly_parse.h"


void outcome_init(Outcome *outcome)
{
    if (outcome == NULL) {
        return;
    }
    poly_init(&outcome->left);
    poly_init(&outcome->right);
    poly_init(&outcome->result);
    outcome->has_right = false;
    outcome->point = 0;
    outcome->has_point = false;
    outcome->value = 0;
    outcome->is_value = false;
}


void outcome_free(Outcome *outcome)
{
    if (outcome == NULL) {
        return;
    }
    poly_free(&outcome->left);
    poly_free(&outcome->right);
    poly_free(&outcome->result);
}


static Status load_operands(const Command *command, const Polynomial *accumulator, Outcome *outcome)
{
    size_t next = 0;
    Status status = STATUS_OK;
    if (command->arg_count == full_arg_count(command->kind)) {
        status = parse_polynomial(command->args[0], &outcome->left);
        next = 1;
    } else {
        status = poly_copy(accumulator, &outcome->left);
    }
    if (status != STATUS_OK || command->kind == COMMAND_DIFF) {
        return status;
    }
    if (command->kind == COMMAND_EVAL) {
        status = parse_integer(command->args[next], &outcome->point);
        outcome->has_point = status == STATUS_OK;
        return status;
    }
    status = parse_polynomial(command->args[next], &outcome->right);
    outcome->has_right = status == STATUS_OK;
    return status;
}


static Status compute(const CommandKind kind, Outcome *outcome)
{
    const Polynomial *left = &outcome->left;
    const Polynomial *right = &outcome->right;
    Polynomial unused;
    poly_init(&unused);
    Status status = STATUS_OK;
    switch (kind) {
        case COMMAND_ADD:
            status = poly_add(left, right, &outcome->result);
            break;
        case COMMAND_SUB:
            status = poly_sub(left, right, &outcome->result);
            break;
        case COMMAND_MULT:
            status = poly_mul(left, right, &outcome->result);
            break;
        case COMMAND_DIV:
            status = poly_divmod(left, right, &outcome->result, &unused);
            if (status == STATUS_OK) {
                poly_free(&unused);
            }
            break;
        case COMMAND_MOD:
            status = poly_divmod(left, right, &unused, &outcome->result);
            if (status == STATUS_OK) {
                poly_free(&unused);
            }
            break;
        case COMMAND_EVAL:
            status = poly_eval(left, outcome->point, &outcome->value);
            outcome->is_value = status == STATUS_OK;
            break;
        case COMMAND_DIFF:
            status = poly_diff(left, &outcome->result);
            break;
        case COMMAND_CMPS:
            status = poly_compose(left, right, &outcome->result);
            break;
    }
    return status;
}


Status run_command(const Command *command, Polynomial *accumulator, Outcome *out_outcome)
{
    if (command == NULL || accumulator == NULL || out_outcome == NULL) {
        return STATUS_ERR_NULL_ARGUMENT;
    }
    Outcome outcome;
    outcome_init(&outcome);
    Status status = load_operands(command, accumulator, &outcome);
    if (status == STATUS_OK) {
        status = compute(command->kind, &outcome);
    }
    if (status == STATUS_OK && command->kind != COMMAND_EVAL) {
        Polynomial copy;
        status = poly_copy(&outcome.result, &copy);
        if (status == STATUS_OK) {
            poly_free(accumulator);
            *accumulator = copy;
        }
    }
    if (status != STATUS_OK) {
        outcome_free(&outcome);
        return status;
    }
    *out_outcome = outcome;
    return STATUS_OK;
}
