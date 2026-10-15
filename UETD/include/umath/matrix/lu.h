/// @file include/umath/matrix/lu.h
/// @brief Planned pivoted LU factorization. Every operation remains fail-closed.
#ifndef UETD_UMATH_MATRIX_LU_H
#define UETD_UMATH_MATRIX_LU_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-LU): Fix packed L/U layout, partial-pivot tolerance, permutation direction/sign, and failure mutation
/// policy.
umath_status_t umath_matrix_lu_factor(umath_scalar_t* row_major, size_t n, size_t* permutation, int* permutation_sign);
/// TODO(UMATH-LU): Validate pivots and solve with bounded forward/back substitution without heap allocation.
umath_status_t umath_matrix_lu_solve(const umath_scalar_t* lu, const size_t* permutation, size_t n, const umath_scalar_t* rhs,
                                     umath_scalar_t* solution);

/// Coverage API scaffolds. These return UMATH_NOT_IMPLEMENTED and leave outputs unchanged.

umath_status_t umath_matrix_lu_determinant(const umath_scalar_t* lu, const size_t* permutation, size_t n, int permutation_sign, umath_scalar_t* determinant);

#ifdef __cplusplus
}
#endif
#endif
