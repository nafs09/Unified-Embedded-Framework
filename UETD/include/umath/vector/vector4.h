/// @file include/umath/vector/vector4.h
/// @brief Four-component vector value type and common operations.
#ifndef UETD_UMATH_VECTOR4_H
#define UETD_UMATH_VECTOR4_H
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    umath_scalar_t x, y, z, w;
} umath_vec4_t;
/// Add two 4D vectors.
umath_vec4_t umath_vec4_add(umath_vec4_t a, umath_vec4_t b);
/// Subtract one 4D vector from another.
umath_vec4_t umath_vec4_sub(umath_vec4_t a, umath_vec4_t b);
/// Scale a 4D vector by a scalar.
umath_vec4_t umath_vec4_scale(umath_vec4_t value, umath_scalar_t scale);
/// Compute the dot product of two 4D vectors.
umath_scalar_t umath_vec4_dot(umath_vec4_t a, umath_vec4_t b);
/// Compute the Euclidean norm of a 4D vector.
umath_status_t umath_vec4_norm(umath_vec4_t value, umath_scalar_t* norm);
/// Normalize a 4D vector to unit length.
umath_status_t umath_vec4_normalize(umath_vec4_t value, umath_vec4_t* unit);

/// Componentwise division returns UMATH_SINGULAR if any divisor component is zero.
umath_status_t umath_vec4_mul(umath_vec4_t left, umath_vec4_t right, umath_vec4_t* result);
/// Divide a 4D vector by a scalar.
umath_status_t umath_vec4_div(umath_vec4_t left, umath_vec4_t right, umath_vec4_t* result);
/// Compute the distance between two 4D vectors.
umath_status_t umath_vec4_distance(umath_vec4_t left, umath_vec4_t right, umath_scalar_t* distance);
/// Linearly interpolate between two 4D vectors.
umath_status_t umath_vec4_lerp(umath_vec4_t start, umath_vec4_t end, umath_scalar_t t, umath_vec4_t* result);

/// The unsigned angle is in [0, pi]; zero vectors return UMATH_SINGULAR.
umath_status_t umath_vec4_angle(umath_vec4_t left, umath_vec4_t right, umath_scalar_t* radians);
/// Projection and rejection require a nonzero axis. Reflection requires a nonzero normal.
umath_status_t umath_vec4_project(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* projection);
/// Remove the component of a 4D vector parallel to the supplied normal.
umath_status_t umath_vec4_reject(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* rejection);
/// Reflect a 4D vector across the supplied normal.
umath_status_t umath_vec4_reflect(umath_vec4_t value, umath_vec4_t normal, umath_vec4_t* reflected);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_VECTOR4_H */
