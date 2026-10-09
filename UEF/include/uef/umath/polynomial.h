/// @file include/uef/umath/polynomial.h
/// @brief Allocation-free scalar polynomial evaluation and calculus.
#ifndef UEF_UMATH_POLYNOMIAL_H
#define UEF_UMATH_POLYNOMIAL_H
#include <uef/umath/config.h>

#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Coefficients are in descending power order; evaluate with Horner's method.
umath_status_t umath_polynomial_evaluate(const umath_scalar_t *coefficients,
                                         size_t degree,
                                         umath_scalar_t x,
                                         umath_scalar_t *value);
umath_status_t umath_polynomial_derivative_evaluate(const umath_scalar_t *coefficients,
                                                    size_t degree,
                                                    umath_scalar_t x,
                                                    umath_scalar_t *value);
umath_status_t umath_polynomial_integral_evaluate(const umath_scalar_t *coefficients,
                                                  size_t degree,
                                                  umath_scalar_t lower,
                                                  umath_scalar_t upper,
                                                  umath_scalar_t *integral);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_POLYNOMIAL_H */