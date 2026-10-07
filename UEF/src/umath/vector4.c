/// @file src/umath/vector4.c
/// @brief Four-dimensional vector operations.
#include <math.h>
#include <uef/umath/scalar.h>
#include <uef/umath/vector4.h>

umath_vec4_t umath_vec4_add(umath_vec4_t a, umath_vec4_t b)
{
    const umath_vec4_t result = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
    return result;
}

umath_vec4_t umath_vec4_sub(umath_vec4_t a, umath_vec4_t b)
{
    const umath_vec4_t result = { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
    return result;
}

umath_vec4_t umath_vec4_scale(umath_vec4_t value, umath_scalar_t scale)
{
    const umath_vec4_t result = { value.x * scale, value.y * scale, value.z * scale, value.w * scale };
    return result;
}

umath_scalar_t umath_vec4_dot(umath_vec4_t a, umath_vec4_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

umath_status_t umath_vec4_norm(umath_vec4_t value, umath_scalar_t *norm)
{
    const double result = sqrt((double)value.x * value.x + (double)value.y * value.y +
                               (double)value.z * value.z + (double)value.w * value.w);
    if (norm == NULL || !isfinite(result) ||
        !umath_scalar_is_finite((umath_scalar_t)result)) return UMATH_NUMERIC_FAILURE;
    *norm = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_vec4_normalize(umath_vec4_t value, umath_vec4_t *unit)
{
    umath_scalar_t norm;
    umath_vec4_t result;
    if (unit == NULL || umath_vec4_norm(value, &norm) != UMATH_OK || norm <= 0) {
        return UMATH_INVALID_ARGUMENT;
    }
    result.x = value.x / norm;
    result.y = value.y / norm;
    result.z = value.z / norm;
    result.w = value.w / norm;
    *unit = result;
    return UMATH_OK;
}