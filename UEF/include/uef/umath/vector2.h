/// @file include/uef/umath/vector2.h
/// @brief Two-component vector value type and common operations.
#ifndef UEF_UMATH_VECTOR2_H
#define UEF_UMATH_VECTOR2_H
#include <uef/umath/config.h>

#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { umath_scalar_t x, y; } umath_vec2_t;
umath_vec2_t umath_vec2_add(umath_vec2_t a, umath_vec2_t b);
umath_vec2_t umath_vec2_sub(umath_vec2_t a, umath_vec2_t b);
umath_vec2_t umath_vec2_scale(umath_vec2_t value, umath_scalar_t scale);
umath_scalar_t umath_vec2_dot(umath_vec2_t a, umath_vec2_t b);
umath_scalar_t umath_vec2_cross(umath_vec2_t a, umath_vec2_t b);
umath_status_t umath_vec2_norm(umath_vec2_t value, umath_scalar_t *norm);
umath_status_t umath_vec2_normalize(umath_vec2_t value, umath_vec2_t *unit);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_VECTOR2_H */