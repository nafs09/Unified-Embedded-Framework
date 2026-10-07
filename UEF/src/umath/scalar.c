/// @file src/umath/scalar.c
/// @brief Portable checked scalar operations; target modules may provide verified accelerators.
#include <math.h>
#include <uef/umath/scalar.h>

static umath_status_t store_finite(double value, umath_scalar_t *result)
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
    return isfinite((double)value) != 0;
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
    return store_finite(sin((double)radians), result);
}

umath_status_t umath_cos(umath_scalar_t radians, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(radians)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(cos((double)radians), result);
}

umath_status_t umath_atan2(umath_scalar_t y, umath_scalar_t x,
                           umath_scalar_t *radians)
{
    if (radians == NULL || !umath_scalar_is_finite(x) ||
        !umath_scalar_is_finite(y)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(atan2((double)y, (double)x), radians);
}

umath_status_t umath_sqrt(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) || value < 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(sqrt((double)value), result);
}

umath_status_t umath_invsqrt(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) || value <= 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(1.0 / sqrt((double)value), result);
}

umath_status_t umath_exp(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(exp((double)value), result);
}

umath_status_t umath_log(umath_scalar_t value, umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(value) || value <= 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(log((double)value), result);
}

umath_status_t umath_hypot(umath_scalar_t x, umath_scalar_t y,
                           umath_scalar_t *result)
{
    if (result == NULL || !umath_scalar_is_finite(x) ||
        !umath_scalar_is_finite(y)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(hypot((double)x, (double)y), result);
}

umath_status_t umath_wrap_angle_pi(umath_scalar_t radians,
                                   umath_scalar_t *wrapped)
{
    const double pi = 3.14159265358979323846;
    const double two_pi = 2.0 * pi;
    double value;
    if (wrapped == NULL || !umath_scalar_is_finite(radians)) {
        return UMATH_INVALID_ARGUMENT;
    }
    value = fmod((double)radians, two_pi);
    if (value >= pi) value -= two_pi;
    else if (value < -pi) value += two_pi;
    return store_finite(value, wrapped);
}
