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
umath_vec4_t umath_vec4_add(umath_vec4_t a, umath_vec4_t b);
umath_vec4_t umath_vec4_sub(umath_vec4_t a, umath_vec4_t b);
umath_vec4_t umath_vec4_scale(umath_vec4_t value, umath_scalar_t scale);
umath_scalar_t umath_vec4_dot(umath_vec4_t a, umath_vec4_t b);
umath_status_t umath_vec4_norm(umath_vec4_t value, umath_scalar_t* norm);
umath_status_t umath_vec4_normalize(umath_vec4_t value, umath_vec4_t* unit);

/// Componentwise division returns UMATH_SINGULAR if any divisor component is zero.
umath_status_t umath_vec4_mul(umath_vec4_t left, umath_vec4_t right, umath_vec4_t* result);
umath_status_t umath_vec4_div(umath_vec4_t left, umath_vec4_t right, umath_vec4_t* result);
umath_status_t umath_vec4_distance(umath_vec4_t left, umath_vec4_t right, umath_scalar_t* distance);
umath_status_t umath_vec4_lerp(umath_vec4_t start, umath_vec4_t end, umath_scalar_t t, umath_vec4_t* result);

/// The unsigned angle is in [0, pi]; zero vectors return UMATH_SINGULAR.
umath_status_t umath_vec4_angle(umath_vec4_t left, umath_vec4_t right, umath_scalar_t* radians);
/// Projection and rejection require a nonzero axis. Reflection requires a nonzero normal.
umath_status_t umath_vec4_project(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* projection);
umath_status_t umath_vec4_reject(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* rejection);
umath_status_t umath_vec4_reflect(umath_vec4_t value, umath_vec4_t normal, umath_vec4_t* reflected);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_VECTOR4_H */
