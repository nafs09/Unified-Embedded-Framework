/// @file src/umath/fixed.c
/// @brief Saturating fixed-point operations with explicit Q-format scaling.
#include <limits.h>
#include <math.h>
#include <uef/umath/fixed.h>
#include <uef/umath/internal/scalar_math.h>

static int16_t clamp_q15(int64_t value)
{
    if (value > INT16_MAX) return INT16_MAX;
    if (value < INT16_MIN) return INT16_MIN;
    return (int16_t)value;
}

static int32_t clamp_q31(int64_t value)
{
    if (value > INT32_MAX) return INT32_MAX;
    if (value < INT32_MIN) return INT32_MIN;
    return (int32_t)value;
}

uef_q15_t umath_q15_add_sat(uef_q15_t a, uef_q15_t b)
{ return clamp_q15((int32_t)a + (int32_t)b); }
uef_q15_t umath_q15_sub_sat(uef_q15_t a, uef_q15_t b)
{ return clamp_q15((int32_t)a - (int32_t)b); }
uef_q15_t umath_q15_mul_sat(uef_q15_t a, uef_q15_t b)
{ return clamp_q15(((int32_t)a * (int32_t)b) / (INT32_C(1) << UMATH_Q15_FRAC_BITS)); }

uef_q31_t umath_q31_add_sat(uef_q31_t a, uef_q31_t b)
{ return clamp_q31((int64_t)a + (int64_t)b); }
uef_q31_t umath_q31_sub_sat(uef_q31_t a, uef_q31_t b)
{ return clamp_q31((int64_t)a - (int64_t)b); }
uef_q31_t umath_q31_mul_sat(uef_q31_t a, uef_q31_t b)
{ return clamp_q31(((int64_t)a * (int64_t)b) / (INT64_C(1) << UMATH_Q31_FRAC_BITS)); }

uef_q16_t umath_q16_add_sat(uef_q16_t a, uef_q16_t b)
{ return clamp_q31((int64_t)a + (int64_t)b); }
uef_q16_t umath_q16_sub_sat(uef_q16_t a, uef_q16_t b)
{ return clamp_q31((int64_t)a - (int64_t)b); }
uef_q16_t umath_q16_mul_sat(uef_q16_t a, uef_q16_t b)
{ return clamp_q31(((int64_t)a * (int64_t)b) / (INT64_C(1) << UMATH_Q16_FRAC_BITS)); }

/* Compare the positive endpoint without first rounding INT32_MAX to 2^31 in
 * single precision. Once the scaled value is in range, the int32 conversion
 * is defined and half-way values round away from zero. */
#define UEF_UMATH_SCALAR_ABOVE_MAX(value, maximum) \
    _Generic((value), \
        uef_f32_t: ((maximum) < INT32_C(16777216) ? \
            ((value) > (umath_scalar_t)(maximum)) : \
            ((value) >= (umath_scalar_t)((int64_t)(maximum) + INT64_C(1)))), \
        uef_f64_t: ((value) > (umath_scalar_t)(maximum)))

static umath_status_t fixed_from_scalar(umath_scalar_t value,
                                        umath_scalar_t scale,
                                        int32_t minimum, int32_t maximum,
                                        int32_t *result)
{
    umath_scalar_t scaled, rounded;
    if (result == NULL || !umath_scalar_is_finite(value)) {
        return UMATH_INVALID_ARGUMENT;
    }
    scaled = value * scale;
    if (!isfinite(scaled)) return UMATH_NUMERIC_FAILURE;
    if (scaled < (umath_scalar_t)minimum ||
        UEF_UMATH_SCALAR_ABOVE_MAX(scaled, maximum)) return UMATH_NUMERIC_FAILURE;
    rounded = scaled < UMATH_SCALAR_C(0) ?
        UEF_UMATH_SCALAR_UNARY_MATH(ceil, scaled - UMATH_SCALAR_C(0.5)) :
        UEF_UMATH_SCALAR_UNARY_MATH(floor, scaled + UMATH_SCALAR_C(0.5));
    *result = (int32_t)rounded;
    return UMATH_OK;
}

umath_status_t umath_q15_from_scalar(umath_scalar_t value, uef_q15_t *result)
{
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_from_scalar(value, UMATH_SCALAR_C(32768),
        INT16_MIN, INT16_MAX, &converted);
    if (status == UMATH_OK) *result = (uef_q15_t)converted;
    return status;
}

umath_status_t umath_q31_from_scalar(umath_scalar_t value, uef_q31_t *result)
{
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_from_scalar(value, UMATH_SCALAR_C(2147483648),
        INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uef_q31_t)converted;
    return status;
}

umath_status_t umath_q16_from_scalar(umath_scalar_t value, uef_q16_t *result)
{
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_from_scalar(value, UMATH_SCALAR_C(65536),
        INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uef_q16_t)converted;
    return status;
}

umath_scalar_t umath_q15_to_scalar(uef_q15_t value)
{ return (umath_scalar_t)value / UMATH_SCALAR_C(32768); }
umath_scalar_t umath_q31_to_scalar(uef_q31_t value)
{ return (umath_scalar_t)value / UMATH_SCALAR_C(2147483648); }
umath_scalar_t umath_q16_to_scalar(uef_q16_t value)
{ return (umath_scalar_t)value / UMATH_SCALAR_C(65536); }
