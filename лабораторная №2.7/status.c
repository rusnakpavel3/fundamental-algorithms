#include "status.h"

const char *status_message(const Status status)
{
    switch (status) {
        case STATUS_OK:
            return "успешно";
        case STATUS_ERR_NULL_ARGUMENT:
            return "передан пустой указатель";
        case STATUS_ERR_EPSILON_INVALID:
            return "эпсилон должен быть конечным числом из интервала (0; 1)";
        case STATUS_ERR_DIMENSION_INVALID:
            return "размерность пространства должна быть больше нуля";
        case STATUS_ERR_NO_VECTORS:
            return "не передано ни одного вектора";
        case STATUS_ERR_DIMENSION_MISMATCH:
            return "размерность вектора (или матрицы) не совпадает с размерностью пространства";
        case STATUS_ERR_COORDINATE_NOT_FINITE:
            return "координата вектора не является конечным числом";
        case STATUS_ERR_NORM_FUNCTION_NULL:
            return "вместо функции нормы передан NULL";
        case STATUS_ERR_NORM_INVALID:
            return "функция нормы вернула отрицательное или не конечное значение";
        case STATUS_ERR_P_INVALID:
            return "параметр p должен быть конечным числом не меньше 1";
        case STATUS_ERR_MATRIX_INVALID:
            return "элементы матрицы должны быть конечными числами (и не слишком большими)";
        case STATUS_ERR_MATRIX_NOT_POSITIVE_DEFINITE:
            return "матрица не положительно определённая (или не проверена через matrix_norm_init)";
        case STATUS_ERR_NORM_OVERFLOW:
            return "значение нормы не помещается в double";
        case STATUS_ERR_SIZE_OVERFLOW:
            return "слишком много данных: размер не помещается в size_t";
        case STATUS_ERR_ALLOC_FAILED:
            return "не удалось выделить память";
    }
    return "неизвестный статус";
}
