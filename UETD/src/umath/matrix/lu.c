/// @file src/umath/matrix/lu.c
/// @brief Fail-closed outlines for pivoted LU operations.
#include <umath/matrix/lu.h>

umath_status_t umath_matrix_lu_factor(umath_scalar_t* row_major, size_t n, size_t* permutation, int* permutation_sign) {
    /* TODO(UMATH-LU): Implement scaled partial pivoting, singularity tolerance,
     * packed L/U storage, and transactional validation before exposing factor data. */
    (void)row_major;
    (void)n;
    (void)permutation;
    (void)permutation_sign;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_matrix_lu_solve(const umath_scalar_t* lu, const size_t* permutation, size_t n, const umath_scalar_t* rhs,
                                     umath_scalar_t* solution) {
    /* TODO(UMATH-LU): Validate permutation and diagonal pivots, then solve the
     * permuted triangular systems using caller storage and explicit alias rules. */
    (void)lu;
    (void)permutation;
    (void)n;
    (void)rhs;
    (void)solution;
    return UMATH_NOT_IMPLEMENTED;
}

/* Coverage scaffolds: equations and edge-case policies remain to be implemented. */
umath_status_t umath_matrix_lu_determinant(const umath_scalar_t* lu, const size_t* permutation, size_t n, int permutation_sign, umath_scalar_t* determinant) {
    (void)lu;
    (void)permutation;
    (void)n;
    (void)permutation_sign;
    (void)determinant;
    return UMATH_NOT_IMPLEMENTED;
}

