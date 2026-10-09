/// @file include/uef/umath/fixed.h
/// @brief Explicit Q-format arithmetic and checked floating-point conversion.
#ifndef UEF_UMATH_FIXED_H
#define UEF_UMATH_FIXED_H
#include <uef/umath/config.h>

#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UMATH_Q15_FRAC_BITS 15
#define UMATH_Q31_FRAC_BITS 31
#define UMATH_Q16_FRAC_BITS 16
#define UMATH_Q16_ONE (INT32_C(1) << UMATH_Q16_FRAC_BITS)

/// Fixed-point multiply truncates fractional bits toward zero after the widened product.
/// Scalar-to-fixed conversion rounds half away from zero and rejects out-of-range values.
uef_q15_t umath_q15_add_sat(uef_q15_t a, uef_q15_t b);
uef_q15_t umath_q15_sub_sat(uef_q15_t a, uef_q15_t b);
uef_q15_t umath_q15_mul_sat(uef_q15_t a, uef_q15_t b);
uef_q31_t umath_q31_add_sat(uef_q31_t a, uef_q31_t b);
uef_q31_t umath_q31_sub_sat(uef_q31_t a, uef_q31_t b);
uef_q31_t umath_q31_mul_sat(uef_q31_t a, uef_q31_t b);
uef_q16_t umath_q16_add_sat(uef_q16_t a, uef_q16_t b);
uef_q16_t umath_q16_sub_sat(uef_q16_t a, uef_q16_t b);
uef_q16_t umath_q16_mul_sat(uef_q16_t a, uef_q16_t b);
umath_status_t umath_q15_from_scalar(umath_scalar_t value, uef_q15_t *result);
umath_status_t umath_q31_from_scalar(umath_scalar_t value, uef_q31_t *result);
umath_status_t umath_q16_from_scalar(umath_scalar_t value, uef_q16_t *result);
umath_scalar_t umath_q15_to_scalar(uef_q15_t value);
umath_scalar_t umath_q31_to_scalar(uef_q31_t value);
umath_scalar_t umath_q16_to_scalar(uef_q16_t value);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_FIXED_H */
