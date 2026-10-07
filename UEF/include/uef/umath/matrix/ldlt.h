/// @file include/uef/umath/matrix/ldlt.h
/// @brief Planned pivoted LDL-transpose factorization for symmetric systems.
#ifndef UEF_UMATH_MATRIX_LDLT_H
#define UEF_UMATH_MATRIX_LDLT_H
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-LDLT): Choose bounded Bunch-Kaufman or rook pivoting and define 1x1/2x2 D-block, permutation, tolerance, and mutation contracts.
umath_status_t umath_matrix_ldlt_factor(umath_scalar_t *row_major, size_t n,
                                        umath_scalar_t *diagonal_blocks,
                                        size_t *permutation,
                                        size_t *block_sizes);
/// TODO(UMATH-LDLT): Validate block metadata and solve L, D, and L-transpose in the exact factor convention.
umath_status_t umath_matrix_ldlt_solve(const umath_scalar_t *lower,
                                       const umath_scalar_t *diagonal_blocks,
                                       const size_t *permutation,
                                       const size_t *block_sizes, size_t n,
                                       const umath_scalar_t *rhs,
                                       umath_scalar_t *solution);
#ifdef __cplusplus
}
#endif
#endif