/// @file src/umath/vector.c
/// @brief Checked operations for caller-owned contiguous scalar vectors.
#include <math.h>
#include <uef/umath/scalar.h>
#include <uef/umath/vector.h>

umath_status_t umath_vector_clamp(const umath_scalar_t *input,
                                  size_t count,
                                  umath_scalar_t minimum,
                                  umath_scalar_t maximum,
                                  umath_scalar_t *output)
{
    size_t index;
    if (input == NULL || output == NULL || count == 0U ||
        !umath_scalar_is_finite(minimum) || !umath_scalar_is_finite(maximum) ||
        minimum > maximum) return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        if (!umath_scalar_is_finite(input[index])) return UMATH_NUMERIC_FAILURE;
    }
    for (index = 0U; index < count; ++index) {
        output[index] = input[index] < minimum ? minimum :
                        (input[index] > maximum ? maximum : input[index]);
    }
    return UMATH_OK;
}

umath_status_t umath_vector_dot(const umath_scalar_t *left,
                                const umath_scalar_t *right,
                                size_t count,
                                umath_scalar_t *result)
{
    size_t index;
    double sum = 0.0;
    if (left == NULL || right == NULL || result == NULL || count == 0U) {
        return UMATH_INVALID_ARGUMENT;
    }
    for (index = 0U; index < count; ++index) {
        if (!umath_scalar_is_finite(left[index]) ||
            !umath_scalar_is_finite(right[index])) return UMATH_NUMERIC_FAILURE;
        sum += (double)left[index] * (double)right[index];
        if (!isfinite(sum)) return UMATH_NUMERIC_FAILURE;
    }
    if (!umath_scalar_is_finite((umath_scalar_t)sum)) return UMATH_NUMERIC_FAILURE;
    *result = (umath_scalar_t)sum;
    return UMATH_OK;
}

umath_status_t umath_vector_norm(const umath_scalar_t *input,
                                 size_t count,
                                 umath_scalar_t *result)
{
    size_t index;
    double scale = 0.0, sum_squares = 1.0, norm;
    if (input == NULL || result == NULL || count == 0U) return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        const double value = fabs((double)input[index]);
        if (!isfinite(value)) return UMATH_NUMERIC_FAILURE;
        if (value == 0.0) continue;
        if (scale < value) {
            const double ratio = scale / value;
            sum_squares = 1.0 + sum_squares * ratio * ratio;
            scale = value;
        } else {
            const double ratio = value / scale;
            sum_squares += ratio * ratio;
        }
    }
    norm = scale == 0.0 ? 0.0 : scale * sqrt(sum_squares);
    if (!isfinite(norm) || !umath_scalar_is_finite((umath_scalar_t)norm)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *result = (umath_scalar_t)norm;
    return UMATH_OK;
}