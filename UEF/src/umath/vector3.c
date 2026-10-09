/// @file src/umath/vector3.c
/// @brief Three-dimensional vector operations in a right-handed coordinate basis.
#include <math.h>
#include <uef/umath/internal/scalar_math.h>
#include <uef/umath/scalar.h>
#include <uef/umath/vector3.h>

umath_vec3_t umath_vec3_add(umath_vec3_t a, umath_vec3_t b)
{
    const umath_vec3_t result = { a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}

umath_vec3_t umath_vec3_sub(umath_vec3_t a, umath_vec3_t b)
{
    const umath_vec3_t result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

umath_vec3_t umath_vec3_scale(umath_vec3_t value, umath_scalar_t scale)
{
    const umath_vec3_t result = { value.x * scale, value.y * scale, value.z * scale };
    return result;
}

umath_scalar_t umath_vec3_dot(umath_vec3_t a, umath_vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

umath_vec3_t umath_vec3_cross(umath_vec3_t a, umath_vec3_t b)
{
    const umath_vec3_t result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return result;
}

umath_status_t umath_vec3_norm(umath_vec3_t value, umath_scalar_t *norm)
{
    const umath_accumulator_t xy = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)value.x, (umath_accumulator_t)value.y);
    const umath_accumulator_t result = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, xy, (umath_accumulator_t)value.z);
    if (norm == NULL) return UMATH_INVALID_ARGUMENT;
    if (!isfinite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *norm = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_vec3_normalize(umath_vec3_t value, umath_vec3_t *unit)
{
    umath_scalar_t norm;
    umath_vec3_t result;
    umath_status_t status;
    if (unit == NULL) return UMATH_INVALID_ARGUMENT;
    status = umath_vec3_norm(value, &norm);
    if (status != UMATH_OK) return status;
    if (norm <= 0) return UMATH_INVALID_ARGUMENT;
    result.x = value.x / norm;
    result.y = value.y / norm;
    result.z = value.z / norm;
    *unit = result;
    return UMATH_OK;
}
