/// @file include/uef/umath/autodiff/dual.h
/// @brief Planned forward-mode dual scalar for fixed-cost derivative propagation.
#ifndef UEF_UMATH_AUTODIFF_DUAL_H
#define UEF_UMATH_AUTODIFF_DUAL_H
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// One value and one seeded derivative direction; callers sweep seeds to form Jacobians.
typedef struct { umath_scalar_t value, derivative; } umath_dual_t;
umath_status_t umath_dual_add(umath_dual_t a, umath_dual_t b, umath_dual_t *out);
umath_status_t umath_dual_sub(umath_dual_t a, umath_dual_t b, umath_dual_t *out);
umath_status_t umath_dual_mul(umath_dual_t a, umath_dual_t b, umath_dual_t *out);
/// TODO(UMATH-DUAL): Handle zero/ill-conditioned denominators and preserve output on failure.
umath_status_t umath_dual_div(umath_dual_t numerator, umath_dual_t denominator, umath_dual_t *out);
/// TODO(UMATH-DUAL): Define derivative propagation and finite/overflow policy for each scalar function.
umath_status_t umath_dual_sin(umath_dual_t input, umath_dual_t *out);
umath_status_t umath_dual_cos(umath_dual_t input, umath_dual_t *out);
umath_status_t umath_dual_exp(umath_dual_t input, umath_dual_t *out);
umath_status_t umath_dual_log(umath_dual_t input, umath_dual_t *out);
umath_status_t umath_dual_sqrt(umath_dual_t input, umath_dual_t *out);
#ifdef __cplusplus
}
#endif
#endif