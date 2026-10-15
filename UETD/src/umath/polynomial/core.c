/// @file src/umath/polynomial/core.c
/// @brief Scalar polynomial evaluation and integration in descending coefficient order.
#include <math.h>
#include <stdint.h>
#include <umath/polynomial/core.h>
#include <umath/scalar.h>

umath_status_t umath_polynomial_evaluate(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t x,
                                         umath_scalar_t* value) {
    size_t index;
    umath_accumulator_t result;
    if (coefficients == NULL || value == NULL || degree == SIZE_MAX || !umath_scalar_is_finite(x)) return UMATH_INVALID_ARGUMENT;
    result = (umath_accumulator_t)coefficients[0];
    if (!umath_accumulator_is_finite(result)) return UMATH_NUMERIC_FAILURE;
    for (index = 1U; index <= degree; ++index) {
        if (!umath_scalar_is_finite(coefficients[index])) return UMATH_NUMERIC_FAILURE;
        result = result * (umath_accumulator_t)x + (umath_accumulator_t)coefficients[index];
        if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) {
            return UMATH_NUMERIC_FAILURE;
        }
    }
    *value = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_polynomial_derivative_evaluate(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t x,
                                                    umath_scalar_t* value) {
    size_t index;
    umath_accumulator_t result;
    if (coefficients == NULL || value == NULL || degree == SIZE_MAX || !umath_scalar_is_finite(x)) return UMATH_INVALID_ARGUMENT;
    if (degree == 0U) {
        *value = (umath_scalar_t)0;
        return UMATH_OK;
    }
    result = (umath_accumulator_t)coefficients[0] * (umath_accumulator_t)degree;
    for (index = 1U; index < degree; ++index) {
        if (!umath_scalar_is_finite(coefficients[index])) return UMATH_NUMERIC_FAILURE;
        result = result * (umath_accumulator_t)x +
                 (umath_accumulator_t)coefficients[index] * (umath_accumulator_t)(degree - index);
        if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) {
            return UMATH_NUMERIC_FAILURE;
        }
    }
    if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) { return UMATH_NUMERIC_FAILURE; }
    *value = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_polynomial_integral_evaluate(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t lower,
                                                  umath_scalar_t upper, umath_scalar_t* integral) {
    size_t index;
    umath_accumulator_t at_lower, at_upper;
    if (coefficients == NULL || integral == NULL || degree == SIZE_MAX || !umath_scalar_is_finite(lower) ||
        !umath_scalar_is_finite(upper)) { return UMATH_INVALID_ARGUMENT; }
    at_lower = (umath_accumulator_t)coefficients[0] / (umath_accumulator_t)(degree + 1U);
    at_upper = at_lower;
    if (!umath_accumulator_is_finite(at_lower)) return UMATH_NUMERIC_FAILURE;
    for (index = 1U; index <= degree; ++index) {
        const umath_accumulator_t coefficient = (umath_accumulator_t)coefficients[index] /
                                                (umath_accumulator_t)(degree - index + 1U);
        if (!umath_accumulator_is_finite(coefficient)) return UMATH_NUMERIC_FAILURE;
        at_lower = at_lower * (umath_accumulator_t)lower + coefficient;
        at_upper = at_upper * (umath_accumulator_t)upper + coefficient;
        if (!umath_accumulator_is_finite(at_lower) || !umath_accumulator_is_finite(at_upper)) return UMATH_NUMERIC_FAILURE;
    }
    at_lower *= (umath_accumulator_t)lower;
    at_upper *= (umath_accumulator_t)upper;
    if (!umath_accumulator_is_finite(at_lower) || !umath_accumulator_is_finite(at_upper) ||
        !umath_scalar_is_finite((umath_scalar_t)(at_upper - at_lower))) { return UMATH_NUMERIC_FAILURE; }
    *integral = (umath_scalar_t)(at_upper - at_lower);
    return UMATH_OK;
}

static bool polynomial_coefficients_finite(const umath_scalar_t* coefficients, size_t degree) {
    size_t i;
    if (coefficients == NULL || degree == SIZE_MAX) return false;
    for (i = 0; i <= degree; ++i) if (!umath_scalar_is_finite(coefficients[i])) return false;
    return true;
}

static bool polynomial_store(umath_accumulator_t value, umath_scalar_t* output) {
    const umath_scalar_t converted = (umath_scalar_t)value;
    if (!umath_accumulator_is_finite(value) || !umath_scalar_is_finite(converted)) return false;
    *output = converted;
    return true;
}

umath_status_t umath_polynomial_derivative_coefficients(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t* output, size_t output_capacity) {
    size_t i, count;
    if (output == NULL || !polynomial_coefficients_finite(coefficients, degree)) return UMATH_INVALID_ARGUMENT;
    count = degree == 0 ? 1 : degree;
    if (output_capacity < count) return UMATH_DIMENSION_MISMATCH;
    if (degree == 0) { output[0] = 0; return UMATH_OK; }
    for (i = 0; i < degree; ++i)
        if (!polynomial_store((umath_accumulator_t)coefficients[i] * (umath_accumulator_t)(degree - i), &output[i])) return UMATH_NUMERIC_FAILURE;
    return UMATH_OK;
}

umath_status_t umath_polynomial_integral_coefficients(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t integration_constant, umath_scalar_t* output, size_t output_capacity) {
    size_t i;
    if (output == NULL || !umath_scalar_is_finite(integration_constant) || !polynomial_coefficients_finite(coefficients, degree)) return UMATH_INVALID_ARGUMENT;
    if (degree > SIZE_MAX - 2U) return UMATH_INVALID_ARGUMENT;
    if (output_capacity < degree + 2U) return UMATH_DIMENSION_MISMATCH;
    for (i = 0; i <= degree; ++i)
        if (!polynomial_store((umath_accumulator_t)coefficients[i] / (umath_accumulator_t)(degree - i + 1U), &output[i])) return UMATH_NUMERIC_FAILURE;
    output[degree + 1U] = integration_constant;
    return UMATH_OK;
}

umath_status_t umath_polynomial_add(const umath_scalar_t* left, size_t left_degree, const umath_scalar_t* right, size_t right_degree, umath_scalar_t* output, size_t output_capacity) {
    size_t degree, count, i;
    if (output == NULL || !polynomial_coefficients_finite(left, left_degree) || !polynomial_coefficients_finite(right, right_degree)) return UMATH_INVALID_ARGUMENT;
    degree = left_degree > right_degree ? left_degree : right_degree;
    if (degree == SIZE_MAX) return UMATH_INVALID_ARGUMENT;
    count = degree + 1U;
    if (output_capacity < count) return UMATH_DIMENSION_MISMATCH;
    for (i = 0; i < count; ++i) {
        umath_accumulator_t sum = 0;
        if (i >= degree - left_degree) sum += (umath_accumulator_t)left[i - (degree - left_degree)];
        if (i >= degree - right_degree) sum += (umath_accumulator_t)right[i - (degree - right_degree)];
        if (!polynomial_store(sum, &output[i])) return UMATH_NUMERIC_FAILURE;
    }
    return UMATH_OK;
}

umath_status_t umath_polynomial_multiply(const umath_scalar_t* left, size_t left_degree, const umath_scalar_t* right, size_t right_degree, umath_scalar_t* output, size_t output_capacity) {
    size_t degree, count, i, j;
    if (output == NULL || !polynomial_coefficients_finite(left, left_degree) || !polynomial_coefficients_finite(right, right_degree)) return UMATH_INVALID_ARGUMENT;
    if (left_degree > SIZE_MAX - right_degree) return UMATH_INVALID_ARGUMENT;
    degree = left_degree + right_degree;
    if (degree == SIZE_MAX) return UMATH_INVALID_ARGUMENT;
    count = degree + 1U;
    if (output_capacity < count) return UMATH_DIMENSION_MISMATCH;
    for (i = 0; i < count; ++i) output[i] = 0;
    for (i = 0; i <= left_degree; ++i) {
        for (j = 0; j <= right_degree; ++j) {
            const umath_accumulator_t sum = (umath_accumulator_t)output[i + j] +
                                            (umath_accumulator_t)left[i] * (umath_accumulator_t)right[j];
            if (!polynomial_store(sum, &output[i + j])) return UMATH_NUMERIC_FAILURE;
        }
    }
    return UMATH_OK;
}

umath_status_t umath_polynomial_compose(const umath_scalar_t* outer, size_t outer_degree, const umath_scalar_t* inner, size_t inner_degree, umath_scalar_t* output, size_t output_capacity) {
    size_t result_degree, count, step, current_degree = 0;
    if (output == NULL || !polynomial_coefficients_finite(outer, outer_degree) || !polynomial_coefficients_finite(inner, inner_degree)) return UMATH_INVALID_ARGUMENT;
    if (inner_degree == 0) result_degree = 0;
    else {
        if (outer_degree > (SIZE_MAX - 1U) / inner_degree) return UMATH_INVALID_ARGUMENT;
        result_degree = outer_degree * inner_degree;
    }
    if (result_degree == SIZE_MAX) return UMATH_INVALID_ARGUMENT;
    count = result_degree + 1U;
    if (output_capacity < count) return UMATH_DIMENSION_MISMATCH;
    if (inner_degree == 0) {
        umath_accumulator_t value = (umath_accumulator_t)outer[0];
        size_t i;
        for (i = 1; i <= outer_degree; ++i) {
            value = value * (umath_accumulator_t)inner[0] + (umath_accumulator_t)outer[i];
            if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
        }
        return polynomial_store(value, output) ? UMATH_OK : UMATH_NUMERIC_FAILURE;
    }
    output[0] = outer[0];
    for (step = 1; step <= outer_degree; ++step) {
        size_t new_degree = current_degree + inner_degree;
        size_t k = new_degree + 1U;
        while (k > 0) {
            size_t j;
            umath_accumulator_t sum = 0;
            --k;
            for (j = 0; j <= inner_degree; ++j) {
                if (k >= j && k - j <= current_degree)
                    sum += (umath_accumulator_t)output[k - j] * (umath_accumulator_t)inner[j];
            }
            if (k == new_degree) sum += (umath_accumulator_t)outer[step];
            if (!polynomial_store(sum, &output[k])) return UMATH_NUMERIC_FAILURE;
        }
        current_degree = new_degree;
    }
    return UMATH_OK;
}

