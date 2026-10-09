/// @file src/umath/matrix/cholesky.c
/// @brief Allocation-free Cholesky factorization and solve.
#include <math.h>
#include <uef/umath/internal/scalar_math.h>
#include <uef/umath/matrix/cholesky.h>
#include <uef/umath/scalar.h>

umath_status_t umath_cholesky_factor(umath_scalar_t *matrix, size_t dimension)
{
    size_t row, column, k;
    if (matrix == NULL || dimension == 0U || dimension > SIZE_MAX / dimension) {
        return UMATH_INVALID_ARGUMENT;
    }
    for (row = 0U; row < dimension; ++row) {
        for (column = 0U; column <= row; ++column) {
            umath_accumulator_t sum =
                (umath_accumulator_t)matrix[row * dimension + column];
            if (!isfinite(sum)) return UMATH_NUMERIC_FAILURE;
            for (k = 0U; k < column; ++k) {
                sum -= (umath_accumulator_t)matrix[row * dimension + k] *
                       (umath_accumulator_t)matrix[column * dimension + k];
            }
            if (!isfinite(sum)) return UMATH_NUMERIC_FAILURE;
            if (row == column) {
                if (!(sum > UMATH_ACCUMULATOR_C(0))) {
                    return UMATH_NOT_POSITIVE_DEFINITE;
                }
                matrix[row * dimension + column] = (umath_scalar_t)
                    UEF_UMATH_ACCUM_UNARY_MATH(sqrt, sum);
            } else {
                const umath_accumulator_t diagonal =
                    (umath_accumulator_t)matrix[column * dimension + column];
                if (!(diagonal > UMATH_ACCUMULATOR_C(0)) || !isfinite(diagonal)) {
                    return UMATH_NOT_POSITIVE_DEFINITE;
                }
                matrix[row * dimension + column] = (umath_scalar_t)(sum / diagonal);
            }
        }
        for (column = row + 1U; column < dimension; ++column) {
            matrix[row * dimension + column] = (umath_scalar_t)0;
        }
    }
    return UMATH_OK;
}

umath_status_t umath_cholesky_solve(const umath_scalar_t *lower_factor,
                                    const umath_scalar_t *right_hand_side,
                                    size_t dimension,
                                    umath_scalar_t *solution)
{
    size_t row, column;
    if (lower_factor == NULL || right_hand_side == NULL || solution == NULL ||
        dimension == 0U || dimension > SIZE_MAX / dimension) {
        return UMATH_INVALID_ARGUMENT;
    }
    for (row = 0U; row < dimension; ++row) {
        umath_accumulator_t value = (umath_accumulator_t)right_hand_side[row];
        for (column = 0U; column < row; ++column) {
            value -= (umath_accumulator_t)lower_factor[row * dimension + column] *
                     (umath_accumulator_t)solution[column];
        }
        if (!isfinite(value) ||
            !(lower_factor[row * dimension + row] > (umath_scalar_t)0)) {
            return UMATH_NUMERIC_FAILURE;
        }
        solution[row] = (umath_scalar_t)(value / lower_factor[row * dimension + row]);
    }
    for (row = dimension; row-- > 0U;) {
        umath_accumulator_t value = (umath_accumulator_t)solution[row];
        for (column = row + 1U; column < dimension; ++column) {
            value -= (umath_accumulator_t)lower_factor[column * dimension + row] *
                     (umath_accumulator_t)solution[column];
        }
        if (!isfinite(value)) return UMATH_NUMERIC_FAILURE;
        solution[row] = (umath_scalar_t)(value / lower_factor[row * dimension + row]);
    }
    return UMATH_OK;
}
