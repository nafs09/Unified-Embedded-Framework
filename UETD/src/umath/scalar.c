/// @file src/umath/scalar.c
/// @brief Portable checked scalar operations; target modules may provide verified accelerators.
#include <umath/internal/scalar_math.h>
#include <umath/constants.h>
#include <umath/scalar.h>

static umath_status_t store_finite(umath_accumulator_t value, umath_scalar_t* result) {
    umath_scalar_t converted;
    if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
    converted = (umath_scalar_t)value;
    if (!umath_scalar_is_finite(converted)) { return UMATH_NUMERIC_FAILURE; }
    *result = converted;
    return UMATH_OK;
}

bool umath_scalar_is_finite(umath_scalar_t value) { return isfinite(value) != 0; }

bool umath_accumulator_is_finite(umath_accumulator_t value) { return isfinite(value) != 0; }

umath_status_t umath_scalar_neg(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(-(umath_accumulator_t)value, result);
}

umath_status_t umath_scalar_add(umath_scalar_t left, umath_scalar_t right, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(left) || !umath_scalar_is_finite(right)) return UMATH_INVALID_ARGUMENT;
    return store_finite((umath_accumulator_t)left + (umath_accumulator_t)right, result);
}

umath_status_t umath_scalar_sub(umath_scalar_t left, umath_scalar_t right, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(left) || !umath_scalar_is_finite(right)) return UMATH_INVALID_ARGUMENT;
    return store_finite((umath_accumulator_t)left - (umath_accumulator_t)right, result);
}

umath_status_t umath_scalar_mul(umath_scalar_t left, umath_scalar_t right, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(left) || !umath_scalar_is_finite(right)) return UMATH_INVALID_ARGUMENT;
    return store_finite((umath_accumulator_t)left * (umath_accumulator_t)right, result);
}

umath_status_t umath_scalar_div(umath_scalar_t numerator, umath_scalar_t denominator, umath_scalar_t* quotient) {
    if (quotient == NULL || !umath_scalar_is_finite(numerator) || !umath_scalar_is_finite(denominator))
        return UMATH_INVALID_ARGUMENT;
    if (denominator == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    return store_finite((umath_accumulator_t)numerator / (umath_accumulator_t)denominator, quotient);
}

umath_status_t umath_scalar_reciprocal(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    if (value == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    return store_finite(UMATH_ACCUMULATOR_C(1) / (umath_accumulator_t)value, result);
}

umath_status_t umath_scalar_powi(umath_scalar_t base, uetd_i32_t exponent, umath_scalar_t* result) {
    umath_accumulator_t factor, value = UMATH_ACCUMULATOR_C(1);
    uetd_u32_t power;
    if (result == NULL || !umath_scalar_is_finite(base)) return UMATH_INVALID_ARGUMENT;
    if (exponent < 0 && base == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    power = exponent < 0 ? (uetd_u32_t)(-(int64_t)exponent) : (uetd_u32_t)exponent;
    factor = (umath_accumulator_t)base;
    if (exponent < 0) factor = UMATH_ACCUMULATOR_C(1) / factor;
    while (power != 0U) {
        if ((power & 1U) != 0U) {
            value *= factor;
            if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
        }
        power >>= 1U;
        if (power != 0U) {
            factor *= factor;
            if (!umath_accumulator_is_finite(factor)) return UMATH_NUMERIC_FAILURE;
        }
    }
    return store_finite(value, result);
}

umath_status_t umath_scalar_pow(umath_scalar_t base, umath_scalar_t exponent, umath_scalar_t* result) {
    umath_accumulator_t base_acc, exponent_acc;
    bool negative_result = false;
    if (result == NULL || !umath_scalar_is_finite(base) || !umath_scalar_is_finite(exponent)) return UMATH_INVALID_ARGUMENT;
    if (base == UMATH_SCALAR_C(0)) {
        if (exponent < UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
        if (exponent == UMATH_SCALAR_C(0)) return store_finite(UMATH_ACCUMULATOR_C(1), result);
        return store_finite(UMATH_ACCUMULATOR_C(0), result);
    }
    if (base < UMATH_SCALAR_C(0)) {
        if (UETD_UMATH_SCALAR_UNARY_MATH(trunc, exponent) != exponent) return UMATH_INVALID_ARGUMENT;
        negative_result = UETD_UMATH_SCALAR_BINARY_MATH(fmod, exponent, UMATH_SCALAR_C(2)) != UMATH_SCALAR_C(0);
        base_acc = -(umath_accumulator_t)base;
    } else {
        base_acc = (umath_accumulator_t)base;
    }
    exponent_acc = (umath_accumulator_t)exponent;
    umath_accumulator_t value = UETD_UMATH_ACCUM_BINARY_MATH(pow, base_acc, exponent_acc);
    if (negative_result) value = -value;
    return store_finite(value, result);
}

umath_status_t umath_scalar_clamp(umath_scalar_t value, umath_scalar_t minimum, umath_scalar_t maximum, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || !umath_scalar_is_finite(minimum) ||
        !umath_scalar_is_finite(maximum) || minimum > maximum) { return UMATH_INVALID_ARGUMENT; }
    *result = value < minimum ? minimum : (value > maximum ? maximum : value);
    return UMATH_OK;
}

umath_status_t umath_sin(umath_scalar_t radians, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(radians)) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(sin, (umath_accumulator_t)radians), result);
}

umath_status_t umath_cos(umath_scalar_t radians, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(radians)) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(cos, (umath_accumulator_t)radians), result);
}

umath_status_t umath_tan(umath_scalar_t radians, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(radians)) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(tan, (umath_accumulator_t)radians), result);
}

umath_status_t umath_atan2(umath_scalar_t y, umath_scalar_t x, umath_scalar_t* radians) {
    if (radians == NULL || !umath_scalar_is_finite(x) || !umath_scalar_is_finite(y)) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_BINARY_MATH(atan2, (umath_accumulator_t)y, (umath_accumulator_t)x), radians);
}

umath_status_t umath_asin(umath_scalar_t value, umath_scalar_t* radians) {
    if (radians == NULL || !umath_scalar_is_finite(value) || value < UMATH_SCALAR_C(-1) || value > UMATH_SCALAR_C(1))
        return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(asin, (umath_accumulator_t)value), radians);
}

umath_status_t umath_acos(umath_scalar_t value, umath_scalar_t* radians) {
    if (radians == NULL || !umath_scalar_is_finite(value) || value < UMATH_SCALAR_C(-1) || value > UMATH_SCALAR_C(1))
        return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(acos, (umath_accumulator_t)value), radians);
}

umath_status_t umath_atan(umath_scalar_t value, umath_scalar_t* radians) {
    if (radians == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(atan, (umath_accumulator_t)value), radians);
}

umath_status_t umath_sqrt(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value < 0) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(sqrt, (umath_accumulator_t)value), result);
}

umath_status_t umath_invsqrt(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value <= 0) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UMATH_ACCUMULATOR_C(1) / UETD_UMATH_ACCUM_UNARY_MATH(sqrt, (umath_accumulator_t)value), result);
}

umath_status_t umath_exp(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(exp, (umath_accumulator_t)value), result);
}

umath_status_t umath_ln(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value <= 0) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(log, (umath_accumulator_t)value), result);
}

umath_status_t umath_sinh(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(sinh, (umath_accumulator_t)value), result);
}

umath_status_t umath_cosh(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(cosh, (umath_accumulator_t)value), result);
}

umath_status_t umath_tanh(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(tanh, (umath_accumulator_t)value), result);
}

umath_status_t umath_hypot(umath_scalar_t x, umath_scalar_t y, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(x) || !umath_scalar_is_finite(y)) { return UMATH_INVALID_ARGUMENT; }
    return store_finite(UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)x, (umath_accumulator_t)y), result);
}

umath_status_t umath_fma(umath_scalar_t left, umath_scalar_t right, umath_scalar_t addend, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(left) || !umath_scalar_is_finite(right) || !umath_scalar_is_finite(addend)) {
        return UMATH_INVALID_ARGUMENT;
    }
    return store_finite(UETD_UMATH_ACCUM_FMA((umath_accumulator_t)left, (umath_accumulator_t)right, (umath_accumulator_t)addend),
                        result);
}

umath_status_t umath_wrap_angle_pi(umath_scalar_t radians, umath_scalar_t* wrapped) {
    umath_accumulator_t value;
    if (wrapped == NULL || !umath_scalar_is_finite(radians)) { return UMATH_INVALID_ARGUMENT; }
    value = UETD_UMATH_ACCUM_BINARY_MATH(fmod, (umath_accumulator_t)radians, UMATH_TAU);
    if (value >= UMATH_PI) value -= UMATH_TAU;
    else if (value < -UMATH_PI) value += UMATH_TAU;
    return store_finite(value, wrapped);
}

/* Stable small-angle series are used below this threshold. The retained
 * terms keep truncation below accumulator precision near zero. */
#define UMATH_SMALL_ANGLE_THRESHOLD UMATH_ACCUMULATOR_C(0.1)

umath_status_t umath_scalar_log1p(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value <= UMATH_SCALAR_C(-1)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(log1p, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_expm1(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(expm1, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_log2(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value <= UMATH_SCALAR_C(0)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(log2, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_log10(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value <= UMATH_SCALAR_C(0)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(log10, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_exp2(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(exp2, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_cbrt(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(cbrt, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_floor(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(floor, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_ceil(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(ceil, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_trunc(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(trunc, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_round(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    /* C round() resolves exact half-way cases away from zero. */
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(round, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_asinh(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(asinh, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_acosh(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value < UMATH_SCALAR_C(1)) return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(acosh, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_atanh(umath_scalar_t value, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(value) || value <= UMATH_SCALAR_C(-1) || value >= UMATH_SCALAR_C(1))
        return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_UNARY_MATH(atanh, (umath_accumulator_t)value), result);
}

umath_status_t umath_scalar_fmod(umath_scalar_t numerator, umath_scalar_t denominator, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(numerator) || !umath_scalar_is_finite(denominator))
        return UMATH_INVALID_ARGUMENT;
    if (denominator == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    return store_finite(UETD_UMATH_ACCUM_BINARY_MATH(fmod, (umath_accumulator_t)numerator,
                                                       (umath_accumulator_t)denominator), result);
}

umath_status_t umath_scalar_remainder(umath_scalar_t numerator, umath_scalar_t denominator, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(numerator) || !umath_scalar_is_finite(denominator))
        return UMATH_INVALID_ARGUMENT;
    if (denominator == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    return store_finite(UETD_UMATH_ACCUM_BINARY_MATH(remainder, (umath_accumulator_t)numerator,
                                                       (umath_accumulator_t)denominator), result);
}

umath_status_t umath_scalar_modf(umath_scalar_t value, umath_scalar_t* integral, umath_scalar_t* fractional) {
    umath_accumulator_t whole, fraction;
    umath_scalar_t converted_whole, converted_fraction;
    if (integral == NULL || fractional == NULL || integral == fractional || !umath_scalar_is_finite(value))
        return UMATH_INVALID_ARGUMENT;
    fraction = _Generic((umath_accumulator_t)0, uetd_f32_t: modff, uetd_f64_t: modf)((umath_accumulator_t)value, &whole);
    if (store_finite(whole, &converted_whole) != UMATH_OK ||
        store_finite(fraction, &converted_fraction) != UMATH_OK) return UMATH_NUMERIC_FAILURE;
    *integral = converted_whole;
    *fractional = converted_fraction;
    return UMATH_OK;
}

umath_status_t umath_scalar_copysign(umath_scalar_t magnitude, umath_scalar_t sign, umath_scalar_t* result) {
    if (result == NULL || !umath_scalar_is_finite(magnitude) || !umath_scalar_is_finite(sign))
        return UMATH_INVALID_ARGUMENT;
    return store_finite(UETD_UMATH_ACCUM_BINARY_MATH(copysign, (umath_accumulator_t)magnitude,
                                                       (umath_accumulator_t)sign), result);
}

umath_status_t umath_sinc(umath_scalar_t radians, umath_scalar_t* result) {
    umath_accumulator_t x, x2, value;
    if (result == NULL || !umath_scalar_is_finite(radians)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)radians;
    if (x == UMATH_ACCUMULATOR_C(0)) return store_finite(UMATH_ACCUMULATOR_C(1), result);
    if (x < UMATH_SMALL_ANGLE_THRESHOLD && x > -UMATH_SMALL_ANGLE_THRESHOLD) {
        x2 = x * x;
        value = UMATH_ACCUMULATOR_C(1) + x2 * (-UMATH_ACCUMULATOR_C(1) / 6 +
                x2 * (UMATH_ACCUMULATOR_C(1) / 120 +
                x2 * (-UMATH_ACCUMULATOR_C(1) / 5040 + x2 / 362880)));
    } else {
        value = UETD_UMATH_ACCUM_UNARY_MATH(sin, x) / x;
    }
    return store_finite(value, result);
}

umath_status_t umath_cosc(umath_scalar_t radians, umath_scalar_t* result) {
    umath_accumulator_t x, x2, value;
    if (result == NULL || !umath_scalar_is_finite(radians)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)radians;
    if (x == UMATH_ACCUMULATOR_C(0)) return store_finite(UMATH_ACCUMULATOR_C(0.5), result);
    if (x < UMATH_SMALL_ANGLE_THRESHOLD && x > -UMATH_SMALL_ANGLE_THRESHOLD) {
        x2 = x * x;
        value = UMATH_ACCUMULATOR_C(0.5) + x2 * (-UMATH_ACCUMULATOR_C(1) / 24 +
                x2 * (UMATH_ACCUMULATOR_C(1) / 720 +
                x2 * (-UMATH_ACCUMULATOR_C(1) / 40320 + x2 / 3628800)));
    } else {
        const umath_accumulator_t half_sine = UETD_UMATH_ACCUM_UNARY_MATH(sin, x / 2);
        value = (UMATH_ACCUMULATOR_C(2) * half_sine * half_sine) / (x * x);
    }
    return store_finite(value, result);
}

umath_status_t umath_sincc(umath_scalar_t radians, umath_scalar_t* result) {
    umath_accumulator_t x, x2, value;
    if (result == NULL || !umath_scalar_is_finite(radians)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)radians;
    if (x == UMATH_ACCUMULATOR_C(0)) return store_finite(UMATH_ACCUMULATOR_C(1) / 6, result);
    if (x < UMATH_SMALL_ANGLE_THRESHOLD && x > -UMATH_SMALL_ANGLE_THRESHOLD) {
        x2 = x * x;
        value = UMATH_ACCUMULATOR_C(1) / 6 + x2 * (-UMATH_ACCUMULATOR_C(1) / 120 +
                x2 * (UMATH_ACCUMULATOR_C(1) / 5040 +
                x2 * (-UMATH_ACCUMULATOR_C(1) / 362880 + x2 / 39916800)));
    } else {
        value = (x - UETD_UMATH_ACCUM_UNARY_MATH(sin, x)) / (x * x * x);
    }
    return store_finite(value, result);
}

