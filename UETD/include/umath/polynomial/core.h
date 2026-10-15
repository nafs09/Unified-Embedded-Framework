/// @file include/umath/polynomial/core.h
/// @brief Allocation-free scalar polynomial evaluation and calculus.
#ifndef UETD_UMATH_POLYNOMIAL_H
#define UETD_UMATH_POLYNOMIAL_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Coefficients use descending power order. Evaluation uses Horner's method.
/// Coefficient-building operations require output capacity for the full result and disjoint output/input buffers.
/// The derivative of a constant is represented by the one-coefficient zero polynomial.
/// Addition, multiplication, and composition retain leading zero coefficients.
umath_status_t umath_polynomial_evaluate(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t x,
                                         umath_scalar_t* value);
umath_status_t umath_polynomial_derivative_evaluate(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t x,
                                                    umath_scalar_t* value);
umath_status_t umath_polynomial_integral_evaluate(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t lower,
                                                  umath_scalar_t upper, umath_scalar_t* integral);
umath_status_t umath_polynomial_derivative_coefficients(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t* output, size_t output_capacity);
umath_status_t umath_polynomial_integral_coefficients(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t integration_constant, umath_scalar_t* output, size_t output_capacity);
umath_status_t umath_polynomial_add(const umath_scalar_t* left, size_t left_degree, const umath_scalar_t* right, size_t right_degree, umath_scalar_t* output, size_t output_capacity);
umath_status_t umath_polynomial_multiply(const umath_scalar_t* left, size_t left_degree, const umath_scalar_t* right, size_t right_degree, umath_scalar_t* output, size_t output_capacity);
umath_status_t umath_polynomial_compose(const umath_scalar_t* outer, size_t outer_degree, const umath_scalar_t* inner, size_t inner_degree, umath_scalar_t* output, size_t output_capacity);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_POLYNOMIAL_H */
