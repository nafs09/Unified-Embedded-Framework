/// @file include/uef/umath/matrix/svd.h
/// @brief Planned bounded singular-value decomposition and solve.
#ifndef UEF_UMATH_MATRIX_SVD_H
#define UEF_UMATH_MATRIX_SVD_H
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-SVD): Choose a bounded method and define rectangular U/S/Vt storage, sorted values, sweep limit, and zero/repeated singular-value behavior.
umath_status_t umath_matrix_svd(const umath_scalar_t *input,
                                size_t rows, size_t columns,
                                umath_scalar_t *left_vectors,
                                umath_scalar_t *singular_values,
                                umath_scalar_t *right_vectors_transposed,
                                size_t maximum_sweeps, size_t *sweeps_used);
/// TODO(UMATH-SVD): Define relative cutoff, rank reporting, minimum-norm behavior, and aliasing for the pseudoinverse solve.
umath_status_t umath_matrix_svd_solve(const umath_scalar_t *left_vectors,
                                      const umath_scalar_t *singular_values,
                                      const umath_scalar_t *right_vectors_transposed,
                                      size_t rows, size_t columns,
                                      const umath_scalar_t *rhs,
                                      umath_scalar_t relative_tolerance,
                                      umath_scalar_t *solution, size_t *rank);
#ifdef __cplusplus
}
#endif
#endif