/// @file include/uef/umath/matrix/lu.h
/// @brief Planned pivoted LU factorization. Every operation remains fail-closed.
#ifndef UEF_UMATH_MATRIX_LU_H
#define UEF_UMATH_MATRIX_LU_H
#include <uef/umath/config.h>
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-LU): Fix packed L/U layout, partial-pivot tolerance, permutation direction/sign, and failure mutation policy.
umath_status_t umath_matrix_lu_factor(umath_scalar_t *row_major, size_t n,
                                      size_t *permutation, int *permutation_sign);
/// TODO(UMATH-LU): Validate pivots and solve with bounded forward/back substitution without heap allocation.
umath_status_t umath_matrix_lu_solve(const umath_scalar_t *lu,
                                     const size_t *permutation, size_t n,
                                     const umath_scalar_t *rhs,
                                     umath_scalar_t *solution);
#ifdef __cplusplus
}
#endif
#endif