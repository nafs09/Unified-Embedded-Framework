/// @file src/umath/probability/multivariate_normal.c
/// @brief Fail-closed multivariate Gaussian likelihood outline.
#include <uef/umath/probability/multivariate_normal.h>

umath_status_t umath_multivariate_normal_logpdf_cholesky(
    const umath_scalar_t *residual, const umath_scalar_t *lower_cholesky,
    size_t dimension, umath_scalar_t *whitened_workspace,
    umath_scalar_t *log_density)
{
    /* TODO(UMATH-MVN): Forward-solve L*z=residual, accumulate ||z||^2 and log(det(L)) in bounded precision, then commit the finite log-density. */
    (void)residual;
    (void)lower_cholesky;
    (void)dimension;
    (void)whitened_workspace;
    (void)log_density;
    return UMATH_NOT_IMPLEMENTED;
}
