/// @file include/umath/matrix/qr.h
/// @brief Planned Householder QR factorization and least-squares solve.
#ifndef UETD_UMATH_MATRIX_QR_H
#define UETD_UMATH_MATRIX_QR_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-QR): Specify packed Householder storage, tau layout, rectangular matrix rules, and numerical rank
/// tolerance.
umath_status_t umath_matrix_qr_factor(umath_scalar_t* row_major, size_t rows, size_t columns, umath_scalar_t* tau);
/// TODO(UMATH-QR): Define rank-deficient policy, tall/wide matrix behavior, workspace, and output guarantees.
umath_status_t umath_matrix_qr_least_squares(const umath_scalar_t* qr, const umath_scalar_t* tau, size_t rows, size_t columns,
                                             const umath_scalar_t* rhs, umath_scalar_t rank_tolerance, umath_scalar_t* solution,
                                             size_t* rank);
#ifdef __cplusplus
}
#endif
#endif
