/// @file src/umath/matrix/symmetric_eigen.c
/// @brief Fail-closed outline for a bounded symmetric eigensolver.
#include <uef/umath/matrix/symmetric_eigen.h>

umath_status_t umath_matrix_symmetric_eigen(const umath_scalar_t *row_major,
                                            size_t dimension,
                                            umath_scalar_t *eigenvalues,
                                            umath_scalar_t *eigenvectors,
                                            size_t maximum_sweeps,
                                            size_t *sweeps_used)
{
    /* TODO(UMATH-EIGEN): Implement the documented symmetric Jacobi method with
     * bounded sweeps, stable rotations, sorted eigenpairs, and no heap usage. */
    (void)row_major; (void)dimension; (void)eigenvalues; (void)eigenvectors;
    (void)maximum_sweeps; (void)sweeps_used;
    return UMATH_NOT_IMPLEMENTED;
}