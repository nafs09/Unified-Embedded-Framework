/// @file include/uef/umath/matrix/cholesky.h
/// @brief In-place Cholesky factorization and triangular solve.
#ifndef UEF_UMATH_MATRIX_CHOLESKY_H
#define UEF_UMATH_MATRIX_CHOLESKY_H
#include <uef/umath/config.h>

#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Factor a row-major symmetric positive-definite matrix as lower L in place.
/// On failure, treat the input buffer as modified and discard or restore it.
umath_status_t umath_cholesky_factor(umath_scalar_t *matrix, size_t dimension);
/// Solve L*L^T*x=b for a row-major lower-triangular Cholesky factor.
umath_status_t umath_cholesky_solve(const umath_scalar_t *lower_factor,
                                    const umath_scalar_t *right_hand_side,
                                    size_t dimension,
                                    umath_scalar_t *solution);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_MATRIX_CHOLESKY_H */