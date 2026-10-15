/// @file include/umath/polynomial/roots.h
/// @brief Planned bounded real and complex polynomial root solvers.
#ifndef UETD_UMATH_POLYNOMIAL_ROOTS_H
#define UETD_UMATH_POLYNOMIAL_ROOTS_H
#include <stddef.h>
#include <umath/complex.h>
#include <umath/config.h>
#ifdef __cplusplus
extern "C" {
#endif
/// Coefficients use descending powers. TODO: specify repeated-root, tolerance, ordering, and iteration-limit policy.
umath_status_t umath_poly_roots_real(const umath_scalar_t* coefficients, size_t degree, umath_scalar_t* roots, size_t capacity,
                                     size_t* root_count);
/// TODO(UMATH-POLY-ROOTS): Choose a bounded complex solver and define scaling, convergence, repeated roots, and
/// workspace.
umath_status_t umath_poly_roots_complex(const umath_complex_t* coefficients, size_t degree, umath_complex_t* roots,
                                        size_t capacity, size_t* root_count);
#ifdef __cplusplus
}
#endif
#endif
