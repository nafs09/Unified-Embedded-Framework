/// @file include/umath/fixed.h
/// @brief Explicit Q-format arithmetic and checked floating-point conversion.
#ifndef UETD_UMATH_FIXED_H
#define UETD_UMATH_FIXED_H
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Saturating absolute value, floor-rounded square root, and pairwise Q-format conversion are checked APIs.
#define UMATH_Q15_FRAC_BITS 15
#define UMATH_Q31_FRAC_BITS 31
#define UMATH_Q16_FRAC_BITS 16
#define UMATH_Q16_ONE (INT32_C(1) << UMATH_Q16_FRAC_BITS)

/// Fixed-point multiply truncates fractional bits toward zero after the widened product.
/// Fixed-point division truncates toward zero and saturates; a zero denominator returns UMATH_SINGULAR and leaves output unchanged.
/// Scalar-to-fixed conversion rounds half away from zero and rejects out-of-range values.
uetd_q15_t umath_q15_add_sat(uetd_q15_t a, uetd_q15_t b);
uetd_q15_t umath_q15_sub_sat(uetd_q15_t a, uetd_q15_t b);
uetd_q15_t umath_q15_mul_sat(uetd_q15_t a, uetd_q15_t b);
umath_status_t umath_q15_div_sat(uetd_q15_t numerator, uetd_q15_t denominator, uetd_q15_t* quotient);
uetd_q31_t umath_q31_add_sat(uetd_q31_t a, uetd_q31_t b);
uetd_q31_t umath_q31_sub_sat(uetd_q31_t a, uetd_q31_t b);
uetd_q31_t umath_q31_mul_sat(uetd_q31_t a, uetd_q31_t b);
umath_status_t umath_q31_div_sat(uetd_q31_t numerator, uetd_q31_t denominator, uetd_q31_t* quotient);
uetd_q16_t umath_q16_add_sat(uetd_q16_t a, uetd_q16_t b);
uetd_q16_t umath_q16_sub_sat(uetd_q16_t a, uetd_q16_t b);
uetd_q16_t umath_q16_mul_sat(uetd_q16_t a, uetd_q16_t b);
umath_status_t umath_q16_div_sat(uetd_q16_t numerator, uetd_q16_t denominator, uetd_q16_t* quotient);
umath_status_t umath_q15_from_scalar(umath_scalar_t value, uetd_q15_t* result);
umath_status_t umath_q31_from_scalar(umath_scalar_t value, uetd_q31_t* result);
umath_status_t umath_q16_from_scalar(umath_scalar_t value, uetd_q16_t* result);
umath_scalar_t umath_q15_to_scalar(uetd_q15_t value);
umath_scalar_t umath_q31_to_scalar(uetd_q31_t value);
umath_scalar_t umath_q16_to_scalar(uetd_q16_t value);

/// Saturating absolute values, integer square roots, and Q-format conversions.

umath_status_t umath_q15_abs_sat(uetd_q15_t value, uetd_q15_t* result);
umath_status_t umath_q15_sqrt(uetd_q15_t value, uetd_q15_t* result);
umath_status_t umath_q31_abs_sat(uetd_q31_t value, uetd_q31_t* result);
umath_status_t umath_q31_sqrt(uetd_q31_t value, uetd_q31_t* result);
umath_status_t umath_q16_abs_sat(uetd_q16_t value, uetd_q16_t* result);
umath_status_t umath_q16_sqrt(uetd_q16_t value, uetd_q16_t* result);
umath_status_t umath_q15_to_q31_sat(uetd_q15_t value, uetd_q31_t* result);
umath_status_t umath_q15_to_q16_sat(uetd_q15_t value, uetd_q16_t* result);
umath_status_t umath_q31_to_q15_sat(uetd_q31_t value, uetd_q15_t* result);
umath_status_t umath_q31_to_q16_sat(uetd_q31_t value, uetd_q16_t* result);
umath_status_t umath_q16_to_q15_sat(uetd_q16_t value, uetd_q15_t* result);
umath_status_t umath_q16_to_q31_sat(uetd_q16_t value, uetd_q31_t* result);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_FIXED_H */
