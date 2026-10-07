/// @file src/umath/optimization/least_squares.c
/// @brief Fail-closed outlines for least-squares calculations.
#include <uef/umath/optimization/least_squares.h>

umath_status_t umath_least_squares_solve(const umath_scalar_t *design,
                                         size_t rows, size_t columns,
                                         const umath_scalar_t *observations,
                                         umath_scalar_t rank_tolerance,
                                         umath_scalar_t *parameters, size_t *rank)
{
    /* TODO(UMATH-LS): Use stable QR/SVD modules, caller-owned workspace and an
     * explicit rank policy; return a minimum-norm solution if selected. */
    (void)design; (void)rows; (void)columns; (void)observations;
    (void)rank_tolerance; (void)parameters; (void)rank;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_weighted_least_squares_solve(const umath_scalar_t *design,
                                                  size_t rows, size_t columns,
                                                  const umath_scalar_t *observations,
                                                  const umath_scalar_t *weights,
                                                  umath_scalar_t rank_tolerance,
                                                  umath_scalar_t *parameters, size_t *rank)
{
    /* TODO(UMATH-WLS): Scale rows using validated positive weights, then solve
     * without squaring the design condition number. */
    (void)design; (void)rows; (void)columns; (void)observations; (void)weights;
    (void)rank_tolerance; (void)parameters; (void)rank;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_residual_norm(const umath_scalar_t *observations,
                                   const umath_scalar_t *predictions,
                                   size_t count, umath_scalar_t *norm)
{
    /* TODO(UMATH-LS): Form residuals in wider precision and accumulate a scaled
     * norm without modifying observations or predictions. */
    (void)observations; (void)predictions; (void)count; (void)norm;
    return UMATH_NOT_IMPLEMENTED;
}