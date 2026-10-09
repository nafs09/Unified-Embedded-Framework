/// @file include/uef/umath/vector4.h
/// @brief Four-component vector value type and common operations.
#ifndef UEF_UMATH_VECTOR4_H
#define UEF_UMATH_VECTOR4_H
#include <uef/umath/config.h>

#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { umath_scalar_t x, y, z, w; } umath_vec4_t;
umath_vec4_t umath_vec4_add(umath_vec4_t a, umath_vec4_t b);
umath_vec4_t umath_vec4_sub(umath_vec4_t a, umath_vec4_t b);
umath_vec4_t umath_vec4_scale(umath_vec4_t value, umath_scalar_t scale);
umath_scalar_t umath_vec4_dot(umath_vec4_t a, umath_vec4_t b);
umath_status_t umath_vec4_norm(umath_vec4_t value, umath_scalar_t *norm);
umath_status_t umath_vec4_normalize(umath_vec4_t value, umath_vec4_t *unit);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_VECTOR4_H */