/// @file src/umath/scalar.c
/// @brief Portable checked scalar operations; target modules may provide verified accelerators.
#include <uef/umath/internal/scalar_math.h>
#include <uef/umath/scalar.h>

static umath_status_t store_finite(umath_accumulator_t value,
                                   umath_scalar_t *result)
{
    const umath_scalar_t converted = (umath_scalar_t)value;
    if (!isfinite(value) || !umath_scalar_is_finite(converted)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *result = converted;
    return UMATH_OK;
}

bool umath_scalar_is_finite(umath_scalar_t value)
{
    return isfinite(value) != 0;
}

umath_status_t umath_scalar_clamp(umath_scalar_t value,
                                  umath_scalar_t minimum,
                                  umath_scalar_t maximum,
                                  umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) ||
        !umath_scalar_is_finite(minimum) || !umath_scalar_is_finite(maximum) ||
        minimum > maximum) {
        return UMATH_INVALID_ARGUMENT;
    }
    *result = value < minimum ? minimum : (value > maximum ? maximum : value);
    return UMATH_OK;
}

umath_status_t umath_sin(umath_scalar_t radians, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(radians)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_UNARY_MATH(
        sin, (umath_accumulator_t)radians), result);
}

umath_status_t umath_cos(umath_scalar_t radians, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(radians)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_UNARY_MATH(
        cos, (umath_accumulator_t)radians), result);
}

umath_status_t umath_atan2(umath_scalar_t y, umath_scalar_t x,
                           umath_scalar_t *radians)
{
    if (radians == NULL || !umath_scalar_is_finite(x) ||
        !umath_scalar_is_finite(y)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_BINARY_MATH(
        atan2, (umath_accumulator_t)y, (umath_accumulator_t)x), radians);
}

umath_status_t umath_sqrt(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) || value < 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_UNARY_MATH(
        sqrt, (umath_accumulator_t)value), result);
}

umath_status_t umath_invsqrt(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) || value <= 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UMATH_ACCUMULATOR_C(1) /
        UEF_UMATH_ACCUM_UNARY_MATH(sqrt, (umath_accumulator_t)value), result);
}

umath_status_t umath_exp(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_UNARY_MATH(
        exp, (umath_accumulator_t)value), result);
}

umath_status_t umath_log(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) || value <= 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_UNARY_MATH(
        log, (umath_accumulator_t)value), result);
}

umath_status_t umath_hypot(umath_scalar_t x, umath_scalar_t y,
                           umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(x) ||
        !umath_scalar_is_finite(y)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)x, (umath_accumulator_t)y), result);
}

umath_status_t umath_wrap_angle_pi(umath_scalar_t radians,
                                   umath_scalar_t *wrapped)
{
    const umath_accumulator_t pi = UMATH_ACCUMULATOR_C(3.14159265358979323846);
    const umath_accumulator_t two_pi = UMATH_ACCUMULATOR_C(2) * pi;
    umath_accumulator_t value;
    if (wrapped == NULL || !umath_scalar_is_finite(radians)) {
        return UMATH_INVALID_ARGUMENT;
    }
    value = UEF_UMATH_ACCUM_BINARY_MATH(
        fmod, (umath_accumulator_t)radians, two_pi);
    if (value >= pi) value -= two_pi;
    else if (value < -pi) value += two_pi;
    return store_finite(value, wrapped);
}
