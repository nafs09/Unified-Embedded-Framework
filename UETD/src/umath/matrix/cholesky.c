/// @file src/umath/matrix/cholesky.c
/// @brief Allocation-free Cholesky factorization and solve.
#include <math.h>
#include <umath/internal/scalar_math.h>
#include <umath/matrix/cholesky.h>
#include <umath/scalar.h>

umath_status_t umath_cholesky_factor(umath_scalar_t* matrix, size_t dimension) {
    size_t row, column, k;
    if (matrix == NULL || dimension == 0U || dimension > SIZE_MAX / dimension) { return UMATH_INVALID_ARGUMENT; }
    for (row = 0U; row < dimension; ++row) {
        for (column = 0U; column <= row; ++column) {
            umath_accumulator_t sum = (umath_accumulator_t)matrix[row * dimension + column];
            if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
            for (k = 0U; k < column; ++k) {
                sum -= (umath_accumulator_t)matrix[row * dimension + k] * (umath_accumulator_t)matrix[column * dimension + k];
            }
            if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
            if (row == column) {
                if (!(sum > UMATH_ACCUMULATOR_C(0))) { return UMATH_NOT_POSITIVE_DEFINITE; }
                matrix[row * dimension + column] = (umath_scalar_t)UETD_UMATH_ACCUM_UNARY_MATH(sqrt, sum);
            } else {
                const umath_accumulator_t diagonal = (umath_accumulator_t)matrix[column * dimension + column];
                if (!(diagonal > UMATH_ACCUMULATOR_C(0)) || !umath_accumulator_is_finite(diagonal)) {
                    return UMATH_NOT_POSITIVE_DEFINITE;
                }
                matrix[row * dimension + column] = (umath_scalar_t)(sum / diagonal);
            }
        }
        for (column = row + 1U; column < dimension; ++column) { matrix[row * dimension + column] = (umath_scalar_t)0; }
    }
    return UMATH_OK;
}

umath_status_t umath_cholesky_solve(const umath_scalar_t* lower_factor, const umath_scalar_t* right_hand_side, size_t dimension,
                                    umath_scalar_t* solution) {
    size_t row, column;
    if (lower_factor == NULL || right_hand_side == NULL || solution == NULL || dimension == 0U ||
        dimension > SIZE_MAX / dimension) { return UMATH_INVALID_ARGUMENT; }
    for (row = 0U; row < dimension; ++row) {
        umath_accumulator_t value = (umath_accumulator_t)right_hand_side[row];
        for (column = 0U; column < row; ++column) {
            value -= (umath_accumulator_t)lower_factor[row * dimension + column] * (umath_accumulator_t)solution[column];
        }
        if (!umath_accumulator_is_finite(value) || !(lower_factor[row * dimension + row] > (umath_scalar_t)0)) {
            return UMATH_NUMERIC_FAILURE;
        }
        solution[row] = (umath_scalar_t)(value / lower_factor[row * dimension + row]);
    }
    for (row = dimension; row-- > 0U;) {
        umath_accumulator_t value = (umath_accumulator_t)solution[row];
        for (column = row + 1U; column < dimension; ++column) {
            value -= (umath_accumulator_t)lower_factor[column * dimension + row] * (umath_accumulator_t)solution[column];
        }
        if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
        solution[row] = (umath_scalar_t)(value / lower_factor[row * dimension + row]);
    }
    return UMATH_OK;
}

umath_status_t umath_cholesky_log_determinant(const umath_scalar_t* lower_factor, size_t dimension, umath_scalar_t* log_determinant) {
    size_t index;
    umath_accumulator_t sum = UMATH_ACCUMULATOR_C(0);
    umath_accumulator_t value;
    umath_scalar_t converted;
    if (lower_factor == NULL || log_determinant == NULL || dimension == 0U || dimension > SIZE_MAX / dimension)
        return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < dimension; ++index) {
        const umath_scalar_t diagonal = lower_factor[index * dimension + index];
        if (!umath_scalar_is_finite(diagonal)) return UMATH_NUMERIC_FAILURE;
        if (diagonal <= UMATH_SCALAR_C(0)) return UMATH_NOT_POSITIVE_DEFINITE;
        sum += UETD_UMATH_ACCUM_UNARY_MATH(log, (umath_accumulator_t)diagonal);
        if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
    }
    value = UMATH_ACCUMULATOR_C(2) * sum;
    if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
    converted = (umath_scalar_t)value;
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *log_determinant = converted;
    return UMATH_OK;
}

umath_status_t umath_cholesky_inverse(const umath_scalar_t* lower_factor, size_t dimension, umath_scalar_t* inverse) {
    size_t row, column, index, elements;
    if (lower_factor == NULL || inverse == NULL || dimension == 0U || dimension > SIZE_MAX / dimension)
        return UMATH_INVALID_ARGUMENT;
    elements = dimension * dimension;
    if (elements > SIZE_MAX / sizeof(*inverse)) return UMATH_INVALID_ARGUMENT;
    for (row = 0U; row < dimension; ++row) {
        for (column = 0U; column <= row; ++column) {
            const umath_scalar_t value = lower_factor[row * dimension + column];
            if (!umath_scalar_is_finite(value)) return UMATH_NUMERIC_FAILURE;
            if (row == column && value <= UMATH_SCALAR_C(0)) return UMATH_NOT_POSITIVE_DEFINITE;
        }
    }

    /* Use the caller's output as the bounded workspace: first form L^-1 in
     * its lower triangle, then write the upper triangle of L^-T L^-1. */
    for (row = 0U; row < dimension; ++row)
        for (column = 0U; column < dimension; ++column)
            inverse[row * dimension + column] = UMATH_SCALAR_C(0);
    for (column = 0U; column < dimension; ++column) {
        for (row = column; row < dimension; ++row) {
            umath_accumulator_t sum = row == column ? UMATH_ACCUMULATOR_C(1) : UMATH_ACCUMULATOR_C(0);
            umath_accumulator_t value;
            for (index = column; index < row; ++index) {
                sum -= (umath_accumulator_t)lower_factor[row * dimension + index] *
                       (umath_accumulator_t)inverse[index * dimension + column];
                if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
            }
            value = sum / (umath_accumulator_t)lower_factor[row * dimension + row];
            if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
            {
                const umath_scalar_t converted = (umath_scalar_t)value;
                if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
                inverse[row * dimension + column] = converted;
            }
        }
    }
    for (row = 0U; row < dimension; ++row) {
        for (column = row; column < dimension; ++column) {
            umath_accumulator_t sum = UMATH_ACCUMULATOR_C(0);
            umath_scalar_t converted;
            for (index = column; index < dimension; ++index) {
                sum += (umath_accumulator_t)inverse[index * dimension + row] *
                       (umath_accumulator_t)inverse[index * dimension + column];
                if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
            }
            converted = (umath_scalar_t)sum;
            if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
            inverse[row * dimension + column] = converted;
        }
    }
    for (row = 0U; row < dimension; ++row)
        for (column = row + 1U; column < dimension; ++column)
            inverse[column * dimension + row] = inverse[row * dimension + column];
    return UMATH_OK;
}

