/// @file src/umath/fixed.c
/// @brief Saturating fixed-point operations with explicit Q-format scaling.
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <umath/fixed.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>

static int16_t clamp_q15(int64_t value) {
    if (value > INT16_MAX) return INT16_MAX;
    if (value < INT16_MIN) return INT16_MIN;
    return (int16_t)value;
}

static int32_t clamp_q31(int64_t value) {
    if (value > INT32_MAX) return INT32_MAX;
    if (value < INT32_MIN) return INT32_MIN;
    return (int32_t)value;
}

uetd_q15_t umath_q15_add_sat(uetd_q15_t a, uetd_q15_t b) { return clamp_q15((int32_t)a + (int32_t)b); }

uetd_q15_t umath_q15_sub_sat(uetd_q15_t a, uetd_q15_t b) { return clamp_q15((int32_t)a - (int32_t)b); }

uetd_q15_t umath_q15_mul_sat(uetd_q15_t a, uetd_q15_t b) {
    return clamp_q15(((int32_t)a * (int32_t)b) / (INT32_C(1) << UMATH_Q15_FRAC_BITS));
}

umath_status_t umath_q15_div_sat(uetd_q15_t numerator, uetd_q15_t denominator, uetd_q15_t* quotient) {
    if (quotient == NULL) return UMATH_INVALID_ARGUMENT;
    if (denominator == 0) return UMATH_SINGULAR;
    *quotient = clamp_q15(((int64_t)numerator * (INT64_C(1) << UMATH_Q15_FRAC_BITS)) / denominator);
    return UMATH_OK;
}

uetd_q31_t umath_q31_add_sat(uetd_q31_t a, uetd_q31_t b) { return clamp_q31((int64_t)a + (int64_t)b); }

uetd_q31_t umath_q31_sub_sat(uetd_q31_t a, uetd_q31_t b) { return clamp_q31((int64_t)a - (int64_t)b); }

uetd_q31_t umath_q31_mul_sat(uetd_q31_t a, uetd_q31_t b) {
    return clamp_q31(((int64_t)a * (int64_t)b) / (INT64_C(1) << UMATH_Q31_FRAC_BITS));
}

umath_status_t umath_q31_div_sat(uetd_q31_t numerator, uetd_q31_t denominator, uetd_q31_t* quotient) {
    int32_t value;
    if (quotient == NULL) return UMATH_INVALID_ARGUMENT;
    if (denominator == 0) return UMATH_SINGULAR;
    value = clamp_q31(((int64_t)numerator * (INT64_C(1) << UMATH_Q31_FRAC_BITS)) / denominator);
    *quotient = value;
    return UMATH_OK;
}

uetd_q16_t umath_q16_add_sat(uetd_q16_t a, uetd_q16_t b) { return clamp_q31((int64_t)a + (int64_t)b); }

uetd_q16_t umath_q16_sub_sat(uetd_q16_t a, uetd_q16_t b) { return clamp_q31((int64_t)a - (int64_t)b); }

uetd_q16_t umath_q16_mul_sat(uetd_q16_t a, uetd_q16_t b) {
    return clamp_q31(((int64_t)a * (int64_t)b) / (INT64_C(1) << UMATH_Q16_FRAC_BITS));
}

umath_status_t umath_q16_div_sat(uetd_q16_t numerator, uetd_q16_t denominator, uetd_q16_t* quotient) {
    int32_t value;
    if (quotient == NULL) return UMATH_INVALID_ARGUMENT;
    if (denominator == 0) return UMATH_SINGULAR;
    value = clamp_q31(((int64_t)numerator * (INT64_C(1) << UMATH_Q16_FRAC_BITS)) / denominator);
    *quotient = value;
    return UMATH_OK;
}

/* Compare the positive endpoint without first rounding INT32_MAX to 2^31 in
 * single precision. Once the scaled value is in range, the int32 conversion
 * is defined and half-way values round away from zero. */
#define UETD_UMATH_SCALAR_ABOVE_MAX(value, maximum)                                                                               \
    _Generic(+(value),                                                                                                           \
        uetd_f32_t: ((maximum) < INT32_C(16777216) ? ((value) > (umath_scalar_t)(maximum))                                        \
                                                  : ((value) >= (umath_scalar_t)((int64_t)(maximum) + INT64_C(1)))),             \
        uetd_f64_t: ((value) > (umath_scalar_t)(maximum)))

static umath_status_t fixed_from_scalar(umath_scalar_t value, umath_scalar_t scale, int32_t minimum, int32_t maximum,
                                        int32_t* result) {
    umath_scalar_t scaled, rounded;
    if (result == NULL || !umath_scalar_is_finite(value)) { return UMATH_INVALID_ARGUMENT; }
    scaled = value * scale;
    if (!umath_scalar_is_finite(scaled)) return UMATH_NUMERIC_FAILURE;
    if (scaled < (umath_scalar_t)minimum || UETD_UMATH_SCALAR_ABOVE_MAX(scaled, maximum)) return UMATH_NUMERIC_FAILURE;
    rounded = scaled < UMATH_SCALAR_C(0) ? UETD_UMATH_SCALAR_UNARY_MATH(ceil, scaled - UMATH_SCALAR_C(0.5))
                                         : UETD_UMATH_SCALAR_UNARY_MATH(floor, scaled + UMATH_SCALAR_C(0.5));
    *result = (int32_t)rounded;
    return UMATH_OK;
}

umath_status_t umath_q15_from_scalar(umath_scalar_t value, uetd_q15_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_from_scalar(value, UMATH_SCALAR_C(32768), INT16_MIN, INT16_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q15_t)converted;
    return status;
}

umath_status_t umath_q31_from_scalar(umath_scalar_t value, uetd_q31_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_from_scalar(value, UMATH_SCALAR_C(2147483648), INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q31_t)converted;
    return status;
}

umath_status_t umath_q16_from_scalar(umath_scalar_t value, uetd_q16_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_from_scalar(value, UMATH_SCALAR_C(65536), INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q16_t)converted;
    return status;
}

umath_scalar_t umath_q15_to_scalar(uetd_q15_t value) { return (umath_scalar_t)value / UMATH_SCALAR_C(32768); }

umath_scalar_t umath_q31_to_scalar(uetd_q31_t value) { return (umath_scalar_t)value / UMATH_SCALAR_C(2147483648); }

umath_scalar_t umath_q16_to_scalar(uetd_q16_t value) { return (umath_scalar_t)value / UMATH_SCALAR_C(65536); }

static uint64_t integer_sqrt_u64(uint64_t value) {
    uint64_t root = 0U;
    uint64_t bit = UINT64_C(1) << 62;
    while (bit > value) bit >>= 2;
    while (bit != 0U) {
        if (value >= root + bit) {
            value -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

static umath_status_t fixed_sqrt(int32_t raw_value, unsigned fractional_bits, int32_t* result) {
    uint64_t radicand;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    if (raw_value < 0) return UMATH_INVALID_ARGUMENT;
    radicand = (uint64_t)(uint32_t)raw_value << fractional_bits;
    *result = (int32_t)integer_sqrt_u64(radicand);
    return UMATH_OK;
}

static int32_t saturate_i64(int64_t value, int32_t minimum, int32_t maximum) {
    if (value > maximum) return maximum;
    if (value < minimum) return minimum;
    return (int32_t)value;
}

static umath_status_t convert_q_format(int32_t value, unsigned source_fractional_bits,
                                       unsigned destination_fractional_bits, int32_t minimum,
                                       int32_t maximum, int32_t* result) {
    int64_t converted;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    if (destination_fractional_bits > source_fractional_bits) {
        const unsigned shift = destination_fractional_bits - source_fractional_bits;
        converted = (int64_t)value * (INT64_C(1) << shift);
    } else if (destination_fractional_bits < source_fractional_bits) {
        const unsigned shift = source_fractional_bits - destination_fractional_bits;
        const int64_t magnitude = value < 0 ? -(int64_t)value : (int64_t)value;
        const uint64_t divisor = UINT64_C(1) << shift;
        uint64_t quotient = (uint64_t)magnitude / divisor;
        const uint64_t remainder = (uint64_t)magnitude % divisor;
        if (remainder >= (divisor >> 1)) ++quotient;
        converted = value < 0 ? -(int64_t)quotient : (int64_t)quotient;
    } else {
        converted = value;
    }
    *result = saturate_i64(converted, minimum, maximum);
    return UMATH_OK;
}

umath_status_t umath_q15_abs_sat(uetd_q15_t value, uetd_q15_t* result) {
    int32_t magnitude;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    magnitude = value < 0 ? -(int32_t)value : (int32_t)value;
    if (magnitude > INT16_MAX) magnitude = INT16_MAX;
    *result = (uetd_q15_t)magnitude;
    return UMATH_OK;
}

umath_status_t umath_q15_sqrt(uetd_q15_t value, uetd_q15_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_sqrt((int32_t)value, UMATH_Q15_FRAC_BITS, &converted);
    if (status == UMATH_OK) *result = (uetd_q15_t)converted;
    return status;
}

umath_status_t umath_q31_abs_sat(uetd_q31_t value, uetd_q31_t* result) {
    int64_t magnitude;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    magnitude = value < 0 ? -(int64_t)value : (int64_t)value;
    if (magnitude > INT32_MAX) magnitude = INT32_MAX;
    *result = (uetd_q31_t)magnitude;
    return UMATH_OK;
}

umath_status_t umath_q31_sqrt(uetd_q31_t value, uetd_q31_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_sqrt(value, UMATH_Q31_FRAC_BITS, &converted);
    if (status == UMATH_OK) *result = (uetd_q31_t)converted;
    return status;
}

umath_status_t umath_q16_abs_sat(uetd_q16_t value, uetd_q16_t* result) {
    int64_t magnitude;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    magnitude = value < 0 ? -(int64_t)value : (int64_t)value;
    if (magnitude > INT32_MAX) magnitude = INT32_MAX;
    *result = (uetd_q16_t)magnitude;
    return UMATH_OK;
}

umath_status_t umath_q16_sqrt(uetd_q16_t value, uetd_q16_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = fixed_sqrt(value, UMATH_Q16_FRAC_BITS, &converted);
    if (status == UMATH_OK) *result = (uetd_q16_t)converted;
    return status;
}

umath_status_t umath_q15_to_q31_sat(uetd_q15_t value, uetd_q31_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = convert_q_format((int32_t)value, UMATH_Q15_FRAC_BITS, UMATH_Q31_FRAC_BITS,
                              INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q31_t)converted;
    return status;
}

umath_status_t umath_q15_to_q16_sat(uetd_q15_t value, uetd_q16_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = convert_q_format((int32_t)value, UMATH_Q15_FRAC_BITS, UMATH_Q16_FRAC_BITS,
                              INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q16_t)converted;
    return status;
}

umath_status_t umath_q31_to_q15_sat(uetd_q31_t value, uetd_q15_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = convert_q_format(value, UMATH_Q31_FRAC_BITS, UMATH_Q15_FRAC_BITS, INT16_MIN, INT16_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q15_t)converted;
    return status;
}

umath_status_t umath_q31_to_q16_sat(uetd_q31_t value, uetd_q16_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = convert_q_format(value, UMATH_Q31_FRAC_BITS, UMATH_Q16_FRAC_BITS, INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q16_t)converted;
    return status;
}

umath_status_t umath_q16_to_q15_sat(uetd_q16_t value, uetd_q15_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = convert_q_format(value, UMATH_Q16_FRAC_BITS, UMATH_Q15_FRAC_BITS, INT16_MIN, INT16_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q15_t)converted;
    return status;
}

umath_status_t umath_q16_to_q31_sat(uetd_q16_t value, uetd_q31_t* result) {
    int32_t converted;
    umath_status_t status;
    if (result == NULL) return UMATH_INVALID_ARGUMENT;
    status = convert_q_format(value, UMATH_Q16_FRAC_BITS, UMATH_Q31_FRAC_BITS, INT32_MIN, INT32_MAX, &converted);
    if (status == UMATH_OK) *result = (uetd_q31_t)converted;
    return status;
}

