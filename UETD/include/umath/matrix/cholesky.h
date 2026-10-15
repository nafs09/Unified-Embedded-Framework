/// @file include/umath/matrix/cholesky.h
/// @brief In-place Cholesky factorization and triangular solve.
#ifndef UETD_UMATH_MATRIX_CHOLESKY_H
#define UETD_UMATH_MATRIX_CHOLESKY_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Factor a row-major symmetric positive-definite matrix as lower L in place.
/// On failure, treat the input buffer as modified and discard or restore it.
umath_status_t umath_cholesky_factor(umath_scalar_t* matrix, size_t dimension);
/// Solve L*L^T*x=b for a row-major lower-triangular Cholesky factor.
umath_status_t umath_cholesky_solve(const umath_scalar_t* lower_factor, const umath_scalar_t* right_hand_side, size_t dimension,
                                    umath_scalar_t* solution);
/// Factor-based log-determinant leaves its output unchanged on failure.

/// The inverse requires non-overlapping lower-factor and output buffers. The output is scratch storage;
/// discard it if a numeric failure occurs after calculation begins.
umath_status_t umath_cholesky_log_determinant(const umath_scalar_t* lower_factor, size_t dimension, umath_scalar_t* log_determinant);
umath_status_t umath_cholesky_inverse(const umath_scalar_t* lower_factor, size_t dimension, umath_scalar_t* inverse);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_MATRIX_CHOLESKY_H */
