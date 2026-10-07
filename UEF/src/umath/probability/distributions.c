/// @file src/umath/probability/distributions.c
/// @brief Fail-closed outlines for probability calculations.
#include <uef/umath/probability/distributions.h>

umath_status_t umath_logsumexp(const umath_scalar_t *values, size_t count, umath_scalar_t *result)
{
    /* TODO(UMATH-PROBABILITY): Subtract the maximum before exponentiation and
     * define empty/all-negative-infinity/non-finite input behavior. */
    (void)values; (void)count; (void)result; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_normal_logpdf(umath_scalar_t value, umath_scalar_t mean,
                                   umath_scalar_t variance, umath_scalar_t *log_density)
{
    /* TODO(UMATH-PROBABILITY): Validate variance and compute the normalized
     * Gaussian log density without underflowing a direct PDF. */
    (void)value; (void)mean; (void)variance; (void)log_density;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_normal_cdf(umath_scalar_t value, umath_scalar_t mean,
                                umath_scalar_t standard_deviation,
                                umath_scalar_t *probability)
{
    /* TODO(UMATH-PROBABILITY): Choose a bounded approximation, state its error,
     * and handle extreme standardized values without cancellation. */
    (void)value; (void)mean; (void)standard_deviation; (void)probability;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_chi_square_cdf(umath_scalar_t value, size_t degrees_of_freedom,
                                    umath_scalar_t *probability)
{
    /* TODO(UMATH-PROBABILITY): Evaluate regularized lower incomplete gamma with
     * iteration limits, tail accuracy, and explicit invalid-degree behavior. */
    (void)value; (void)degrees_of_freedom; (void)probability;
    return UMATH_NOT_IMPLEMENTED;
}