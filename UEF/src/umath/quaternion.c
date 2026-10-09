/// @file src/umath/quaternion.c
/// @brief Quaternion operations using scalar-first Hamilton convention.
#include <math.h>
#include <uef/umath/internal/scalar_math.h>
#include <uef/umath/quaternion.h>
#include <uef/umath/scalar.h>

umath_quaternion_t umath_quaternion_identity(void)
{
    const umath_quaternion_t identity = { (umath_scalar_t)1, 0, 0, 0 };
    return identity;
}

umath_quaternion_t umath_quaternion_conjugate(umath_quaternion_t value)
{
    const umath_quaternion_t result = { value.w, -value.x, -value.y, -value.z };
    return result;
}

umath_quaternion_t umath_quaternion_multiply(umath_quaternion_t a,
                                            umath_quaternion_t b)
{
    const umath_quaternion_t result = {
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
    return result;
}

umath_status_t umath_quaternion_normalize(umath_quaternion_t value,
                                          umath_quaternion_t *unit)
{
    const umath_accumulator_t wx = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)value.w, (umath_accumulator_t)value.x);
    const umath_accumulator_t yz = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)value.y, (umath_accumulator_t)value.z);
    const umath_accumulator_t norm = UEF_UMATH_ACCUM_BINARY_MATH(hypot, wx, yz);
    umath_quaternion_t result;
    if (unit == NULL || !isfinite(norm) || norm <= UMATH_ACCUMULATOR_C(0)) {
        return UMATH_INVALID_ARGUMENT;
    }
    result.w = (umath_scalar_t)(value.w / norm);
    result.x = (umath_scalar_t)(value.x / norm);
    result.y = (umath_scalar_t)(value.y / norm);
    result.z = (umath_scalar_t)(value.z / norm);
    *unit = result;
    return UMATH_OK;
}

umath_status_t umath_quaternion_rotate(umath_quaternion_t rotation,
                                       umath_vec3_t input,
                                       umath_vec3_t *output)
{
    umath_quaternion_t q, p, rotated;
    umath_vec3_t result;
    umath_status_t status;
    if (output == NULL) return UMATH_INVALID_ARGUMENT;
    status = umath_quaternion_normalize(rotation, &q);
    if (status != UMATH_OK) return status;
    p.w = 0;
    p.x = input.x;
    p.y = input.y;
    p.z = input.z;
    rotated = umath_quaternion_multiply(
        umath_quaternion_multiply(q, p), umath_quaternion_conjugate(q));
    result.x = rotated.x;
    result.y = rotated.y;
    result.z = rotated.z;
    if (!umath_scalar_is_finite(result.x) || !umath_scalar_is_finite(result.y) ||
        !umath_scalar_is_finite(result.z)) return UMATH_NUMERIC_FAILURE;
    *output = result;
    return UMATH_OK;
}

umath_status_t umath_quaternion_from_axis_angle(umath_vec3_t axis,
                                                umath_scalar_t angle_radians,
                                                umath_quaternion_t *rotation)
{
    umath_vec3_t unit_axis;
    umath_quaternion_t result;
    umath_status_t status;
    const umath_accumulator_t half =
        UMATH_ACCUMULATOR_C(0.5) * (umath_accumulator_t)angle_radians;
    umath_accumulator_t sine;
    if (rotation == NULL || !umath_scalar_is_finite(angle_radians)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_vec3_normalize(axis, &unit_axis);
    if (status != UMATH_OK) return status;
    sine = UEF_UMATH_ACCUM_UNARY_MATH(sin, half);
    result.w = (umath_scalar_t)UEF_UMATH_ACCUM_UNARY_MATH(cos, half);
    result.x = (umath_scalar_t)((umath_accumulator_t)unit_axis.x *
        sine);
    result.y = (umath_scalar_t)((umath_accumulator_t)unit_axis.y *
        sine);
    result.z = (umath_scalar_t)((umath_accumulator_t)unit_axis.z *
        sine);
    if (!umath_scalar_is_finite(result.w) || !umath_scalar_is_finite(result.x) ||
        !umath_scalar_is_finite(result.y) || !umath_scalar_is_finite(result.z)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *rotation = result;
    return UMATH_OK;
}

umath_status_t umath_quaternion_to_axis_angle(umath_quaternion_t rotation,
                                              umath_vec3_t *axis,
                                              umath_scalar_t *angle_radians)
{
    umath_quaternion_t q;
    umath_vec3_t result_axis;
    umath_status_t status;
    umath_accumulator_t vector_norm, angle;
    if (axis == NULL || angle_radians == NULL) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_quaternion_normalize(rotation, &q);
    if (status != UMATH_OK) return status;
    if (q.w < 0) {
        q.w = -q.w; q.x = -q.x; q.y = -q.y; q.z = -q.z;
    }
    vector_norm = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)q.x, (umath_accumulator_t)q.y);
    vector_norm = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, vector_norm, (umath_accumulator_t)q.z);
    angle = UMATH_ACCUMULATOR_C(2) * UEF_UMATH_ACCUM_BINARY_MATH(
        atan2, vector_norm, (umath_accumulator_t)q.w);
    if (vector_norm <= UMATH_ACCUMULATOR_C(1.0e-12)) {
        result_axis.x = 1; result_axis.y = 0; result_axis.z = 0;
    } else {
        result_axis.x = (umath_scalar_t)(q.x / vector_norm);
        result_axis.y = (umath_scalar_t)(q.y / vector_norm);
        result_axis.z = (umath_scalar_t)(q.z / vector_norm);
    }
    *axis = result_axis;
    *angle_radians = (umath_scalar_t)angle;
    return UMATH_OK;
}
