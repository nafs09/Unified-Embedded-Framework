/// @file include/umath/scalar.h
/// @brief Checked scalar operations over the project-selected UMATH scalar.
#ifndef UETD_UMATH_SCALAR_H
#define UETD_UMATH_SCALAR_H
#include <stdbool.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Checked scalar APIs leave the output unchanged on failure. Null/non-finite inputs are invalid;
/// a mathematically finite result outside the selected scalar range is UMATH_NUMERIC_FAILURE.
/// Checked scalar APIs use accumulator-precision libm operations and commit only finite, representable results.
bool umath_scalar_is_finite(umath_scalar_t value);
/// Check an accumulator directly without narrowing it to the configured scalar type.
bool umath_accumulator_is_finite(umath_accumulator_t value);
/// Inline scalar-typed helpers for already validated arithmetic values.
static inline umath_scalar_t umath_scalar_abs(umath_scalar_t value) { return value < UMATH_SCALAR_C(0) ? -value : value; }
static inline umath_scalar_t umath_scalar_min(umath_scalar_t left, umath_scalar_t right) { return left < right ? left : right; }
static inline umath_scalar_t umath_scalar_max(umath_scalar_t left, umath_scalar_t right) { return left > right ? left : right; }
static inline umath_scalar_t umath_scalar_sign(umath_scalar_t value) {
    return value > UMATH_SCALAR_C(0) ? UMATH_SCALAR_C(1) : value < UMATH_SCALAR_C(0) ? UMATH_SCALAR_C(-1) : UMATH_SCALAR_C(0);
}
umath_status_t umath_scalar_neg(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_add(umath_scalar_t left, umath_scalar_t right, umath_scalar_t* result);
umath_status_t umath_scalar_sub(umath_scalar_t left, umath_scalar_t right, umath_scalar_t* result);
umath_status_t umath_scalar_mul(umath_scalar_t left, umath_scalar_t right, umath_scalar_t* result);
umath_status_t umath_scalar_div(umath_scalar_t numerator, umath_scalar_t denominator, umath_scalar_t* quotient);
umath_status_t umath_scalar_reciprocal(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_powi(umath_scalar_t base, uetd_i32_t exponent, umath_scalar_t* result);
/// Real power permits negative bases only for exactly integral exponents; 0^0 is defined as 1.
umath_status_t umath_scalar_pow(umath_scalar_t base, umath_scalar_t exponent, umath_scalar_t* result);
umath_status_t umath_scalar_clamp(umath_scalar_t value, umath_scalar_t minimum, umath_scalar_t maximum, umath_scalar_t* result);
umath_status_t umath_sin(umath_scalar_t radians, umath_scalar_t* result);
umath_status_t umath_cos(umath_scalar_t radians, umath_scalar_t* result);
umath_status_t umath_tan(umath_scalar_t radians, umath_scalar_t* result);
umath_status_t umath_atan2(umath_scalar_t y, umath_scalar_t x, umath_scalar_t* radians);
umath_status_t umath_asin(umath_scalar_t value, umath_scalar_t* radians);
umath_status_t umath_acos(umath_scalar_t value, umath_scalar_t* radians);
umath_status_t umath_atan(umath_scalar_t value, umath_scalar_t* radians);
umath_status_t umath_sqrt(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_invsqrt(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_exp(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_ln(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_sinh(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_cosh(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_tanh(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_hypot(umath_scalar_t x, umath_scalar_t y, umath_scalar_t* result);
/// Fused multiply-add rounded at accumulator precision, then converted to the configured scalar.
umath_status_t umath_fma(umath_scalar_t left, umath_scalar_t right, umath_scalar_t addend, umath_scalar_t* result);
/// Wrap an angle in radians to the half-open interval [-pi, pi).
umath_status_t umath_wrap_angle_pi(umath_scalar_t radians, umath_scalar_t* wrapped);
umath_status_t umath_scalar_log1p(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_expm1(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_log2(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_log10(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_exp2(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_cbrt(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_floor(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_ceil(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_trunc(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_round(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_asinh(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_acosh(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_atanh(umath_scalar_t value, umath_scalar_t* result);
umath_status_t umath_scalar_fmod(umath_scalar_t numerator, umath_scalar_t denominator, umath_scalar_t* result);
umath_status_t umath_scalar_remainder(umath_scalar_t numerator, umath_scalar_t denominator, umath_scalar_t* result);
umath_status_t umath_scalar_modf(umath_scalar_t value, umath_scalar_t* integral, umath_scalar_t* fractional);
umath_status_t umath_scalar_copysign(umath_scalar_t magnitude, umath_scalar_t sign, umath_scalar_t* result);
/// Stable small-angle scalar kernels used by estimator and geometry algorithms.
umath_status_t umath_sinc(umath_scalar_t radians, umath_scalar_t* result);
umath_status_t umath_cosc(umath_scalar_t radians, umath_scalar_t* result);
umath_status_t umath_sincc(umath_scalar_t radians, umath_scalar_t* result);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_SCALAR_H */
