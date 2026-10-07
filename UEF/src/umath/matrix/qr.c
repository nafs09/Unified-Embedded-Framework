/// @file src/umath/matrix/qr.c
/// @brief Fail-closed outlines for Householder QR operations.
#include <uef/umath/matrix/qr.h>

umath_status_t umath_matrix_qr_factor(umath_scalar_t *row_major, size_t rows,
                                      size_t columns, umath_scalar_t *tau)
{
    /* TODO(UMATH-QR): Add scaled Householder vectors in row-major storage and
     * return coefficients without allocating or exceeding the dimension bound. */
    (void)row_major; (void)rows; (void)columns; (void)tau;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_matrix_qr_least_squares(const umath_scalar_t *qr,
                                             const umath_scalar_t *tau,
                                             size_t rows, size_t columns,
                                             const umath_scalar_t *rhs,
                                             umath_scalar_t rank_tolerance,
                                             umath_scalar_t *solution,
                                             size_t *rank)
{
    /* TODO(UMATH-QR): Apply Q-transpose and triangular solve, detect numerical
     * rank, and define minimum-norm behavior for rank-deficient inputs. */
    (void)qr; (void)tau; (void)rows; (void)columns; (void)rhs;
    (void)rank_tolerance; (void)solution; (void)rank;
    return UMATH_NOT_IMPLEMENTED;
}