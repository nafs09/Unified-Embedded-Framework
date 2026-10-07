/// @file src/umath/matrix/ldlt.c
/// @brief Fail-closed outlines for symmetric indefinite factorization.
#include <uef/umath/matrix/ldlt.h>

umath_status_t umath_matrix_ldlt_factor(umath_scalar_t *row_major, size_t n,
                                        umath_scalar_t *diagonal_blocks,
                                        size_t *permutation,
                                        size_t *block_sizes)
{
    /* TODO(UMATH-LDLT): Implement the selected symmetric pivot policy, encode
     * every 1x1/2x2 pivot, and bound work and scratch storage by dimension. */
    (void)row_major; (void)n; (void)diagonal_blocks; (void)permutation; (void)block_sizes;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_matrix_ldlt_solve(const umath_scalar_t *lower,
                                       const umath_scalar_t *diagonal_blocks,
                                       const size_t *permutation,
                                       const size_t *block_sizes, size_t n,
                                       const umath_scalar_t *rhs,
                                       umath_scalar_t *solution)
{
    /* TODO(UMATH-LDLT): Apply permutation and bounded forward, block-diagonal,
     * and backward solves; preserve solution if descriptors or pivots fail. */
    (void)lower; (void)diagonal_blocks; (void)permutation; (void)block_sizes;
    (void)n; (void)rhs; (void)solution;
    return UMATH_NOT_IMPLEMENTED;
}