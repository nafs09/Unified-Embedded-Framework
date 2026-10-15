/// @file include/umath/derivatives/dual.h
/// @brief Forward-mode dual scalar for fixed-cost derivative propagation.
#ifndef UETD_UMATH_AUTODIFF_DUAL_H
#define UETD_UMATH_AUTODIFF_DUAL_H
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// One value and one seeded derivative direction; callers sweep seeds to form Jacobians.
typedef struct {
    umath_scalar_t value, derivative;
} umath_dual_t;
/// Elementary extensions and piecewise operations use the derivative and discontinuity policies documented per API.
static inline umath_dual_t umath_dual_constant(umath_scalar_t value) {
    umath_dual_t result;
    result.value = value;
    result.derivative = UMATH_SCALAR_C(0);
    return result;
}
static inline umath_dual_t umath_dual_seed(umath_scalar_t value) {
    umath_dual_t result;
    result.value = value;
    result.derivative = UMATH_SCALAR_C(1);
    return result;
}
umath_status_t umath_dual_neg(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_add(umath_dual_t a, umath_dual_t b, umath_dual_t* out);
umath_status_t umath_dual_sub(umath_dual_t a, umath_dual_t b, umath_dual_t* out);
umath_status_t umath_dual_mul(umath_dual_t a, umath_dual_t b, umath_dual_t* out);
umath_status_t umath_dual_div(umath_dual_t numerator, umath_dual_t denominator, umath_dual_t* out);
umath_status_t umath_dual_reciprocal(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_powi(umath_dual_t base, uetd_i32_t exponent, umath_dual_t* out);
/// Real/dual exponentiation requires a strictly positive primal base.
umath_status_t umath_dual_pow(umath_dual_t base, umath_dual_t exponent, umath_dual_t* out);
umath_status_t umath_dual_sin(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_cos(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_tan(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_asin(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_acos(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_atan(umath_dual_t input, umath_dual_t* out);
/// atan2 returns UMATH_SINGULAR at the origin and on its negative-x branch cut.
umath_status_t umath_dual_atan2(umath_dual_t y, umath_dual_t x, umath_dual_t* out);
umath_status_t umath_dual_exp(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_ln(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_sqrt(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_invsqrt(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_sinh(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_cosh(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_tanh(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_hypot(umath_dual_t x, umath_dual_t y, umath_dual_t* out);
umath_status_t umath_dual_fma(umath_dual_t left, umath_dual_t right, umath_dual_t addend, umath_dual_t* out);

/// Additional checked elementary functions and piecewise operations.

umath_status_t umath_dual_log1p(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_expm1(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_log2(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_log10(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_exp2(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_cbrt(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_asinh(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_acosh(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_atanh(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_floor(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_ceil(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_trunc(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_round(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_fmod(umath_dual_t numerator, umath_dual_t denominator, umath_dual_t* out);
umath_status_t umath_dual_remainder(umath_dual_t numerator, umath_dual_t denominator, umath_dual_t* out);
umath_status_t umath_dual_abs(umath_dual_t input, umath_dual_t* out);
umath_status_t umath_dual_min(umath_dual_t left, umath_dual_t right, umath_dual_t* out);
umath_status_t umath_dual_max(umath_dual_t left, umath_dual_t right, umath_dual_t* out);
umath_status_t umath_dual_clamp(umath_dual_t input, umath_scalar_t minimum, umath_scalar_t maximum, umath_dual_t* out);
umath_status_t umath_dual_wrap_angle_pi(umath_dual_t input, umath_dual_t* out);

#ifdef __cplusplus
}
#endif
#endif
