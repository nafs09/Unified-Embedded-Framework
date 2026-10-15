/// @file src/umath/vector/vector.c
/// @brief Checked operations for caller-owned contiguous scalar vectors.
#include <math.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>
#include <umath/vector/vector.h>

typedef enum { VECTOR_ADD, VECTOR_SUB, VECTOR_MUL, VECTOR_DIV, VECTOR_SCALE } vector_operation_t;

static umath_accumulator_t vector_operation(umath_scalar_t left, umath_scalar_t right, vector_operation_t operation) {
    if (operation == VECTOR_ADD) return (umath_accumulator_t)left + (umath_accumulator_t)right;
    if (operation == VECTOR_SUB) return (umath_accumulator_t)left - (umath_accumulator_t)right;
    if (operation == VECTOR_MUL) return (umath_accumulator_t)left * (umath_accumulator_t)right;
    if (operation == VECTOR_DIV) return (umath_accumulator_t)left / (umath_accumulator_t)right;
    return (umath_accumulator_t)left * (umath_accumulator_t)right;
}

static umath_status_t vector_arithmetic(const umath_scalar_t* left, const umath_scalar_t* right, size_t count,
                                        umath_scalar_t scale, umath_scalar_t* output, vector_operation_t operation) {
    size_t index;
    if (left == NULL || output == NULL || count == 0U ||
        (operation != VECTOR_SCALE && right == NULL) ||
        (operation == VECTOR_SCALE && !umath_scalar_is_finite(scale))) return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        umath_scalar_t rhs = operation == VECTOR_SCALE ? scale : right[index];
        umath_accumulator_t value;
        if (!umath_scalar_is_finite(left[index]) || !umath_scalar_is_finite(rhs)) return UMATH_NUMERIC_FAILURE;
        if (operation == VECTOR_DIV && rhs == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
        value = vector_operation(left[index], rhs, operation);
        if (!umath_accumulator_is_finite(value) || !umath_scalar_is_finite((umath_scalar_t)value))
            return UMATH_NUMERIC_FAILURE;
    }
    for (index = 0U; index < count; ++index) {
        umath_scalar_t rhs = operation == VECTOR_SCALE ? scale : right[index];
        output[index] = (umath_scalar_t)vector_operation(left[index], rhs, operation);
    }
    return UMATH_OK;
}

umath_status_t umath_vector_add(const umath_scalar_t* left, const umath_scalar_t* right, size_t count,
                                umath_scalar_t* output) {
    return vector_arithmetic(left, right, count, UMATH_SCALAR_C(0), output, VECTOR_ADD);
}

umath_status_t umath_vector_sub(const umath_scalar_t* left, const umath_scalar_t* right, size_t count,
                                umath_scalar_t* output) {
    return vector_arithmetic(left, right, count, UMATH_SCALAR_C(0), output, VECTOR_SUB);
}

umath_status_t umath_vector_mul(const umath_scalar_t* left, const umath_scalar_t* right, size_t count,
                                umath_scalar_t* output) {
    return vector_arithmetic(left, right, count, UMATH_SCALAR_C(0), output, VECTOR_MUL);
}

umath_status_t umath_vector_div(const umath_scalar_t* numerator, const umath_scalar_t* denominator, size_t count,
                                umath_scalar_t* quotient) {
    return vector_arithmetic(numerator, denominator, count, UMATH_SCALAR_C(0), quotient, VECTOR_DIV);
}

umath_status_t umath_vector_scale(const umath_scalar_t* input, size_t count, umath_scalar_t scale,
                                  umath_scalar_t* output) {
    return vector_arithmetic(input, NULL, count, scale, output, VECTOR_SCALE);
}

umath_status_t umath_vector_clamp(const umath_scalar_t* input, size_t count, umath_scalar_t minimum, umath_scalar_t maximum,
                                  umath_scalar_t* output) {
    size_t index;
    if (input == NULL || output == NULL || count == 0U || !umath_scalar_is_finite(minimum) || !umath_scalar_is_finite(maximum) ||
        minimum > maximum)
        return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        if (!umath_scalar_is_finite(input[index])) return UMATH_NUMERIC_FAILURE;
    }
    for (index = 0U; index < count; ++index) {
        output[index] = input[index] < minimum ? minimum : (input[index] > maximum ? maximum : input[index]);
    }
    return UMATH_OK;
}

umath_status_t umath_vector_dot(const umath_scalar_t* left, const umath_scalar_t* right, size_t count, umath_scalar_t* result) {
    size_t index;
    umath_accumulator_t sum = UMATH_ACCUMULATOR_C(0);
    if (left == NULL || right == NULL || result == NULL || count == 0U) { return UMATH_INVALID_ARGUMENT; }
    for (index = 0U; index < count; ++index) {
        if (!umath_scalar_is_finite(left[index]) || !umath_scalar_is_finite(right[index])) return UMATH_NUMERIC_FAILURE;
        sum += (umath_accumulator_t)left[index] * (umath_accumulator_t)right[index];
        if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
    }
    if (!umath_scalar_is_finite((umath_scalar_t)sum)) return UMATH_NUMERIC_FAILURE;
    *result = (umath_scalar_t)sum;
    return UMATH_OK;
}

static umath_status_t vector_norm_accumulator(const umath_scalar_t* input, size_t count, umath_accumulator_t* norm) {
    size_t index;
    umath_accumulator_t scale = UMATH_ACCUMULATOR_C(0);
    umath_accumulator_t sum_squares = UMATH_ACCUMULATOR_C(1);
    if (input == NULL || norm == NULL || count == 0U) return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        umath_accumulator_t value;
        if (!umath_scalar_is_finite(input[index])) return UMATH_NUMERIC_FAILURE;
        value = UETD_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)input[index]);
        if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
        if (value == UMATH_ACCUMULATOR_C(0)) continue;
        if (scale < value) {
            const umath_accumulator_t ratio = scale / value;
            sum_squares = UMATH_ACCUMULATOR_C(1) + sum_squares * ratio * ratio;
            scale = value;
        } else {
            const umath_accumulator_t ratio = value / scale;
            sum_squares += ratio * ratio;
        }
        if (!umath_accumulator_is_finite(sum_squares)) return UMATH_NUMERIC_FAILURE;
    }
    *norm = scale == UMATH_ACCUMULATOR_C(0) ? UMATH_ACCUMULATOR_C(0)
                                             : scale * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, sum_squares);
    return umath_accumulator_is_finite(*norm) ? UMATH_OK : UMATH_NUMERIC_FAILURE;
}

umath_status_t umath_vector_norm(const umath_scalar_t* input, size_t count, umath_scalar_t* result) {
    umath_accumulator_t norm;
    umath_status_t status;
    if (input == NULL || result == NULL || count == 0U) return UMATH_INVALID_ARGUMENT;
    status = vector_norm_accumulator(input, count, &norm);
    if (status != UMATH_OK) return status;
    if (!umath_scalar_is_finite((umath_scalar_t)norm)) return UMATH_NUMERIC_FAILURE;
    *result = (umath_scalar_t)norm;
    return UMATH_OK;
}

umath_status_t umath_vector_normalize(const umath_scalar_t* input, size_t count, umath_scalar_t* output) {
    size_t index;
    umath_accumulator_t norm;
    umath_status_t status;
    if (input == NULL || output == NULL || count == 0U) return UMATH_INVALID_ARGUMENT;
    status = vector_norm_accumulator(input, count, &norm);
    if (status != UMATH_OK) return status;
    if (norm == UMATH_ACCUMULATOR_C(0)) return UMATH_SINGULAR;
    for (index = 0U; index < count; ++index) {
        const umath_accumulator_t component = (umath_accumulator_t)input[index] / norm;
        if (!umath_accumulator_is_finite(component) || !umath_scalar_is_finite((umath_scalar_t)component))
            return UMATH_NUMERIC_FAILURE;
    }
    for (index = 0U; index < count; ++index)
        output[index] = (umath_scalar_t)((umath_accumulator_t)input[index] / norm);
    return UMATH_OK;
}

umath_status_t umath_vector_distance(const umath_scalar_t* left, const umath_scalar_t* right, size_t count, umath_scalar_t* distance) {
    size_t index;
    umath_accumulator_t scale = UMATH_ACCUMULATOR_C(0);
    umath_accumulator_t sum_squares = UMATH_ACCUMULATOR_C(1);
    umath_accumulator_t norm;
    if (left == NULL || right == NULL || distance == NULL || count == 0U) return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        const umath_accumulator_t a = (umath_accumulator_t)left[index];
        const umath_accumulator_t b = (umath_accumulator_t)right[index];
        umath_accumulator_t magnitude;
        umath_accumulator_t difference;
        if (!umath_scalar_is_finite(left[index]) || !umath_scalar_is_finite(right[index])) return UMATH_NUMERIC_FAILURE;
        difference = a - b;
        if (!umath_accumulator_is_finite(difference)) return UMATH_NUMERIC_FAILURE;
        magnitude = UETD_UMATH_ACCUM_UNARY_MATH(fabs, difference);
        if (magnitude == UMATH_ACCUMULATOR_C(0)) continue;
        if (scale < magnitude) {
            const umath_accumulator_t ratio = scale / magnitude;
            sum_squares = UMATH_ACCUMULATOR_C(1) + sum_squares * ratio * ratio;
            scale = magnitude;
        } else {
            const umath_accumulator_t ratio = magnitude / scale;
            sum_squares += ratio * ratio;
        }
        if (!umath_accumulator_is_finite(sum_squares)) return UMATH_NUMERIC_FAILURE;
    }
    norm = scale == UMATH_ACCUMULATOR_C(0) ? UMATH_ACCUMULATOR_C(0)
                                             : scale * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, sum_squares);
    if (!umath_accumulator_is_finite(norm) || !umath_scalar_is_finite((umath_scalar_t)norm)) return UMATH_NUMERIC_FAILURE;
    *distance = (umath_scalar_t)norm;
    return UMATH_OK;
}

umath_status_t umath_vector_lerp(const umath_scalar_t* start, const umath_scalar_t* end, size_t count, umath_scalar_t t, umath_scalar_t* output) {
    size_t index;
    if (start == NULL || end == NULL || output == NULL || count == 0U || !umath_scalar_is_finite(t))
        return UMATH_INVALID_ARGUMENT;
    for (index = 0U; index < count; ++index) {
        umath_accumulator_t value;
        if (!umath_scalar_is_finite(start[index]) || !umath_scalar_is_finite(end[index])) return UMATH_NUMERIC_FAILURE;
        value = UETD_UMATH_ACCUM_FMA((umath_accumulator_t)t,
                                     (umath_accumulator_t)end[index] - (umath_accumulator_t)start[index],
                                     (umath_accumulator_t)start[index]);
        if (!umath_accumulator_is_finite(value) || !umath_scalar_is_finite((umath_scalar_t)value))
            return UMATH_NUMERIC_FAILURE;
    }
    for (index = 0U; index < count; ++index)
        output[index] = (umath_scalar_t)UETD_UMATH_ACCUM_FMA((umath_accumulator_t)t,
                                                              (umath_accumulator_t)end[index] -
                                                                  (umath_accumulator_t)start[index],
                                                              (umath_accumulator_t)start[index]);
    return UMATH_OK;
}

