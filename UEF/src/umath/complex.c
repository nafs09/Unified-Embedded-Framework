/// @file src/umath/complex.c
/// @brief Complex arithmetic with checked division and transcendental operations.
#include <math.h>
#include <uef/umath/complex.h>
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
    double real, imag;
    if (quotient == NULL || !umath_scalar_is_finite(a.real) ||
        !umath_scalar_is_finite(a.imag) || !umath_scalar_is_finite(b.real) ||
        !umath_scalar_is_finite(b.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    if (b.real == 0 && b.imag == 0) return UMATH_SINGULAR;
    if (fabs((double)b.real) >= fabs((double)b.imag)) {
        const double ratio = (double)b.imag / (double)b.real;
        const double denominator = (double)b.real + (double)b.imag * ratio;
        real = ((double)a.real + (double)a.imag * ratio) / denominator;
        imag = ((double)a.imag - (double)a.real * ratio) / denominator;
    } else {
        const double ratio = (double)b.real / (double)b.imag;
        const double denominator = (double)b.imag + (double)b.real * ratio;
        real = ((double)a.real * ratio + (double)a.imag) / denominator;
        imag = ((double)a.imag * ratio - (double)a.real) / denominator;
    }
    if (!isfinite(real) || !isfinite(imag)) return UMATH_NUMERIC_FAILURE;
    result.real = (umath_scalar_t)real;
    result.imag = (umath_scalar_t)imag;
    *quotient = result;
    return UMATH_OK;
}

umath_status_t umath_complex_magnitude(umath_complex_t value,
                                       umath_scalar_t *magnitude)
{
    const double result = hypot((double)value.real, (double)value.imag);
    if (magnitude == NULL || !umath_scalar_is_finite(value.real) ||
        !umath_scalar_is_finite(value.imag) || !isfinite(result)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *magnitude = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_complex_exp(umath_complex_t value,
                                 umath_complex_t *exponential)
{
    const double scale = exp((double)value.real);
    const double real = scale * cos((double)value.imag);
    const double imag = scale * sin((double)value.imag);
    if (exponential == NULL || !umath_scalar_is_finite(value.real) ||
        !umath_scalar_is_finite(value.imag) || !isfinite(real) || !isfinite(imag)) {
        return UMATH_NUMERIC_FAILURE;
    }
    exponential->real = (umath_scalar_t)real;
    exponential->imag = (umath_scalar_t)imag;
    return UMATH_OK;
}

umath_status_t umath_complex_log(umath_complex_t value,
                                 umath_complex_t *logarithm)
{
    umath_complex_t result;
    const double magnitude = hypot((double)value.real, (double)value.imag);
    if (logarithm == NULL || !umath_scalar_is_finite(value.real) ||
        !umath_scalar_is_finite(value.imag)) {
        return UMATH_INVALID_ARGUMENT;
    }
    if (magnitude == 0.0) return UMATH_SINGULAR;
    result.real = (umath_scalar_t)log(magnitude);
    result.imag = (umath_scalar_t)atan2((double)value.imag, (double)value.real);
    if (!umath_scalar_is_finite(result.real) || !umath_scalar_is_finite(result.imag)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *logarithm = result;
    return UMATH_OK;
}