#ifdef _WIN32
#include <windows.h>
#endif

#include <math.h>
#include <stdio.h>

#include "longest.h"
#include "norms.h"
#include "status.h"

static void print_vector(const Vector *vector)
{
    printf("(");
    for (size_t j = 0; j < vector->dimension; j++) {
        printf("%s%.15g", j > 0 ? ", " : "", vector->coordinates[j]);
    }
    printf(")");
}

static void finish(const char *label, const Status status, LongestResult **results,
                   const char *const *names, const size_t norm_count)
{
    printf("%s\n", label);

    if (status != STATUS_OK) {
        printf("  ошибка: %s\n", status_message(status));
    } else if (*results == NULL) {
        printf("  норм не передано - результатов нет\n");
    } else {
        for (size_t k = 0; k < norm_count; k++) {
            printf("  %s = %.15g:", names[k], (*results)[k].norm);
            for (size_t i = 0; i < (*results)[k].count; i++) {
                printf(" ");
                print_vector(&(*results)[k].vectors[i]);
            }
            printf("\n");
        }
    }

    free_longest_results(*results, norm_count);
    *results = NULL;
}

static void demo_correct(void)
{
    static const char *const NAMES[] = {"||x||inf", "||x||1", "||x||2", "||x||3", "||x||A, A = diag(2, 1)"};
    const double c1[] = {3.0, 4.0};
    const double c2[] = {-5.0, 0.0};
    const double c3[] = {1.0, 1.0};
    const double c4[] = {0.0, 5.0};
    const Vector v1 = {c1, 2};
    const Vector v2 = {c2, 2};
    const Vector v3 = {c3, 2};
    const Vector v4 = {c4, 2};
    const double p1 = 1.0;
    const double p2 = 2.0;
    const double p3 = 3.0;
    const double a_values[] = {2.0, 0.0, 0.0, 1.0};
    LongestResult *results = NULL;

    printf("--- Корректные данные ---\n");

    MatrixNorm a;
    Status status = matrix_norm_init(&a, a_values, 2);
    if (status != STATUS_OK) {
        printf("matrix_norm_init: %s\n", status_message(status));
        return;
    }

    status = find_longest_vectors(&results, 1e-9, 2, 4, 5,
                                  v1, v2, v3, v4,
                                  norm_infinity, (const void *) NULL,
                                  norm_p, (const void *) &p1,
                                  norm_p, (const void *) &p2,
                                  norm_p, (const void *) &p3,
                                  norm_matrix, (const void *) &a);
    finish("Векторы (3, 4), (-5, 0), (1, 1), (0, 5); пять норм:", status, &results, NAMES, 5);

    const double d1[] = {1.0, 2.0, 2.0};
    const double d2[] = {3.0, 0.0, 0.0};
    const double d3[] = {0.0, 0.0, -3.0};
    const Vector w1 = {d1, 3};
    const Vector w2 = {d2, 3};
    const Vector w3 = {d3, 3};

    status = find_longest_vectors(&results, 1e-9, 3, 3, 1, w1, w2, w3, norm_p, (const void *) &p2);
    finish("Пространство 3: (1, 2, 2), (3, 0, 0), (0, 0, -3); ||x||2:", status, &results, NAMES + 2, 1);
}

static void demo_boundary(void)
{
    static const char *const INF[] = {"||x||inf"};
    static const char *const EUCLID[] = {"||x||2"};
    static const char *const P1000[] = {"||x||1000"};
    static const char *const MATRIX[] = {"||x||A, A = ((1, 1), (-1, 1))"};
    const double p2 = 2.0;
    const double p1000 = 1000.0;
    LongestResult *results = NULL;
    Status status = STATUS_OK;

    printf("--- Граничные случаи ---\n");

    const double single[] = {7.0, -1.0};
    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {single, 2},
                                  norm_infinity, (const void *) NULL);
    finish("Один вектор:", status, &results, INF, 1);

    const double zero[] = {0.0, 0.0};
    status = find_longest_vectors(&results, 1e-9, 2, 2, 1, (Vector) {zero, 2}, (Vector) {zero, 2},
                                  norm_infinity, (const void *) NULL);
    finish("Два нулевых вектора (оба самые длинные):", status, &results, INF, 1);

    const double huge[] = {1e300, 1e300};
    const double small[] = {1.0, 1.0};
    status = find_longest_vectors(&results, 1e-9, 2, 2, 1, (Vector) {huge, 2}, (Vector) {small, 2},
                                  norm_p, (const void *) &p2);
    finish("(1e300, 1e300): квадрат переполнился бы, масштаб спасает:", status, &results, EUCLID, 1);

    const double near_a[] = {1.0, 0.0};
    const double near_b[] = {1.0 + 1e-12, 0.0};
    status = find_longest_vectors(&results, 1e-9, 2, 2, 1, (Vector) {near_a, 2}, (Vector) {near_b, 2},
                                  norm_infinity, (const void *) NULL);
    finish("(1, 0) и (1 + 1e-12, 0), eps = 1e-9 - равны:", status, &results, INF, 1);
    status = find_longest_vectors(&results, 1e-15, 2, 2, 1, (Vector) {near_a, 2}, (Vector) {near_b, 2},
                                  norm_infinity, (const void *) NULL);
    finish("То же, eps = 1e-15 - различаются:", status, &results, INF, 1);

    const double c1[] = {3.0, 4.0};
    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_p, (const void *) &p1000);
    finish("(3, 4), p = 1000 - почти как ||x||inf:", status, &results, P1000, 1);

    const double skew_values[] = {1.0, 1.0, -1.0, 1.0};
    const double c2[] = {5.0, 0.0};
    MatrixNorm skew;
    status = matrix_norm_init(&skew, skew_values, 2);
    if (status == STATUS_OK) {
        status = find_longest_vectors(&results, 1e-9, 2, 2, 1, (Vector) {c1, 2}, (Vector) {c2, 2},
                                      norm_matrix, (const void *) &skew);
    }
    finish("Несимметричная A: (Ax, x) = x^2 + y^2, как ||x||2:", status, &results, MATRIX, 1);

    const double m7[] = {-7.0};
    const double p7[] = {7.0};
    const double p3[] = {3.0};
    status = find_longest_vectors(&results, 1e-9, 1, 3, 1, (Vector) {m7, 1}, (Vector) {p7, 1},
                                  (Vector) {p3, 1}, norm_infinity, (const void *) NULL);
    finish("n = 1: (-7), (7), (3):", status, &results, INF, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 0, (Vector) {c1, 2});
    finish("Ни одной нормы:", status, &results, INF, 0);
}

static void demo_errors(void)
{
    static const char *const ONE[] = {"норма"};
    const double c1[] = {3.0, 4.0};
    const double c3d[] = {1.0, 2.0, 3.0};
    const double bad[] = {1.0, NAN};
    const double huge[] = {1e308, 1e308};
    const double p2 = 2.0;
    const double p_half = 0.5;
    const double p_inf = INFINITY;
    const double not_pd[] = {1.0, 2.0, 2.0, 1.0};
    const double identity3[] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    const double p1 = 1.0;
    LongestResult *results = NULL;
    Status status = STATUS_OK;

    printf("--- Неверные данные ---\n");

    status = find_longest_vectors(NULL, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_infinity, (const void *) NULL);
    finish("Указатель на результат NULL:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 0.0, 2, 1, 1, (Vector) {c1, 2}, norm_infinity, (const void *) NULL);
    finish("eps = 0:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 0, 1, 1, (Vector) {c1, 2}, norm_infinity, (const void *) NULL);
    finish("n = 0:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 0, 1, norm_infinity, (const void *) NULL);
    finish("Ни одного вектора:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 2, 1, (Vector) {c1, 2}, (Vector) {c3d, 3},
                                  norm_infinity, (const void *) NULL);
    finish("Вектор размерности 3 при n = 2:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {bad, 2}, norm_infinity, (const void *) NULL);
    finish("Координата NAN:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, (NormFunction) NULL, (const void *) NULL);
    finish("Вместо функции нормы NULL:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_p, (const void *) &p_half);
    finish("p = 0.5:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_p, (const void *) &p_inf);
    finish("p = INFINITY:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_p, (const void *) NULL);
    finish("Для ||x||p не передан p:", status, &results, ONE, 1);

    MatrixNorm matrix;
    status = matrix_norm_init(&matrix, not_pd, 2);
    printf("matrix_norm_init для ((1, 2), (2, 1)):\n  ошибка: %s\n", status_message(status));

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_matrix, (const void *) &matrix);
    finish("Норма с непроверенной матрицей:", status, &results, ONE, 1);

    status = matrix_norm_init(&matrix, identity3, 3);
    if (status == STATUS_OK) {
        status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {c1, 2}, norm_matrix, (const void *) &matrix);
    }
    finish("Матрица 3x3 для векторов размерности 2:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 1, (Vector) {huge, 2}, norm_p, (const void *) &p1);
    finish("(1e308, 1e308), ||x||1 = 2e308:", status, &results, ONE, 1);

    status = find_longest_vectors(&results, 1e-9, 2, 1, 2, (Vector) {c1, 2},
                                  norm_p, (const void *) &p2, norm_p, (const void *) &p_half);
    finish("Ошибка во второй норме (первая уже посчитана - память освобождается):", status, &results, ONE, 2);
}

int main(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    demo_correct();
    demo_boundary();
    demo_errors();

    return 0;
}
