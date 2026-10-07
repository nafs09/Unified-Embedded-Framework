/// @file include/uef/umath/vector3.h
/// @brief Three-component vector value type and common operations.
#ifndef UEF_UMATH_VECTOR3_H
#define UEF_UMATH_VECTOR3_H

#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { umath_scalar_t x, y, z; } umath_vec3_t;
umath_vec3_t umath_vec3_add(umath_vec3_t a, umath_vec3_t b);
umath_vec3_t umath_vec3_sub(umath_vec3_t a, umath_vec3_t b);
umath_vec3_t umath_vec3_scale(umath_vec3_t value, umath_scalar_t scale);
umath_scalar_t umath_vec3_dot(umath_vec3_t a, umath_vec3_t b);
umath_vec3_t umath_vec3_cross(umath_vec3_t a, umath_vec3_t b);
umath_status_t umath_vec3_norm(umath_vec3_t value, umath_scalar_t *norm);
umath_status_t umath_vec3_normalize(umath_vec3_t value, umath_vec3_t *unit);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_VECTOR3_H */