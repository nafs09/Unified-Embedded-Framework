/// @file src/umath/complex.c
/// @brief Complex arithmetic with checked division and transcendental operations.
#include <math.h>
#include <umath/complex.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>

umath_complex_t umath_complex_add(umath_complex_t a, umath_complex_t b) {
    const umath_complex_t result = {a.real + b.real, a.imag + b.imag};
    return result;
}

umath_complex_t umath_complex_sub(umath_complex_t a, umath_complex_t b) {
    const umath_complex_t result = {a.real - b.real, a.imag - b.imag};
    return result;
}

umath_complex_t umath_complex_mul(umath_complex_t a, umath_complex_t b) {
    const umath_complex_t result = {a.real * b.real - a.imag * b.imag, a.real * b.imag + a.imag * b.real};
    return result;
}

umath_complex_t umath_complex_conjugate(umath_complex_t value) {
    const umath_complex_t result = {value.real, -value.imag};
    return result;
}

umath_status_t umath_complex_div(umath_complex_t a, umath_complex_t b, umath_complex_t* quotient) {
    umath_complex_t result;
    umath_accumulator_t real, imag;
    umath_scalar_t real_scalar, imag_scalar;
    if (quotient == NULL || !umath_scalar_is_finite(a.real) || !umath_scalar_is_finite(a.imag) ||
        !umath_scalar_is_finite(b.real) || !umath_scalar_is_finite(b.imag)) { return UMATH_INVALID_ARGUMENT; }
    if (b.real == 0 && b.imag == 0) return UMATH_SINGULAR;
    if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)b.real) >=
        UETD_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)b.imag)) {
        const umath_accumulator_t ratio = (umath_accumulator_t)b.imag / (umath_accumulator_t)b.real;
        const umath_accumulator_t denominator = (umath_accumulator_t)b.real + (umath_accumulator_t)b.imag * ratio;
        real = ((umath_accumulator_t)a.real + (umath_accumulator_t)a.imag * ratio) / denominator;
        imag = ((umath_accumulator_t)a.imag - (umath_accumulator_t)a.real * ratio) / denominator;
    } else {
        const umath_accumulator_t ratio = (umath_accumulator_t)b.real / (umath_accumulator_t)b.imag;
        const umath_accumulator_t denominator = (umath_accumulator_t)b.imag + (umath_accumulator_t)b.real * ratio;
        real = ((umath_accumulator_t)a.real * ratio + (umath_accumulator_t)a.imag) / denominator;
        imag = ((umath_accumulator_t)a.imag * ratio - (umath_accumulator_t)a.real) / denominator;
    }
    if (!umath_accumulator_is_finite(real) || !umath_accumulator_is_finite(imag)) return UMATH_NUMERIC_FAILURE;
    real_scalar = (umath_scalar_t)real;
    imag_scalar = (umath_scalar_t)imag;
    if (!umath_scalar_is_finite(real_scalar) || !umath_scalar_is_finite(imag_scalar)) return UMATH_NUMERIC_FAILURE;
    result.real = real_scalar;
    result.imag = imag_scalar;
    *quotient = result;
    return UMATH_OK;
}

umath_status_t umath_complex_magnitude(umath_complex_t value, umath_scalar_t* magnitude) {
    const umath_accumulator_t result = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.real,
                                                                   (umath_accumulator_t)value.imag);
    umath_scalar_t converted;
    if (magnitude == NULL) return UMATH_INVALID_ARGUMENT;
    if (!umath_scalar_is_finite(value.real) || !umath_scalar_is_finite(value.imag) || !umath_accumulator_is_finite(result)) {
        return UMATH_NUMERIC_FAILURE;
    }
    converted = (umath_scalar_t)result;
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *magnitude = converted;
    return UMATH_OK;
}

umath_status_t umath_complex_exp(umath_complex_t value, umath_complex_t* exponential) {
    umath_accumulator_t scale, real, imag;
    umath_complex_t result;
    if (exponential == NULL || !umath_scalar_is_finite(value.real) || !umath_scalar_is_finite(value.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    scale = UETD_UMATH_ACCUM_UNARY_MATH(exp, (umath_accumulator_t)value.real);
    real = scale * UETD_UMATH_ACCUM_UNARY_MATH(cos, (umath_accumulator_t)value.imag);
    imag = scale * UETD_UMATH_ACCUM_UNARY_MATH(sin, (umath_accumulator_t)value.imag);
    if (!umath_accumulator_is_finite(real) || !umath_accumulator_is_finite(imag)) { return UMATH_NUMERIC_FAILURE; }
    result.real = (umath_scalar_t)real;
    result.imag = (umath_scalar_t)imag;
    if (!umath_scalar_is_finite(result.real) || !umath_scalar_is_finite(result.imag)) return UMATH_NUMERIC_FAILURE;
    *exponential = result;
    return UMATH_OK;
}

umath_status_t umath_complex_log(umath_complex_t value, umath_complex_t* logarithm) {
    umath_complex_t result;
    umath_accumulator_t magnitude;
    if (logarithm == NULL || !umath_scalar_is_finite(value.real) || !umath_scalar_is_finite(value.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    magnitude = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.real, (umath_accumulator_t)value.imag);
    if (!umath_accumulator_is_finite(magnitude)) return UMATH_NUMERIC_FAILURE;
    if (magnitude == UMATH_ACCUMULATOR_C(0)) return UMATH_SINGULAR;
    result.real = (umath_scalar_t)UETD_UMATH_ACCUM_UNARY_MATH(log, magnitude);
    result.imag = (umath_scalar_t)UETD_UMATH_ACCUM_BINARY_MATH(atan2, (umath_accumulator_t)value.imag,
                                                              (umath_accumulator_t)value.real);
    if (!umath_scalar_is_finite(result.real) || !umath_scalar_is_finite(result.imag)) { return UMATH_NUMERIC_FAILURE; }
    *logarithm = result;
    return UMATH_OK;
}

static bool complex_is_finite(umath_complex_t value) {
    return umath_scalar_is_finite(value.real) && umath_scalar_is_finite(value.imag);
}

static umath_status_t complex_commit(umath_accumulator_t real, umath_accumulator_t imag, umath_complex_t* result) {
    umath_complex_t converted;
    if (!umath_accumulator_is_finite(real) || !umath_accumulator_is_finite(imag)) return UMATH_NUMERIC_FAILURE;
    converted.real = (umath_scalar_t)real;
    converted.imag = (umath_scalar_t)imag;
    if (!complex_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *result = converted;
    return UMATH_OK;
}

umath_status_t umath_complex_sqrt(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t magnitude, real, imag;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    magnitude = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.real, (umath_accumulator_t)value.imag);
    if (!umath_accumulator_is_finite(magnitude)) return UMATH_NUMERIC_FAILURE;
    if (value.real >= 0) {
        real = UETD_UMATH_ACCUM_UNARY_MATH(sqrt, UMATH_ACCUMULATOR_C(0.5) * magnitude +
                                                   UMATH_ACCUMULATOR_C(0.5) * (umath_accumulator_t)value.real);
        imag = real == 0 ? UMATH_ACCUMULATOR_C(0) : (umath_accumulator_t)value.imag / (UMATH_ACCUMULATOR_C(2) * real);
    } else {
        imag = UETD_UMATH_ACCUM_UNARY_MATH(sqrt, UMATH_ACCUMULATOR_C(0.5) * magnitude -
                                                   UMATH_ACCUMULATOR_C(0.5) * (umath_accumulator_t)value.real);
        if (signbit(value.imag)) imag = -imag;
        real = imag == 0 ? UMATH_ACCUMULATOR_C(0) :
               UETD_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)value.imag) / (UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(fabs, imag));
    }
    return complex_commit(real, imag, result);
}

umath_status_t umath_complex_pow(umath_complex_t base, umath_complex_t exponent, umath_complex_t* result) {
    umath_accumulator_t magnitude, log_real, log_imag, real, imag, scale, phase;
    if (result == NULL || !complex_is_finite(base) || !complex_is_finite(exponent)) return UMATH_INVALID_ARGUMENT;
    if (exponent.real == 0 && exponent.imag == 0) {
        if (base.real == 0 && base.imag == 0) return UMATH_SINGULAR;
        result->real = (umath_scalar_t)1;
        result->imag = (umath_scalar_t)0;
        return UMATH_OK;
    }
    magnitude = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)base.real, (umath_accumulator_t)base.imag);
    if (magnitude == 0) {
        if (exponent.imag == 0 && exponent.real > 0) {
            result->real = (umath_scalar_t)0;
            result->imag = (umath_scalar_t)0;
            return UMATH_OK;
        }
        return UMATH_SINGULAR;
    }
    if (!umath_accumulator_is_finite(magnitude)) return UMATH_NUMERIC_FAILURE;
    log_real = UETD_UMATH_ACCUM_UNARY_MATH(log, magnitude);
    log_imag = UETD_UMATH_ACCUM_BINARY_MATH(atan2, (umath_accumulator_t)base.imag, (umath_accumulator_t)base.real);
    real = (umath_accumulator_t)exponent.real * log_real - (umath_accumulator_t)exponent.imag * log_imag;
    imag = (umath_accumulator_t)exponent.real * log_imag + (umath_accumulator_t)exponent.imag * log_real;
    scale = UETD_UMATH_ACCUM_UNARY_MATH(exp, real);
    phase = imag;
    return complex_commit(scale * UETD_UMATH_ACCUM_UNARY_MATH(cos, phase),
                          scale * UETD_UMATH_ACCUM_UNARY_MATH(sin, phase), result);
}

umath_status_t umath_complex_argument(umath_complex_t value, umath_scalar_t* radians) {
    umath_scalar_t result;
    if (radians == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    result = (umath_scalar_t)UETD_UMATH_ACCUM_BINARY_MATH(atan2, (umath_accumulator_t)value.imag,
                                                          (umath_accumulator_t)value.real);
    if (!umath_scalar_is_finite(result)) return UMATH_NUMERIC_FAILURE;
    *radians = result;
    return UMATH_OK;
}

umath_status_t umath_complex_from_polar(umath_scalar_t magnitude, umath_scalar_t phase_radians, umath_complex_t* result) {
    umath_accumulator_t phase;
    if (result == NULL || !umath_scalar_is_finite(magnitude) || !umath_scalar_is_finite(phase_radians) || magnitude < 0)
        return UMATH_INVALID_ARGUMENT;
    phase = (umath_accumulator_t)phase_radians;
    return complex_commit((umath_accumulator_t)magnitude * UETD_UMATH_ACCUM_UNARY_MATH(cos, phase),
                          (umath_accumulator_t)magnitude * UETD_UMATH_ACCUM_UNARY_MATH(sin, phase), result);
}

umath_status_t umath_complex_to_polar(umath_complex_t value, umath_scalar_t* magnitude, umath_scalar_t* phase_radians) {
    umath_accumulator_t mag, phase;
    umath_scalar_t converted_mag, converted_phase;
    if (magnitude == NULL || phase_radians == NULL || magnitude == phase_radians || !complex_is_finite(value))
        return UMATH_INVALID_ARGUMENT;
    mag = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.real, (umath_accumulator_t)value.imag);
    phase = UETD_UMATH_ACCUM_BINARY_MATH(atan2, (umath_accumulator_t)value.imag, (umath_accumulator_t)value.real);
    converted_mag = (umath_scalar_t)mag;
    converted_phase = (umath_scalar_t)phase;
    if (!umath_accumulator_is_finite(mag) || !umath_scalar_is_finite(converted_mag) || !umath_scalar_is_finite(converted_phase))
        return UMATH_NUMERIC_FAILURE;
    *magnitude = converted_mag;
    *phase_radians = converted_phase;
    return UMATH_OK;
}

umath_status_t umath_complex_sin(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t x, y;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)value.real; y = (umath_accumulator_t)value.imag;
    return complex_commit(UETD_UMATH_ACCUM_UNARY_MATH(sin, x) * UETD_UMATH_ACCUM_UNARY_MATH(cosh, y),
                          UETD_UMATH_ACCUM_UNARY_MATH(cos, x) * UETD_UMATH_ACCUM_UNARY_MATH(sinh, y), result);
}

umath_status_t umath_complex_cos(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t x, y;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)value.real; y = (umath_accumulator_t)value.imag;
    return complex_commit(UETD_UMATH_ACCUM_UNARY_MATH(cos, x) * UETD_UMATH_ACCUM_UNARY_MATH(cosh, y),
                          -UETD_UMATH_ACCUM_UNARY_MATH(sin, x) * UETD_UMATH_ACCUM_UNARY_MATH(sinh, y), result);
}

umath_status_t umath_complex_tan(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t x, y, decay, decay2, denominator, real, imag;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)value.real; y = (umath_accumulator_t)value.imag;
    decay = UETD_UMATH_ACCUM_UNARY_MATH(exp, -UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(fabs, y));
    decay2 = decay * decay;
    denominator = UMATH_ACCUMULATOR_C(1) + UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(cos, UMATH_ACCUMULATOR_C(2) * x) * decay + decay2;
    if (denominator == 0) return UMATH_SINGULAR;
    real = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(sin, UMATH_ACCUMULATOR_C(2) * x) * decay / denominator;
    imag = (y < 0 ? -UMATH_ACCUMULATOR_C(1) : UMATH_ACCUMULATOR_C(1)) * (UMATH_ACCUMULATOR_C(1) - decay2) / denominator;
    if (y == 0) imag = 0;
    return complex_commit(real, imag, result);
}

umath_status_t umath_complex_sinh(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t x, y;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)value.real; y = (umath_accumulator_t)value.imag;
    return complex_commit(UETD_UMATH_ACCUM_UNARY_MATH(sinh, x) * UETD_UMATH_ACCUM_UNARY_MATH(cos, y),
                          UETD_UMATH_ACCUM_UNARY_MATH(cosh, x) * UETD_UMATH_ACCUM_UNARY_MATH(sin, y), result);
}

umath_status_t umath_complex_cosh(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t x, y;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)value.real; y = (umath_accumulator_t)value.imag;
    return complex_commit(UETD_UMATH_ACCUM_UNARY_MATH(cosh, x) * UETD_UMATH_ACCUM_UNARY_MATH(cos, y),
                          UETD_UMATH_ACCUM_UNARY_MATH(sinh, x) * UETD_UMATH_ACCUM_UNARY_MATH(sin, y), result);
}

umath_status_t umath_complex_tanh(umath_complex_t value, umath_complex_t* result) {
    umath_accumulator_t x, y, decay, decay2, denominator, real, imag;
    if (result == NULL || !complex_is_finite(value)) return UMATH_INVALID_ARGUMENT;
    x = (umath_accumulator_t)value.real; y = (umath_accumulator_t)value.imag;
    decay = UETD_UMATH_ACCUM_UNARY_MATH(exp, -UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(fabs, x));
    decay2 = decay * decay;
    denominator = UMATH_ACCUMULATOR_C(1) + UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(cos, UMATH_ACCUMULATOR_C(2) * y) * decay + decay2;
    if (denominator == 0) return UMATH_SINGULAR;
    real = (x < 0 ? -UMATH_ACCUMULATOR_C(1) : UMATH_ACCUMULATOR_C(1)) * (UMATH_ACCUMULATOR_C(1) - decay2) / denominator;
    imag = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(sin, UMATH_ACCUMULATOR_C(2) * y) * decay / denominator;
    if (x == 0) real = 0;
    return complex_commit(real, imag, result);
}

