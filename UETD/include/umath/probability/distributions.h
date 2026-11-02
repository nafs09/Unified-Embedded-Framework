/// @file include/umath/probability/distributions.h
/// @brief Planned stable log-domain and common probability-density operations.
#ifndef UETD_UMATH_PROBABILITY_DISTRIBUTIONS_H
#define UETD_UMATH_PROBABILITY_DISTRIBUTIONS_H
#include <stddef.h>
#include <ucore/types.h>
#include <umath/config.h>
#include <umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// Compute log(sum(exp(values))) with a numerically stable reduction.
umath_status_t umath_logsumexp(const umath_scalar_t* values, size_t count, umath_scalar_t* result);
/// @brief Evaluate the log probability density of a normal distribution.
/// TODO(UMATH-PROBABILITY): Require strictly positive variance and pin log-domain normalization/overflow behavior.
umath_status_t umath_normal_logpdf(umath_scalar_t value, umath_scalar_t mean, umath_scalar_t variance,
                                   umath_scalar_t* log_density);
/// @brief Evaluate the cumulative distribution function of a normal distribution.
/// TODO(UMATH-PROBABILITY): Specify approximation accuracy and tail behavior for the normal CDF.
umath_status_t umath_normal_cdf(umath_scalar_t value, umath_scalar_t mean, umath_scalar_t standard_deviation,
                                umath_scalar_t* probability);
/// @brief Evaluate the chi-square cumulative distribution function.
/// TODO(UMATH-PROBABILITY): Define bounded incomplete-gamma evaluation and iteration-limit diagnostics.
umath_status_t umath_chi_square_cdf(umath_scalar_t value, size_t degrees_of_freedom, umath_scalar_t* probability);

/// Additional fail-closed scaffolds for estimator, geometry, and probability algorithms.
umath_status_t umath_softmax(const umath_scalar_t* logits, size_t count, umath_scalar_t* probabilities);
/// Compute log-softmax values using a numerically stable reduction.
umath_status_t umath_log_softmax(const umath_scalar_t* logits, size_t count, umath_scalar_t* log_probabilities);

#ifdef __cplusplus
}
#endif
#endif
