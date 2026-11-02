/// @file include/umath/complex.h
/// @brief Complex scalar operations using the configured UMATH precision.
#ifndef UETD_UMATH_COMPLEX_H
#define UETD_UMATH_COMPLEX_H
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    umath_scalar_t real;
    umath_scalar_t imag;
} umath_complex_t;

/// Value-returning arithmetic does not report overflow; check each component with
/// umath_scalar_is_finite when inputs are not already range-bounded.
/// Log and power use the principal complex branch. Log(0) and undefined zero-base powers are singular.
/// Polar magnitude must be nonnegative; the argument of zero is defined as 0.
/// The magnitude and phase output pointers in to_polar must be distinct.
umath_complex_t umath_complex_add(umath_complex_t a, umath_complex_t b);
/// Subtract one complex value from another.
umath_complex_t umath_complex_sub(umath_complex_t a, umath_complex_t b);
/// Multiply two complex values.
umath_complex_t umath_complex_mul(umath_complex_t a, umath_complex_t b);
/// Return the complex conjugate of a value.
umath_complex_t umath_complex_conjugate(umath_complex_t value);
/// Divide one complex value by another.
umath_status_t umath_complex_div(umath_complex_t numerator, umath_complex_t denominator, umath_complex_t* quotient);
/// Compute the magnitude of a complex value.
umath_status_t umath_complex_magnitude(umath_complex_t value, umath_scalar_t* magnitude);
/// Compute the complex exponential.
umath_status_t umath_complex_exp(umath_complex_t value, umath_complex_t* exponential);
/// Compute the principal complex logarithm.
umath_status_t umath_complex_log(umath_complex_t value, umath_complex_t* logarithm);
/// Compute the principal square root of a complex value.
umath_status_t umath_complex_sqrt(umath_complex_t value, umath_complex_t* result);
/// Raise a complex value to a complex power.
umath_status_t umath_complex_pow(umath_complex_t base, umath_complex_t exponent, umath_complex_t* result);
/// Return the phase angle of a complex value in radians.
umath_status_t umath_complex_argument(umath_complex_t value, umath_scalar_t* radians);
/// Construct a complex value from its magnitude and phase.
umath_status_t umath_complex_from_polar(umath_scalar_t magnitude, umath_scalar_t phase_radians, umath_complex_t* result);
/// Convert a complex value to magnitude and phase.
umath_status_t umath_complex_to_polar(umath_complex_t value, umath_scalar_t* magnitude, umath_scalar_t* phase_radians);
/// Compute the complex sine.
umath_status_t umath_complex_sin(umath_complex_t value, umath_complex_t* result);
/// Compute the complex cosine.
umath_status_t umath_complex_cos(umath_complex_t value, umath_complex_t* result);
/// Compute the complex tangent.
umath_status_t umath_complex_tan(umath_complex_t value, umath_complex_t* result);
/// Compute the hyperbolic sine of a complex value.
umath_status_t umath_complex_sinh(umath_complex_t value, umath_complex_t* result);
/// Compute the hyperbolic cosine of a complex value.
umath_status_t umath_complex_cosh(umath_complex_t value, umath_complex_t* result);
/// Compute the hyperbolic tangent of a complex value.
umath_status_t umath_complex_tanh(umath_complex_t value, umath_complex_t* result);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_COMPLEX_H */
