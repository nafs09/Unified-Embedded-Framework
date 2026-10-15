/// @file src/umath/derivatives/dual.c
/// @brief Checked forward-mode automatic differentiation for one seeded direction.
#include <math.h>
#include <umath/derivatives/dual.h>
#include <umath/constants.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>

static bool dual_is_finite(umath_dual_t value) {
    return umath_scalar_is_finite(value.value) && umath_scalar_is_finite(value.derivative);
}

static umath_status_t dual_commit(umath_accumulator_t value, umath_accumulator_t derivative, umath_dual_t* out) {
    umath_dual_t result;
    if (!umath_accumulator_is_finite(value) || !umath_accumulator_is_finite(derivative)) return UMATH_NUMERIC_FAILURE;
    result.value = (umath_scalar_t)value;
    result.derivative = (umath_scalar_t)derivative;
    if (!dual_is_finite(result)) return UMATH_NUMERIC_FAILURE;
    *out = result;
    return UMATH_OK;
}

static umath_status_t dual_round_to_value(umath_dual_t input, umath_dual_t* out,
                                          umath_status_t (*operation)(umath_scalar_t, umath_scalar_t*)) {
    umath_scalar_t value;
    umath_status_t status;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = operation(input.value, &value);
    if (status != UMATH_OK) return status;
    /* Rounding operations use the conventional zero-derivative subgradient,
     * including at their discontinuities. */
    return dual_commit((umath_accumulator_t)value, UMATH_ACCUMULATOR_C(0), out);
}

umath_status_t umath_dual_neg(umath_dual_t input, umath_dual_t* out) {
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    return dual_commit(-(umath_accumulator_t)input.value, -(umath_accumulator_t)input.derivative, out);
}

umath_status_t umath_dual_add(umath_dual_t a, umath_dual_t b, umath_dual_t* out) {
    umath_accumulator_t value, derivative;
    umath_dual_t result;
    if (out == NULL || !umath_scalar_is_finite(a.value) || !umath_scalar_is_finite(a.derivative) ||
        !umath_scalar_is_finite(b.value) || !umath_scalar_is_finite(b.derivative)) { return UMATH_INVALID_ARGUMENT; }
    value = (umath_accumulator_t)a.value + (umath_accumulator_t)b.value;
    derivative = (umath_accumulator_t)a.derivative + (umath_accumulator_t)b.derivative;
    if (!umath_accumulator_is_finite(value) || !umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.value = (umath_scalar_t)value;
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.value) || !umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_sub(umath_dual_t a, umath_dual_t b, umath_dual_t* out) {
    umath_accumulator_t value, derivative;
    umath_dual_t result;
    if (out == NULL || !umath_scalar_is_finite(a.value) || !umath_scalar_is_finite(a.derivative) ||
        !umath_scalar_is_finite(b.value) || !umath_scalar_is_finite(b.derivative)) { return UMATH_INVALID_ARGUMENT; }
    value = (umath_accumulator_t)a.value - (umath_accumulator_t)b.value;
    derivative = (umath_accumulator_t)a.derivative - (umath_accumulator_t)b.derivative;
    if (!umath_accumulator_is_finite(value) || !umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.value = (umath_scalar_t)value;
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.value) || !umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_mul(umath_dual_t a, umath_dual_t b, umath_dual_t* out) {
    umath_accumulator_t value, derivative;
    umath_dual_t result;
    if (out == NULL || !umath_scalar_is_finite(a.value) || !umath_scalar_is_finite(a.derivative) ||
        !umath_scalar_is_finite(b.value) || !umath_scalar_is_finite(b.derivative)) { return UMATH_INVALID_ARGUMENT; }
    value = (umath_accumulator_t)a.value * (umath_accumulator_t)b.value;
    derivative = ((umath_accumulator_t)a.derivative * (umath_accumulator_t)b.value) +
                 ((umath_accumulator_t)b.derivative * (umath_accumulator_t)a.value);
    if (!umath_accumulator_is_finite(value) || !umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.value = (umath_scalar_t)value;
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.value) || !umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_div(umath_dual_t a, umath_dual_t b, umath_dual_t* out) {
    umath_accumulator_t value, derivative;
    umath_dual_t result;
    if (out == NULL || !umath_scalar_is_finite(a.value) || !umath_scalar_is_finite(a.derivative) ||
        !umath_scalar_is_finite(b.value) || !umath_scalar_is_finite(b.derivative)) { return UMATH_INVALID_ARGUMENT; }
    if (b.value == 0) { return UMATH_SINGULAR; }
    value = (umath_accumulator_t)a.value / (umath_accumulator_t)b.value;
    derivative = ((umath_accumulator_t)a.derivative - value * (umath_accumulator_t)b.derivative) / (umath_accumulator_t)b.value;
    if (!umath_accumulator_is_finite(value) || !umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.value = (umath_scalar_t)value;
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.value) || !umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_sin(umath_dual_t input, umath_dual_t* out) {
    umath_accumulator_t derivative;
    umath_scalar_t cosine;
    umath_dual_t result;
    umath_status_t status;
    if (out == NULL || !umath_scalar_is_finite(input.value) || !umath_scalar_is_finite(input.derivative)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_sin(input.value, &result.value);
    if (status != UMATH_OK) { return status; }
    status = umath_cos(input.value, &cosine);
    if (status != UMATH_OK) { return status; }
    derivative = (umath_accumulator_t)input.derivative * (umath_accumulator_t)cosine;
    if (!umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_cos(umath_dual_t input, umath_dual_t* out) {
    umath_accumulator_t derivative;
    umath_scalar_t sine;
    umath_dual_t result;
    umath_status_t status;
    if (out == NULL || !umath_scalar_is_finite(input.value) || !umath_scalar_is_finite(input.derivative)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_cos(input.value, &result.value);
    if (status != UMATH_OK) { return status; }
    status = umath_sin(input.value, &sine);
    if (status != UMATH_OK) { return status; }
    derivative = -(umath_accumulator_t)input.derivative * (umath_accumulator_t)sine;
    if (!umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_tan(umath_dual_t input, umath_dual_t* out) {
    umath_accumulator_t derivative;
    umath_dual_t result;
    umath_status_t status;
    if (out == NULL || !umath_scalar_is_finite(input.value) || !umath_scalar_is_finite(input.derivative)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_tan(input.value, &result.value);
    if (status != UMATH_OK) { return status; }
    umath_accumulator_t tangent = (umath_accumulator_t)result.value;
    derivative = (umath_accumulator_t)input.derivative * (UMATH_ACCUMULATOR_C(1) + tangent * tangent);
    if (!umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_exp(umath_dual_t input, umath_dual_t* out) {
    umath_accumulator_t derivative;
    umath_dual_t result;
    umath_status_t status;
    if (out == NULL || !umath_scalar_is_finite(input.value) || !umath_scalar_is_finite(input.derivative)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_exp(input.value, &result.value);
    if (status != UMATH_OK) { return status; }
    umath_accumulator_t exponent = (umath_accumulator_t)result.value;
    derivative = (umath_accumulator_t)input.derivative * exponent;
    if (!umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_ln(umath_dual_t input, umath_dual_t* out) {
    umath_accumulator_t derivative;
    umath_dual_t result;
    umath_status_t status;
    if (out == NULL || !umath_scalar_is_finite(input.value) || !umath_scalar_is_finite(input.derivative)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_ln(input.value, &result.value);
    if (status != UMATH_OK) { return status; }
    derivative = (umath_accumulator_t)input.derivative / (umath_accumulator_t)input.value;
    if (!umath_accumulator_is_finite(derivative)) { return UMATH_NUMERIC_FAILURE; }
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_sqrt(umath_dual_t input, umath_dual_t* out) {
    umath_accumulator_t derivative;
    umath_dual_t result;
    umath_status_t status;
    if (out == NULL || !umath_scalar_is_finite(input.value) || !umath_scalar_is_finite(input.derivative)) {
        return UMATH_INVALID_ARGUMENT;
    }
    status = umath_sqrt(input.value, &result.value);
    if (status != UMATH_OK) { return status; }
    umath_accumulator_t sqrt = (umath_accumulator_t)result.value;
    derivative = (UMATH_ACCUMULATOR_C(0.5) * (umath_accumulator_t)input.derivative) / sqrt;
    if (!umath_accumulator_is_finite(derivative)) { return UMATH_SINGULAR; }
    result.derivative = (umath_scalar_t)derivative;
    if (!umath_scalar_is_finite(result.derivative)) { return UMATH_NUMERIC_FAILURE; }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_reciprocal(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t reciprocal;
    umath_status_t status;
    umath_accumulator_t value, derivative;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_reciprocal(input.value, &reciprocal);
    if (status != UMATH_OK) return status;
    value = (umath_accumulator_t)reciprocal;
    derivative = -value * ((umath_accumulator_t)input.derivative / (umath_accumulator_t)input.value);
    return dual_commit(value, derivative, out);
}

umath_status_t umath_dual_powi(umath_dual_t base, uetd_i32_t exponent, umath_dual_t* out) {
    umath_scalar_t value_scalar;
    umath_status_t status;
    umath_accumulator_t value, derivative;
    if (out == NULL || !dual_is_finite(base)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_powi(base.value, exponent, &value_scalar);
    if (status != UMATH_OK) return status;
    value = (umath_accumulator_t)value_scalar;
    if (exponent == 0) derivative = UMATH_ACCUMULATOR_C(0);
    else if (base.value == UMATH_SCALAR_C(0)) {
        if (exponent < 0) return UMATH_SINGULAR;
        derivative = exponent == 1 ? (umath_accumulator_t)base.derivative : UMATH_ACCUMULATOR_C(0);
    } else {
        derivative = ((umath_accumulator_t)exponent * (value / (umath_accumulator_t)base.value)) *
                     (umath_accumulator_t)base.derivative;
    }
    return dual_commit(value, derivative, out);
}

umath_status_t umath_dual_pow(umath_dual_t base, umath_dual_t exponent, umath_dual_t* out) {
    umath_scalar_t value_scalar, logarithm;
    umath_status_t status;
    umath_accumulator_t value, derivative;
    if (out == NULL || !dual_is_finite(base) || !dual_is_finite(exponent)) return UMATH_INVALID_ARGUMENT;
    if (base.value <= UMATH_SCALAR_C(0)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_pow(base.value, exponent.value, &value_scalar);
    if (status != UMATH_OK) return status;
    status = umath_ln(base.value, &logarithm);
    if (status != UMATH_OK) return status;
    value = (umath_accumulator_t)value_scalar;
    derivative = value * ((umath_accumulator_t)exponent.derivative * (umath_accumulator_t)logarithm +
                          (umath_accumulator_t)exponent.value *
                              ((umath_accumulator_t)base.derivative / (umath_accumulator_t)base.value));
    return dual_commit(value, derivative, out);
}

umath_status_t umath_dual_asin(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t denominator;
    umath_status_t status;
    umath_accumulator_t radicand;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    if (input.value < UMATH_SCALAR_C(-1) || input.value > UMATH_SCALAR_C(1)) return UMATH_INVALID_ARGUMENT;
    if (input.value == UMATH_SCALAR_C(-1) || input.value == UMATH_SCALAR_C(1)) return UMATH_SINGULAR;
    status = umath_asin(input.value, &denominator);
    if (status != UMATH_OK) return status;
    radicand = UMATH_ACCUMULATOR_C(1) - (umath_accumulator_t)input.value * (umath_accumulator_t)input.value;
    return dual_commit((umath_accumulator_t)denominator,
                       (umath_accumulator_t)input.derivative / UETD_UMATH_ACCUM_UNARY_MATH(sqrt, radicand), out);
}

umath_status_t umath_dual_acos(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t radicand;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    if (input.value < UMATH_SCALAR_C(-1) || input.value > UMATH_SCALAR_C(1)) return UMATH_INVALID_ARGUMENT;
    if (input.value == UMATH_SCALAR_C(-1) || input.value == UMATH_SCALAR_C(1)) return UMATH_SINGULAR;
    status = umath_acos(input.value, &value);
    if (status != UMATH_OK) return status;
    radicand = UMATH_ACCUMULATOR_C(1) - (umath_accumulator_t)input.value * (umath_accumulator_t)input.value;
    return dual_commit((umath_accumulator_t)value,
                       -(umath_accumulator_t)input.derivative / UETD_UMATH_ACCUM_UNARY_MATH(sqrt, radicand), out);
}

umath_status_t umath_dual_atan(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t x, derivative;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_atan(input.value, &value);
    if (status != UMATH_OK) return status;
    x = (umath_accumulator_t)input.value;
    if (x > UMATH_ACCUMULATOR_C(1) || x < UMATH_ACCUMULATOR_C(-1)) {
        const umath_accumulator_t magnitude = x < 0 ? -x : x;
        derivative = ((umath_accumulator_t)input.derivative / magnitude) /
                     (magnitude + UMATH_ACCUMULATOR_C(1) / magnitude);
    } else {
        derivative = (umath_accumulator_t)input.derivative / (UMATH_ACCUMULATOR_C(1) + x * x);
    }
    return dual_commit((umath_accumulator_t)value, derivative, out);
}

umath_status_t umath_dual_atan2(umath_dual_t y, umath_dual_t x, umath_dual_t* out) {
    umath_scalar_t value, radius;
    umath_status_t status;
    umath_accumulator_t r, derivative;
    if (out == NULL || !dual_is_finite(y) || !dual_is_finite(x)) return UMATH_INVALID_ARGUMENT;
    if ((x.value < UMATH_SCALAR_C(0) && y.value == UMATH_SCALAR_C(0)) ||
        (x.value == UMATH_SCALAR_C(0) && y.value == UMATH_SCALAR_C(0))) return UMATH_SINGULAR;
    status = umath_atan2(y.value, x.value, &value);
    if (status != UMATH_OK) return status;
    status = umath_hypot(x.value, y.value, &radius);
    if (status != UMATH_OK) return status;
    r = (umath_accumulator_t)radius;
    derivative = (((umath_accumulator_t)x.value / r) * (umath_accumulator_t)y.derivative -
                  ((umath_accumulator_t)y.value / r) * (umath_accumulator_t)x.derivative) / r;
    return dual_commit((umath_accumulator_t)value, derivative, out);
}

umath_status_t umath_dual_invsqrt(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t inverse_root;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    if (input.value == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    status = umath_invsqrt(input.value, &value);
    if (status != UMATH_OK) return status;
    inverse_root = (umath_accumulator_t)value;
    return dual_commit(inverse_root,
                       -UMATH_ACCUMULATOR_C(0.5) * inverse_root * inverse_root * inverse_root *
                           (umath_accumulator_t)input.derivative,
                       out);
}

umath_status_t umath_dual_sinh(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value, slope;
    umath_status_t status;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_sinh(input.value, &value);
    if (status != UMATH_OK) return status;
    status = umath_cosh(input.value, &slope);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value, (umath_accumulator_t)input.derivative * slope, out);
}

umath_status_t umath_dual_cosh(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value, slope;
    umath_status_t status;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_cosh(input.value, &value);
    if (status != UMATH_OK) return status;
    status = umath_sinh(input.value, &slope);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value, (umath_accumulator_t)input.derivative * slope, out);
}

umath_status_t umath_dual_tanh(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t tangent;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_tanh(input.value, &value);
    if (status != UMATH_OK) return status;
    tangent = (umath_accumulator_t)value;
    return dual_commit(tangent, (umath_accumulator_t)input.derivative *
                                    (UMATH_ACCUMULATOR_C(1) - tangent * tangent),
                       out);
}

umath_status_t umath_dual_hypot(umath_dual_t x, umath_dual_t y, umath_dual_t* out) {
    umath_scalar_t radius;
    umath_status_t status;
    umath_accumulator_t r, derivative;
    if (out == NULL || !dual_is_finite(x) || !dual_is_finite(y)) return UMATH_INVALID_ARGUMENT;
    status = umath_hypot(x.value, y.value, &radius);
    if (status != UMATH_OK) return status;
    if (radius == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    r = (umath_accumulator_t)radius;
    derivative = ((umath_accumulator_t)x.value / r) * (umath_accumulator_t)x.derivative +
                 ((umath_accumulator_t)y.value / r) * (umath_accumulator_t)y.derivative;
    return dual_commit(r, derivative, out);
}

umath_status_t umath_dual_fma(umath_dual_t left, umath_dual_t right, umath_dual_t addend, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t derivative;
    if (out == NULL || !dual_is_finite(left) || !dual_is_finite(right) || !dual_is_finite(addend))
        return UMATH_INVALID_ARGUMENT;
    status = umath_fma(left.value, right.value, addend.value, &value);
    if (status != UMATH_OK) return status;
    derivative = (umath_accumulator_t)left.derivative * (umath_accumulator_t)right.value +
                 (umath_accumulator_t)right.derivative * (umath_accumulator_t)left.value +
                 (umath_accumulator_t)addend.derivative;
    return dual_commit((umath_accumulator_t)value, derivative, out);
}

/* Elementary and piecewise extensions with explicit chain-rule policies. */
umath_status_t umath_dual_log1p(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t denominator;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_log1p(input.value, &value);
    if (status != UMATH_OK) return status;
    denominator = UMATH_ACCUMULATOR_C(1) + (umath_accumulator_t)input.value;
    return dual_commit((umath_accumulator_t)value, (umath_accumulator_t)input.derivative / denominator, out);
}

umath_status_t umath_dual_expm1(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value, slope;
    umath_status_t status;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_expm1(input.value, &value);
    if (status != UMATH_OK) return status;
    status = umath_exp(input.value, &slope);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value,
                       (umath_accumulator_t)input.derivative * (umath_accumulator_t)slope, out);
}

umath_status_t umath_dual_log2(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    const umath_accumulator_t ln_two = UETD_UMATH_ACCUM_UNARY_MATH(log, UMATH_ACCUMULATOR_C(2));
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_log2(input.value, &value);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value,
                       ((umath_accumulator_t)input.derivative / (umath_accumulator_t)input.value) / ln_two, out);
}

umath_status_t umath_dual_log10(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    const umath_accumulator_t ln_ten = UETD_UMATH_ACCUM_UNARY_MATH(log, UMATH_ACCUMULATOR_C(10));
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_log10(input.value, &value);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value,
                       ((umath_accumulator_t)input.derivative / (umath_accumulator_t)input.value) / ln_ten, out);
}

umath_status_t umath_dual_exp2(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    const umath_accumulator_t ln_two = UETD_UMATH_ACCUM_UNARY_MATH(log, UMATH_ACCUMULATOR_C(2));
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_exp2(input.value, &value);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value,
                       ((umath_accumulator_t)value * ln_two) * (umath_accumulator_t)input.derivative, out);
}

umath_status_t umath_dual_cbrt(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t root;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_cbrt(input.value, &value);
    if (status != UMATH_OK) return status;
    if (value == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    root = (umath_accumulator_t)value;
    return dual_commit(root, (umath_accumulator_t)input.derivative /
                                 (UMATH_ACCUMULATOR_C(3) * root * root), out);
}

umath_status_t umath_dual_asinh(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value, denominator;
    umath_status_t status;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_scalar_asinh(input.value, &value);
    if (status != UMATH_OK) return status;
    status = umath_hypot(UMATH_SCALAR_C(1), input.value, &denominator);
    if (status != UMATH_OK) return status;
    return dual_commit((umath_accumulator_t)value,
                       (umath_accumulator_t)input.derivative / (umath_accumulator_t)denominator, out);
}

umath_status_t umath_dual_acosh(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t x, denominator;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    if (input.value == UMATH_SCALAR_C(1)) return UMATH_SINGULAR;
    status = umath_scalar_acosh(input.value, &value);
    if (status != UMATH_OK) return status;
    x = (umath_accumulator_t)input.value;
    denominator = UETD_UMATH_ACCUM_UNARY_MATH(sqrt, x - UMATH_ACCUMULATOR_C(1)) *
                  UETD_UMATH_ACCUM_UNARY_MATH(sqrt, x + UMATH_ACCUMULATOR_C(1));
    return dual_commit((umath_accumulator_t)value,
                       (umath_accumulator_t)input.derivative / denominator, out);
}

umath_status_t umath_dual_atanh(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t x, denominator;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    if (input.value == UMATH_SCALAR_C(-1) || input.value == UMATH_SCALAR_C(1)) return UMATH_SINGULAR;
    status = umath_scalar_atanh(input.value, &value);
    if (status != UMATH_OK) return status;
    x = (umath_accumulator_t)input.value;
    denominator = (UMATH_ACCUMULATOR_C(1) - x) * (UMATH_ACCUMULATOR_C(1) + x);
    return dual_commit((umath_accumulator_t)value,
                       (umath_accumulator_t)input.derivative / denominator, out);
}

umath_status_t umath_dual_floor(umath_dual_t input, umath_dual_t* out) {
    return dual_round_to_value(input, out, umath_scalar_floor);
}

umath_status_t umath_dual_ceil(umath_dual_t input, umath_dual_t* out) {
    return dual_round_to_value(input, out, umath_scalar_ceil);
}

umath_status_t umath_dual_trunc(umath_dual_t input, umath_dual_t* out) {
    return dual_round_to_value(input, out, umath_scalar_trunc);
}

umath_status_t umath_dual_round(umath_dual_t input, umath_dual_t* out) {
    return dual_round_to_value(input, out, umath_scalar_round);
}

umath_status_t umath_dual_fmod(umath_dual_t numerator, umath_dual_t denominator, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t ratio, quotient, derivative;
    if (out == NULL || !dual_is_finite(numerator) || !dual_is_finite(denominator)) return UMATH_INVALID_ARGUMENT;
    if (denominator.value == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    status = umath_scalar_fmod(numerator.value, denominator.value, &value);
    if (status != UMATH_OK) return status;
    ratio = (umath_accumulator_t)numerator.value / (umath_accumulator_t)denominator.value;
    if (!umath_accumulator_is_finite(ratio)) return UMATH_NUMERIC_FAILURE;
    quotient = UETD_UMATH_ACCUM_UNARY_MATH(trunc, ratio);
    if (ratio == quotient && quotient != UMATH_ACCUMULATOR_C(0)) return UMATH_SINGULAR;
    derivative = (umath_accumulator_t)numerator.derivative - quotient * (umath_accumulator_t)denominator.derivative;
    return dual_commit((umath_accumulator_t)value, derivative, out);
}

umath_status_t umath_dual_remainder(umath_dual_t numerator, umath_dual_t denominator, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t ratio, quotient, lower, fraction, derivative;
    if (out == NULL || !dual_is_finite(numerator) || !dual_is_finite(denominator)) return UMATH_INVALID_ARGUMENT;
    if (denominator.value == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    status = umath_scalar_remainder(numerator.value, denominator.value, &value);
    if (status != UMATH_OK) return status;
    ratio = (umath_accumulator_t)numerator.value / (umath_accumulator_t)denominator.value;
    if (!umath_accumulator_is_finite(ratio)) return UMATH_NUMERIC_FAILURE;
    lower = UETD_UMATH_ACCUM_UNARY_MATH(floor, ratio);
    fraction = ratio - lower;
    if (fraction == UMATH_ACCUMULATOR_C(0.5)) return UMATH_SINGULAR;
    quotient = fraction < UMATH_ACCUMULATOR_C(0.5) ? lower : lower + UMATH_ACCUMULATOR_C(1);
    derivative = (umath_accumulator_t)numerator.derivative - quotient * (umath_accumulator_t)denominator.derivative;
    return dual_commit((umath_accumulator_t)value, derivative, out);
}

umath_status_t umath_dual_abs(umath_dual_t input, umath_dual_t* out) {
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    if (input.value == UMATH_SCALAR_C(0)) return UMATH_SINGULAR;
    return dual_commit((umath_accumulator_t)umath_scalar_abs(input.value),
                       input.value < UMATH_SCALAR_C(0) ? -(umath_accumulator_t)input.derivative
                                                       : (umath_accumulator_t)input.derivative,
                       out);
}

umath_status_t umath_dual_min(umath_dual_t left, umath_dual_t right, umath_dual_t* out) {
    if (out == NULL || !dual_is_finite(left) || !dual_is_finite(right)) return UMATH_INVALID_ARGUMENT;
    if (left.value == right.value) return UMATH_SINGULAR;
    *out = left.value < right.value ? left : right;
    return UMATH_OK;
}

umath_status_t umath_dual_max(umath_dual_t left, umath_dual_t right, umath_dual_t* out) {
    if (out == NULL || !dual_is_finite(left) || !dual_is_finite(right)) return UMATH_INVALID_ARGUMENT;
    if (left.value == right.value) return UMATH_SINGULAR;
    *out = left.value > right.value ? left : right;
    return UMATH_OK;
}

umath_status_t umath_dual_clamp(umath_dual_t input, umath_scalar_t minimum, umath_scalar_t maximum, umath_dual_t* out) {
    umath_dual_t result;
    if (out == NULL || !dual_is_finite(input) || !umath_scalar_is_finite(minimum) ||
        !umath_scalar_is_finite(maximum) || minimum > maximum) return UMATH_INVALID_ARGUMENT;
    if (minimum == maximum) {
        result.value = minimum;
        result.derivative = UMATH_SCALAR_C(0);
        *out = result;
        return UMATH_OK;
    }
    if (input.value == minimum || input.value == maximum) return UMATH_SINGULAR;
    if (input.value < minimum) {
        result.value = minimum;
        result.derivative = UMATH_SCALAR_C(0);
    } else if (input.value > maximum) {
        result.value = maximum;
        result.derivative = UMATH_SCALAR_C(0);
    } else {
        result = input;
    }
    *out = result;
    return UMATH_OK;
}

umath_status_t umath_dual_wrap_angle_pi(umath_dual_t input, umath_dual_t* out) {
    umath_scalar_t value;
    umath_status_t status;
    umath_accumulator_t reduced;
    if (out == NULL || !dual_is_finite(input)) return UMATH_INVALID_ARGUMENT;
    status = umath_wrap_angle_pi(input.value, &value);
    if (status != UMATH_OK) return status;
    reduced = UETD_UMATH_ACCUM_BINARY_MATH(fmod, (umath_accumulator_t)input.value, UMATH_TAU);
    if (reduced >= UMATH_PI) reduced -= UMATH_TAU;
    else if (reduced < -UMATH_PI) reduced += UMATH_TAU;
    if (reduced == -UMATH_PI || reduced == UMATH_PI) return UMATH_SINGULAR;
    return dual_commit((umath_accumulator_t)value, (umath_accumulator_t)input.derivative, out);
}

