/// @file include/uef/umath/optimization/least_squares.h
/// @brief Planned dense least-squares and residual utilities.
#ifndef UEF_UMATH_OPTIMIZATION_LEAST_SQUARES_H
#define UEF_UMATH_OPTIMIZATION_LEAST_SQUARES_H
#include <uef/umath/config.h>
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-LS): Pin QR/SVD selection, rank threshold, workspace and minimum-norm behavior.
umath_status_t umath_least_squares_solve(const umath_scalar_t *row_major_design,
                                         size_t rows, size_t columns,
                                         const umath_scalar_t *observations,
                                         umath_scalar_t rank_tolerance,
                                         umath_scalar_t *parameters, size_t *rank);
/// TODO(UMATH-WLS): Define whether weights represent inverse variance or direct objective weights; avoid normal-equation conditioning loss.
umath_status_t umath_weighted_least_squares_solve(const umath_scalar_t *row_major_design,
                                                  size_t rows, size_t columns,
                                                  const umath_scalar_t *observations,
                                                  const umath_scalar_t *weights,
                                                  umath_scalar_t rank_tolerance,
                                                  umath_scalar_t *parameters, size_t *rank);
umath_status_t umath_residual_norm(const umath_scalar_t *observations,
                                   const umath_scalar_t *predictions,
                                   size_t count, umath_scalar_t *norm);
#ifdef __cplusplus
}
#endif
#endif