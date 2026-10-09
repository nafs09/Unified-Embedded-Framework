/// @file src/umath/polynomial.c
/// @brief Scalar polynomial evaluation and integration in descending coefficient order.
#include <math.h>
#include <stdint.h>
#include <uef/umath/polynomial.h>
#include <uef/umath/scalar.h>

umath_status_t umath_polynomial_evaluate(const umath_scalar_t *coefficients,
                                         size_t degree, umath_scalar_t x,
                                         umath_scalar_t *value)
{
    size_t index;
    umath_accumulator_t result;
    if (coefficients == NULL || value == NULL || degree == SIZE_MAX ||
        !umath_scalar_is_finite(x)) return UMATH_INVALID_ARGUMENT;
    result = (umath_accumulator_t)coefficients[0];
    if (!isfinite(result)) return UMATH_NUMERIC_FAILURE;
    for (index = 1U; index <= degree; ++index) {
        if (!umath_scalar_is_finite(coefficients[index])) return UMATH_NUMERIC_FAILURE;
        result = result * (umath_accumulator_t)x +
                 (umath_accumulator_t)coefficients[index];
        if (!isfinite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) {
            return UMATH_NUMERIC_FAILURE;
        }
    }
    *value = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_polynomial_derivative_evaluate(const umath_scalar_t *coefficients,
                                                    size_t degree, umath_scalar_t x,
                                                    umath_scalar_t *value)
{
    size_t index;
    umath_accumulator_t result;
    if (coefficients == NULL || value == NULL || degree == SIZE_MAX ||
        !umath_scalar_is_finite(x)) return UMATH_INVALID_ARGUMENT;
    if (degree == 0U) { *value = (umath_scalar_t)0; return UMATH_OK; }
    result = (umath_accumulator_t)coefficients[0] *
             (umath_accumulator_t)degree;
    for (index = 1U; index < degree; ++index) {
        if (!umath_scalar_is_finite(coefficients[index])) return UMATH_NUMERIC_FAILURE;
        result = result * (umath_accumulator_t)x +
                 (umath_accumulator_t)coefficients[index] *
                 (umath_accumulator_t)(degree - index);
        if (!isfinite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) {
            return UMATH_NUMERIC_FAILURE;
        }
    }
    if (!isfinite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *value = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_polynomial_integral_evaluate(const umath_scalar_t *coefficients,
                                                  size_t degree,
                                                  umath_scalar_t lower,
                                                  umath_scalar_t upper,
                                                  umath_scalar_t *integral)
{
    size_t index;
    umath_accumulator_t at_lower, at_upper;
    if (coefficients == NULL || integral == NULL || degree == SIZE_MAX ||
        !umath_scalar_is_finite(lower) || !umath_scalar_is_finite(upper)) {
        return UMATH_INVALID_ARGUMENT;
    }
    at_lower = (umath_accumulator_t)coefficients[0] /
               (umath_accumulator_t)(degree + 1U);
    at_upper = at_lower;
    if (!isfinite(at_lower)) return UMATH_NUMERIC_FAILURE;
    for (index = 1U; index <= degree; ++index) {
        const umath_accumulator_t coefficient =
            (umath_accumulator_t)coefficients[index] /
            (umath_accumulator_t)(degree - index + 1U);
        if (!isfinite(coefficient)) return UMATH_NUMERIC_FAILURE;
        at_lower = at_lower * (umath_accumulator_t)lower + coefficient;
        at_upper = at_upper * (umath_accumulator_t)upper + coefficient;
        if (!isfinite(at_lower) || !isfinite(at_upper)) return UMATH_NUMERIC_FAILURE;
    }
    at_lower *= (umath_accumulator_t)lower;
    at_upper *= (umath_accumulator_t)upper;
    if (!isfinite(at_lower) || !isfinite(at_upper) ||
        !umath_scalar_is_finite((umath_scalar_t)(at_upper - at_lower))) {
        return UMATH_NUMERIC_FAILURE;
    }
    *integral = (umath_scalar_t)(at_upper - at_lower);
    return UMATH_OK;
}
