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
/// Subtract Q15 values with saturation.
uetd_q15_t umath_q15_sub_sat(uetd_q15_t a, uetd_q15_t b);
/// Multiply Q15 values with saturation.
uetd_q15_t umath_q15_mul_sat(uetd_q15_t a, uetd_q15_t b);
/// Divide Q15 values with saturation.
umath_status_t umath_q15_div_sat(uetd_q15_t numerator, uetd_q15_t denominator, uetd_q15_t* quotient);
/// Add Q31 values with saturation.
uetd_q31_t umath_q31_add_sat(uetd_q31_t a, uetd_q31_t b);
/// Subtract Q31 values with saturation.
uetd_q31_t umath_q31_sub_sat(uetd_q31_t a, uetd_q31_t b);
/// Multiply Q31 values with saturation.
uetd_q31_t umath_q31_mul_sat(uetd_q31_t a, uetd_q31_t b);
/// Divide Q31 values with saturation.
umath_status_t umath_q31_div_sat(uetd_q31_t numerator, uetd_q31_t denominator, uetd_q31_t* quotient);
/// Add Q16 values with saturation.
uetd_q16_t umath_q16_add_sat(uetd_q16_t a, uetd_q16_t b);
/// Subtract Q16 values with saturation.
uetd_q16_t umath_q16_sub_sat(uetd_q16_t a, uetd_q16_t b);
/// Multiply Q16 values with saturation.
uetd_q16_t umath_q16_mul_sat(uetd_q16_t a, uetd_q16_t b);
/// Divide Q16 values with saturation.
umath_status_t umath_q16_div_sat(uetd_q16_t numerator, uetd_q16_t denominator, uetd_q16_t* quotient);
/// Convert a scalar value to Q15 format.
umath_status_t umath_q15_from_scalar(umath_scalar_t value, uetd_q15_t* result);
/// Convert a scalar value to Q31 format.
umath_status_t umath_q31_from_scalar(umath_scalar_t value, uetd_q31_t* result);
/// Convert a scalar value to Q16 format.
umath_status_t umath_q16_from_scalar(umath_scalar_t value, uetd_q16_t* result);
/// Convert a Q15 value to scalar format.
umath_scalar_t umath_q15_to_scalar(uetd_q15_t value);
/// Convert a Q31 value to scalar format.
umath_scalar_t umath_q31_to_scalar(uetd_q31_t value);
/// Convert a Q16 value to scalar format.
umath_scalar_t umath_q16_to_scalar(uetd_q16_t value);

/// Saturating absolute values, integer square roots, and Q-format conversions.

/// Return the absolute value of a Q15 value, saturating at the positive maximum.
umath_status_t umath_q15_abs_sat(uetd_q15_t value, uetd_q15_t* result);
/// Compute the square root of a Q15 value.
umath_status_t umath_q15_sqrt(uetd_q15_t value, uetd_q15_t* result);
/// Compute the absolute value of a Q31 value with saturation.
umath_status_t umath_q31_abs_sat(uetd_q31_t value, uetd_q31_t* result);
/// Compute the square root of a Q31 value.
umath_status_t umath_q31_sqrt(uetd_q31_t value, uetd_q31_t* result);
/// Compute the absolute value of a Q16 value with saturation.
umath_status_t umath_q16_abs_sat(uetd_q16_t value, uetd_q16_t* result);
/// Compute the square root of a Q16 value.
umath_status_t umath_q16_sqrt(uetd_q16_t value, uetd_q16_t* result);
/// Convert between fixed-point formats with saturation.
umath_status_t umath_q15_to_q31_sat(uetd_q15_t value, uetd_q31_t* result);
/// Convert between fixed-point formats with saturation.
umath_status_t umath_q15_to_q16_sat(uetd_q15_t value, uetd_q16_t* result);
/// Convert between fixed-point formats with saturation.
umath_status_t umath_q31_to_q15_sat(uetd_q31_t value, uetd_q15_t* result);
/// Convert between fixed-point formats with saturation.
umath_status_t umath_q31_to_q16_sat(uetd_q31_t value, uetd_q16_t* result);
/// Convert between fixed-point formats with saturation.
umath_status_t umath_q16_to_q15_sat(uetd_q16_t value, uetd_q15_t* result);
/// Convert between fixed-point formats with saturation.
umath_status_t umath_q16_to_q31_sat(uetd_q16_t value, uetd_q31_t* result);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_FIXED_H */
