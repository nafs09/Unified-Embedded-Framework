/// @file include/umath/vector/vector3.h
/// @brief Three-component vector value type and common operations.
#ifndef UETD_UMATH_VECTOR3_H
#define UETD_UMATH_VECTOR3_H
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    umath_scalar_t x, y, z;
} umath_vec3_t;
umath_vec3_t umath_vec3_add(umath_vec3_t a, umath_vec3_t b);
umath_vec3_t umath_vec3_sub(umath_vec3_t a, umath_vec3_t b);
umath_vec3_t umath_vec3_scale(umath_vec3_t value, umath_scalar_t scale);
umath_scalar_t umath_vec3_dot(umath_vec3_t a, umath_vec3_t b);
umath_vec3_t umath_vec3_cross(umath_vec3_t a, umath_vec3_t b);
umath_status_t umath_vec3_norm(umath_vec3_t value, umath_scalar_t* norm);
umath_status_t umath_vec3_normalize(umath_vec3_t value, umath_vec3_t* unit);

/// Componentwise division returns UMATH_SINGULAR if any divisor component is zero.
umath_status_t umath_vec3_mul(umath_vec3_t left, umath_vec3_t right, umath_vec3_t* result);
umath_status_t umath_vec3_div(umath_vec3_t left, umath_vec3_t right, umath_vec3_t* result);
umath_status_t umath_vec3_distance(umath_vec3_t left, umath_vec3_t right, umath_scalar_t* distance);
umath_status_t umath_vec3_lerp(umath_vec3_t start, umath_vec3_t end, umath_scalar_t t, umath_vec3_t* result);

/// The unsigned angle is in [0, pi]; zero vectors return UMATH_SINGULAR.
umath_status_t umath_vec3_angle(umath_vec3_t left, umath_vec3_t right, umath_scalar_t* radians);
/// Projection and rejection require a nonzero axis. Reflection requires a nonzero normal.
umath_status_t umath_vec3_project(umath_vec3_t value, umath_vec3_t axis, umath_vec3_t* projection);
umath_status_t umath_vec3_reject(umath_vec3_t value, umath_vec3_t axis, umath_vec3_t* rejection);
umath_status_t umath_vec3_reflect(umath_vec3_t value, umath_vec3_t normal, umath_vec3_t* reflected);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_VECTOR3_H */
