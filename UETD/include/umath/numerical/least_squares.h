/// @file include/umath/numerical/least_squares.h
/// @brief Planned dense least-squares and residual utilities.
#ifndef UETD_UMATH_OPTIMIZATION_LEAST_SQUARES_H
#define UETD_UMATH_OPTIMIZATION_LEAST_SQUARES_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// @brief Solve an unweighted linear least-squares problem.
/// TODO(UMATH-LS): Pin QR/SVD selection, rank threshold, workspace and minimum-norm behavior.
umath_status_t umath_least_squares_solve(const umath_scalar_t* row_major_design, size_t rows, size_t columns,
                                         const umath_scalar_t* observations, umath_scalar_t rank_tolerance,
                                         umath_scalar_t* parameters, size_t* rank);
/// @brief Solve a linear least-squares problem with per-row weights.
/// TODO(UMATH-WLS): Define whether weights represent inverse variance or direct objective weights; avoid
/// normal-equation conditioning loss.
umath_status_t umath_weighted_least_squares_solve(const umath_scalar_t* row_major_design, size_t rows, size_t columns,
                                                  const umath_scalar_t* observations, const umath_scalar_t* weights,
                                                  umath_scalar_t rank_tolerance, umath_scalar_t* parameters, size_t* rank);
/// Compute the norm of a least-squares residual vector.
umath_status_t umath_residual_norm(const umath_scalar_t* observations, const umath_scalar_t* predictions, size_t count,
                                   umath_scalar_t* norm);
#ifdef __cplusplus
}
#endif
#endif
