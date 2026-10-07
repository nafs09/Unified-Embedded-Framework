/// @file include/uef/umath/probability/multivariate_normal.h
/// @brief Planned multivariate Gaussian log-density using a covariance Cholesky factor.
#ifndef UEF_UMATH_PROBABILITY_MULTIVARIATE_NORMAL_H
#define UEF_UMATH_PROBABILITY_MULTIVARIATE_NORMAL_H

#include <stddef.h>
#include <uef/umath/scalar.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Evaluate the zero-mean residual density for covariance = L*L^T.
/// `lower_cholesky` is row-major; workspace has `dimension` elements.
/// Buffers must be disjoint. The API will be fail-closed until factor validation,
/// summation precision, and unchanged-output-on-failure rules are specified.
/// TODO(UMATH-MVN): Define factor tolerance, singular behavior, bounded accumulation, and output commit semantics.
umath_status_t umath_multivariate_normal_logpdf_cholesky(
    const umath_scalar_t *residual, const umath_scalar_t *lower_cholesky,
    size_t dimension, umath_scalar_t *whitened_workspace,
    umath_scalar_t *log_density);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_PROBABILITY_MULTIVARIATE_NORMAL_H */
