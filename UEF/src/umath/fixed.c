/// @file src/umath/fixed.c
/// @brief Saturating fixed-point operations with explicit Q-format scaling.
#include <math.h>
#include <limits.h>
#include <uef/umath/fixed.h>

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

static umath_status_t fixed_from_scalar(double value, double scale,
                                        double minimum, double maximum,
                                        int32_t *result)
{
    double scaled, rounded;
    if (result == NULL || !isfinite(value)) return UMATH_INVALID_ARGUMENT;
    scaled = value * scale;
    if (!isfinite(scaled)) return UMATH_NUMERIC_FAILURE;
    if (scaled < minimum || scaled > maximum) return UMATH_NUMERIC_FAILURE;
    rounded = scaled < 0.0 ? ceil(scaled - 0.5) : floor(scaled + 0.5);
    *result = (int32_t)rounded;
    return UMATH_OK;
}

umath_status_t umath_q15_from_scalar(umath_scalar_t value, uef_q15_t *result)
{
    int32_t converted;
    const umath_status_t status = fixed_from_scalar((double)value, 32768.0,
        INT16_MIN, INT16_MAX, &converted);
    if (status == UMATH_OK && result != NULL) *result = (uef_q15_t)converted;
    return result == NULL ? UMATH_INVALID_ARGUMENT : status;
}

umath_status_t umath_q31_from_scalar(umath_scalar_t value, uef_q31_t *result)
{
    int32_t converted;
    const umath_status_t status = fixed_from_scalar((double)value, 2147483648.0,
        INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK && result != NULL) *result = (uef_q31_t)converted;
    return result == NULL ? UMATH_INVALID_ARGUMENT : status;
}

umath_status_t umath_q16_from_scalar(umath_scalar_t value, uef_q16_t *result)
{
    int32_t converted;
    const umath_status_t status = fixed_from_scalar((double)value, 65536.0,
        INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK && result != NULL) *result = (uef_q16_t)converted;
    return result == NULL ? UMATH_INVALID_ARGUMENT : status;
}

umath_scalar_t umath_q15_to_scalar(uef_q15_t value)
{ return (umath_scalar_t)((double)value / 32768.0); }
umath_scalar_t umath_q31_to_scalar(uef_q31_t value)
{ return (umath_scalar_t)((double)value / 2147483648.0); }
umath_scalar_t umath_q16_to_scalar(uef_q16_t value)
{ return (umath_scalar_t)((double)value / 65536.0); }