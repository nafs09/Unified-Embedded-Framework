/// @file include/umath/vector/vector2.h
/// @brief Two-component vector value type and common operations.
#ifndef UETD_UMATH_VECTOR2_H
#define UETD_UMATH_VECTOR2_H
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    umath_scalar_t x, y;
} umath_vec2_t;
umath_vec2_t umath_vec2_add(umath_vec2_t a, umath_vec2_t b);
umath_vec2_t umath_vec2_sub(umath_vec2_t a, umath_vec2_t b);
umath_vec2_t umath_vec2_scale(umath_vec2_t value, umath_scalar_t scale);
umath_scalar_t umath_vec2_dot(umath_vec2_t a, umath_vec2_t b);
umath_scalar_t umath_vec2_cross(umath_vec2_t a, umath_vec2_t b);
umath_status_t umath_vec2_norm(umath_vec2_t value, umath_scalar_t* norm);
umath_status_t umath_vec2_normalize(umath_vec2_t value, umath_vec2_t* unit);

/// Componentwise division returns UMATH_SINGULAR if either divisor component is zero.
umath_status_t umath_vec2_mul(umath_vec2_t left, umath_vec2_t right, umath_vec2_t* result);
umath_status_t umath_vec2_div(umath_vec2_t left, umath_vec2_t right, umath_vec2_t* result);
umath_status_t umath_vec2_distance(umath_vec2_t left, umath_vec2_t right, umath_scalar_t* distance);
umath_status_t umath_vec2_lerp(umath_vec2_t start, umath_vec2_t end, umath_scalar_t t, umath_vec2_t* result);

/// Signed angle is in [-pi, pi]; zero vectors return UMATH_SINGULAR.
umath_status_t umath_vec2_signed_angle(umath_vec2_t left, umath_vec2_t right, umath_scalar_t* radians);
/// Projection and rejection require a nonzero axis. Reflection requires a nonzero normal.
umath_status_t umath_vec2_project(umath_vec2_t value, umath_vec2_t axis, umath_vec2_t* projection);
umath_status_t umath_vec2_reject(umath_vec2_t value, umath_vec2_t axis, umath_vec2_t* rejection);
umath_status_t umath_vec2_reflect(umath_vec2_t value, umath_vec2_t normal, umath_vec2_t* reflected);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_VECTOR2_H */
