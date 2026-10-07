/// @file include/uef/umath/complex.h
/// @brief Complex scalar operations using the configured UMATH precision.
#ifndef UEF_UMATH_COMPLEX_H
#define UEF_UMATH_COMPLEX_H

#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    umath_scalar_t real;
    umath_scalar_t imag;
} umath_complex_t;

/// Value-returning arithmetic does not report overflow; check each component with
/// umath_scalar_is_finite when inputs are not already range-bounded.
umath_complex_t umath_complex_add(umath_complex_t a, umath_complex_t b);
umath_complex_t umath_complex_sub(umath_complex_t a, umath_complex_t b);
umath_complex_t umath_complex_mul(umath_complex_t a, umath_complex_t b);
umath_complex_t umath_complex_conjugate(umath_complex_t value);
umath_status_t umath_complex_div(umath_complex_t numerator,
                                 umath_complex_t denominator,
                                 umath_complex_t *quotient);
umath_status_t umath_complex_magnitude(umath_complex_t value,
                                       umath_scalar_t *magnitude);
umath_status_t umath_complex_exp(umath_complex_t value,
                                 umath_complex_t *exponential);
umath_status_t umath_complex_log(umath_complex_t value,
                                 umath_complex_t *logarithm);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_COMPLEX_H */