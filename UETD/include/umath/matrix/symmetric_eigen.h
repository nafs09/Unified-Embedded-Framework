/// @file include/umath/matrix/symmetric_eigen.h
/// @brief Planned bounded eigensystem for real symmetric matrices.
#ifndef UETD_UMATH_MATRIX_SYMMETRIC_EIGEN_H
#define UETD_UMATH_MATRIX_SYMMETRIC_EIGEN_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-EIGEN): Fix Jacobi schedule, symmetry tolerance, eigenvalue sorting, eigenvector column layout, sweep
/// limit, and repeated-eigenvalue policy.
umath_status_t umath_matrix_symmetric_eigen(const umath_scalar_t* row_major, size_t dimension, umath_scalar_t* eigenvalues,
                                            umath_scalar_t* eigenvectors, size_t maximum_sweeps, size_t* sweeps_used);
#ifdef __cplusplus
}
#endif
#endif
