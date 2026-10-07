/// @file include/uef/umath/scalar.h
/// @brief Checked scalar operations over the project-selected UMATH scalar.
#ifndef UEF_UMATH_SCALAR_H
#define UEF_UMATH_SCALAR_H

#include <stdbool.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Checked scalar APIs leave the output unchanged on failure. Null/non-finite inputs are invalid;
/// a mathematically finite result outside the selected scalar range is UMATH_NUMERIC_FAILURE.
bool umath_scalar_is_finite(umath_scalar_t value);
umath_status_t umath_scalar_clamp(umath_scalar_t value,
                                  umath_scalar_t minimum,
                                  umath_scalar_t maximum,
                                  umath_scalar_t *result);
umath_status_t umath_sin(umath_scalar_t radians, umath_scalar_t *result);
umath_status_t umath_cos(umath_scalar_t radians, umath_scalar_t *result);
umath_status_t umath_atan2(umath_scalar_t y, umath_scalar_t x,
                           umath_scalar_t *radians);
umath_status_t umath_sqrt(umath_scalar_t value, umath_scalar_t *result);
umath_status_t umath_invsqrt(umath_scalar_t value, umath_scalar_t *result);
umath_status_t umath_exp(umath_scalar_t value, umath_scalar_t *result);
umath_status_t umath_log(umath_scalar_t value, umath_scalar_t *result);
umath_status_t umath_hypot(umath_scalar_t x, umath_scalar_t y,
                           umath_scalar_t *result);
/// Wrap an angle in radians to the half-open interval [-pi, pi).
umath_status_t umath_wrap_angle_pi(umath_scalar_t radians,
                                   umath_scalar_t *wrapped);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_SCALAR_H */