/// @file src/umath/vector2.c
/// @brief Two-dimensional vector operations.
#include <math.h>
#include <uef/umath/scalar.h>
#include <uef/umath/vector2.h>

umath_vec2_t umath_vec2_add(umath_vec2_t a, umath_vec2_t b)
{
    const umath_vec2_t result = { a.x + b.x, a.y + b.y };
    return result;
}

umath_vec2_t umath_vec2_sub(umath_vec2_t a, umath_vec2_t b)
{
    const umath_vec2_t result = { a.x - b.x, a.y - b.y };
    return result;
}

umath_vec2_t umath_vec2_scale(umath_vec2_t value, umath_scalar_t scale)
{
    const umath_vec2_t result = { value.x * scale, value.y * scale };
    return result;
}

umath_scalar_t umath_vec2_dot(umath_vec2_t a, umath_vec2_t b)
{
    return a.x * b.x + a.y * b.y;
}

umath_scalar_t umath_vec2_cross(umath_vec2_t a, umath_vec2_t b)
{
    return a.x * b.y - a.y * b.x;
}

umath_status_t umath_vec2_norm(umath_vec2_t value, umath_scalar_t *norm)
{
    const double result = hypot((double)value.x, (double)value.y);
    if (norm == NULL || !isfinite(result) ||
        !umath_scalar_is_finite((umath_scalar_t)result)) return UMATH_NUMERIC_FAILURE;
    *norm = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_vec2_normalize(umath_vec2_t value, umath_vec2_t *unit)
{
    umath_scalar_t norm;
    umath_vec2_t result;
    if (unit == NULL || umath_vec2_norm(value, &norm) != UMATH_OK || norm <= 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    result.x = value.x / norm;
    result.y = value.y / norm;
    *unit = result;
    return UMATH_OK;
}