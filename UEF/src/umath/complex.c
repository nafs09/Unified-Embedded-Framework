/// @file src/umath/complex.c
/// @brief Complex arithmetic with checked division and transcendental operations.
#include <math.h>
#include <uef/umath/complex.h>
#include <uef/umath/internal/scalar_math.h>
#include <uef/umath/scalar.h>

umath_complex_t umath_complex_add(umath_complex_t a, umath_complex_t b)
{
    const umath_complex_t result = { a.real + b.real, a.imag + b.imag };
    return result;
}

umath_complex_t umath_complex_sub(umath_complex_t a, umath_complex_t b)
{
    const umath_complex_t result = { a.real - b.real, a.imag - b.imag };
    return result;
}

umath_complex_t umath_complex_mul(umath_complex_t a, umath_complex_t b)
{
    const umath_complex_t result = {
        a.real * b.real - a.imag * b.imag,
        a.real * b.imag + a.imag * b.real
    };
    return result;
}

umath_complex_t umath_complex_conjugate(umath_complex_t value)
{
    const umath_complex_t result = { value.real, -value.imag };
    return result;
}

umath_status_t umath_complex_div(umath_complex_t a, umath_complex_t b,
                                 umath_complex_t *quotient)
{
    umath_complex_t result;
    umath_accumulator_t real, imag;
    umath_scalar_t real_scalar, imag_scalar;
    if (quotient == NULL || !umath_scalar_is_finite(a.real) ||
        !umath_scalar_is_finite(a.imag) || !umath_scalar_is_finite(b.real) ||
        !umath_scalar_is_finite(b.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    if (b.real == 0 && b.imag == 0) return UMATH_SINGULAR;
    if (UEF_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)b.real) >=
        UEF_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)b.imag)) {
        const umath_accumulator_t ratio =
            (umath_accumulator_t)b.imag / (umath_accumulator_t)b.real;
        const umath_accumulator_t denominator = (umath_accumulator_t)b.real +
            (umath_accumulator_t)b.imag * ratio;
        real = ((umath_accumulator_t)a.real +
                (umath_accumulator_t)a.imag * ratio) / denominator;
        imag = ((umath_accumulator_t)a.imag -
                (umath_accumulator_t)a.real * ratio) / denominator;
    } else {
        const umath_accumulator_t ratio =
            (umath_accumulator_t)b.real / (umath_accumulator_t)b.imag;
        const umath_accumulator_t denominator = (umath_accumulator_t)b.imag +
            (umath_accumulator_t)b.real * ratio;
        real = ((umath_accumulator_t)a.real * ratio +
                (umath_accumulator_t)a.imag) / denominator;
        imag = ((umath_accumulator_t)a.imag * ratio -
                (umath_accumulator_t)a.real) / denominator;
    }
    if (!isfinite(real) || !isfinite(imag)) return UMATH_NUMERIC_FAILURE;
    real_scalar = (umath_scalar_t)real;
    imag_scalar = (umath_scalar_t)imag;
    if (!umath_scalar_is_finite(real_scalar) ||
        !umath_scalar_is_finite(imag_scalar)) return UMATH_NUMERIC_FAILURE;
    result.real = real_scalar;
    result.imag = imag_scalar;
    *quotient = result;
    return UMATH_OK;
}

umath_status_t umath_complex_magnitude(umath_complex_t value,
                                       umath_scalar_t *magnitude)
{
    const umath_accumulator_t result = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)value.real, (umath_accumulator_t)value.imag);
    umath_scalar_t converted;
    if (magnitude == NULL) return UMATH_INVALID_ARGUMENT;
    if (!umath_scalar_is_finite(value.real) ||
        !umath_scalar_is_finite(value.imag) || !isfinite(result)) {
        return UMATH_NUMERIC_FAILURE;
    }
    converted = (umath_scalar_t)result;
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *magnitude = converted;
    return UMATH_OK;
}

umath_status_t umath_complex_exp(umath_complex_t value,
                                 umath_complex_t *exponential)
{
    umath_accumulator_t scale, real, imag;
    umath_complex_t result;
    if (exponential == NULL || !umath_scalar_is_finite(value.real) ||
        !umath_scalar_is_finite(value.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    scale = UEF_UMATH_ACCUM_UNARY_MATH(exp, (umath_accumulator_t)value.real);
    real = scale * UEF_UMATH_ACCUM_UNARY_MATH(
        cos, (umath_accumulator_t)value.imag);
    imag = scale * UEF_UMATH_ACCUM_UNARY_MATH(
        sin, (umath_accumulator_t)value.imag);
    if (!isfinite(real) || !isfinite(imag)) {
        return UMATH_NUMERIC_FAILURE;
    }
    result.real = (umath_scalar_t)real;
    result.imag = (umath_scalar_t)imag;
    if (!umath_scalar_is_finite(result.real) ||
        !umath_scalar_is_finite(result.imag)) return UMATH_NUMERIC_FAILURE;
    *exponential = result;
    return UMATH_OK;
}

umath_status_t umath_complex_log(umath_complex_t value,
                                 umath_complex_t *logarithm)
{
    umath_complex_t result;
    umath_accumulator_t magnitude;
    if (logarithm == NULL || !umath_scalar_is_finite(value.real) ||
        !umath_scalar_is_finite(value.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    magnitude = UEF_UMATH_ACCUM_BINARY_MATH(
        hypot, (umath_accumulator_t)value.real, (umath_accumulator_t)value.imag);
    if (!isfinite(magnitude)) return UMATH_NUMERIC_FAILURE;
    if (magnitude == UMATH_ACCUMULATOR_C(0)) return UMATH_SINGULAR;
    result.real = (umath_scalar_t)UEF_UMATH_ACCUM_UNARY_MATH(log, magnitude);
    result.imag = (umath_scalar_t)UEF_UMATH_ACCUM_BINARY_MATH(
        atan2, (umath_accumulator_t)value.imag, (umath_accumulator_t)value.real);
    if (!umath_scalar_is_finite(result.real) || !umath_scalar_is_finite(result.imag)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *logarithm = result;
    return UMATH_OK;
}
