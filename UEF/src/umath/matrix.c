/// @file src/umath/matrix.c
/// @brief Fixed-storage row-major dense matrix kernels.
#include <stdint.h>
#include <math.h>
#include <uef/umath/matrix.h>
#include <uef/umath/scalar.h>

static bool valid_shape(size_t rows, size_t columns)
{
    return rows != 0U && columns != 0U && rows <= SIZE_MAX / columns;
}

umath_status_t umath_matrix_vector(const umath_scalar_t *matrix,
                                   const umath_scalar_t *vector,
                                   size_t rows, size_t columns,
                                   umath_scalar_t *output)
{
    size_t row, column;
    if (matrix == NULL || vector == NULL || output == NULL ||
        !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    for (row = 0U; row < rows; ++row) {
        double sum = 0.0;
        for (column = 0U; column < columns; ++column) {
            const double a = (double)matrix[row * columns + column];
            const double b = (double)vector[column];
            if (!isfinite(a) || !isfinite(b)) return UMATH_NUMERIC_FAILURE;
            sum += a * b;
        }
        if (!isfinite(sum) || !umath_scalar_is_finite((umath_scalar_t)sum)) {
            return UMATH_NUMERIC_FAILURE;
        }
        output[row] = (umath_scalar_t)sum;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_multiply(const umath_scalar_t *left,
                                     const umath_scalar_t *right,
                                     size_t rows, size_t inner, size_t columns,
                                     umath_scalar_t *output)
{
    size_t row, column, k;
    if (left == NULL || right == NULL || output == NULL ||
        !valid_shape(rows, inner) || !valid_shape(inner, columns) ||
        !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    for (row = 0U; row < rows; ++row) {
        for (column = 0U; column < columns; ++column) {
            double sum = 0.0;
            for (k = 0U; k < inner; ++k) {
                const double a = (double)left[row * inner + k];
                const double b = (double)right[k * columns + column];
                if (!isfinite(a) || !isfinite(b)) return UMATH_NUMERIC_FAILURE;
                sum += a * b;
            }
            if (!isfinite(sum) || !umath_scalar_is_finite((umath_scalar_t)sum)) {
                return UMATH_NUMERIC_FAILURE;
            }
            output[row * columns + column] = (umath_scalar_t)sum;
        }
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_transpose(const umath_scalar_t *input,
                                      size_t rows, size_t columns,
                                      umath_scalar_t *output)
{
    size_t row, column;
    if (input == NULL || output == NULL || !valid_shape(rows, columns)) {
        return UMATH_INVALID_ARGUMENT;
    }
    for (row = 0U; row < rows; ++row) {
        for (column = 0U; column < columns; ++column) {
            const umath_scalar_t value = input[row * columns + column];
            if (!umath_scalar_is_finite(value)) return UMATH_NUMERIC_FAILURE;
            output[column * rows + row] = value;
        }
    }
    return UMATH_OK;
}

static umath_status_t matrix_binary(const umath_scalar_t *left,
                                   const umath_scalar_t *right,
                                   size_t rows, size_t columns,
                                   umath_scalar_t *output, bool subtract)
{
    size_t index, count;
    if (left == NULL || right == NULL || output == NULL ||
        !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    count = rows * columns;
    for (index = 0U; index < count; ++index) {
        const double a = (double)left[index], b = (double)right[index];
        const double result = subtract ? a - b : a + b;
        if (!isfinite(a) || !isfinite(b) || !isfinite(result) ||
            !umath_scalar_is_finite((umath_scalar_t)result)) {
            return UMATH_NUMERIC_FAILURE;
        }
        output[index] = (umath_scalar_t)result;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_add(const umath_scalar_t *left,
                                const umath_scalar_t *right,
                                size_t rows, size_t columns,
                                umath_scalar_t *output)
{ return matrix_binary(left, right, rows, columns, output, false); }

umath_status_t umath_matrix_sub(const umath_scalar_t *left,
                                const umath_scalar_t *right,
                                size_t rows, size_t columns,
                                umath_scalar_t *output)
{ return matrix_binary(left, right, rows, columns, output, true); }

umath_status_t umath_matrix_scale(const umath_scalar_t *input,
                                  size_t rows, size_t columns,
                                  umath_scalar_t scale,
                                  umath_scalar_t *output)
{
    size_t index, count;
    if (input == NULL || output == NULL || !valid_shape(rows, columns) ||
        !umath_scalar_is_finite(scale)) return UMATH_INVALID_ARGUMENT;
    count = rows * columns;
    for (index = 0U; index < count; ++index) {
        const double value = (double)input[index] * (double)scale;
        if (!isfinite(value) || !umath_scalar_is_finite((umath_scalar_t)value)) {
            return UMATH_NUMERIC_FAILURE;
        }
        output[index] = (umath_scalar_t)value;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_symmetrize(umath_scalar_t *matrix, size_t dimension)
{
    size_t row, column;
    if (matrix == NULL || !valid_shape(dimension, dimension)) {
        return UMATH_INVALID_ARGUMENT;
    }
    for (row = 0U; row < dimension; ++row) {
        for (column = row + 1U; column < dimension; ++column) {
            const double a = (double)matrix[row * dimension + column];
            const double b = (double)matrix[column * dimension + row];
            const double average = 0.5 * (a + b);
            if (!isfinite(a) || !isfinite(b) || !isfinite(average) ||
                !umath_scalar_is_finite((umath_scalar_t)average)) {
                return UMATH_NUMERIC_FAILURE;
            }
            matrix[row * dimension + column] = (umath_scalar_t)average;
            matrix[column * dimension + row] = (umath_scalar_t)average;
        }
    }
    return UMATH_OK;
}