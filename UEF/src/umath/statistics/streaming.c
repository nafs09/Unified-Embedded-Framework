/// @file src/umath/statistics/streaming.c
/// @brief Fail-closed outlines for sample statistics.
#include <uef/umath/statistics/streaming.h>

umath_status_t umath_stats_mean(const umath_scalar_t *samples, size_t count, umath_scalar_t *mean)
{
    /* TODO(UMATH-STATS): Use stable accumulation, reject empty/non-finite input,
     * and preserve mean when validation or range checks fail. */
    (void)samples; (void)count; (void)mean; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_stats_variance(const umath_scalar_t *samples, size_t count,
                                    bool sample_variance, umath_scalar_t *variance)
{
    /* TODO(UMATH-STATS): Implement Welford or a validated two-pass method and
     * clamp only documented tiny negative roundoff. */
    (void)samples; (void)count; (void)sample_variance; (void)variance;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_stats_covariance(const umath_scalar_t *samples,
                                      size_t sample_count, size_t dimension,
                                      bool sample_covariance,
                                      umath_scalar_t *row_major_covariance)
{
    /* TODO(UMATH-STATS): Document sample storage and use caller-provided bounded
     * workspace while preserving covariance symmetry and PSD as far as possible. */
    (void)samples; (void)sample_count; (void)dimension;
    (void)sample_covariance; (void)row_major_covariance;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_stats_quantile(const umath_scalar_t *samples, size_t count,
                                    umath_scalar_t probability, umath_scalar_t *quantile)
{
    /* TODO(UMATH-STATS): Implement the documented quantile convention with
     * bounded work and no hidden allocation or input mutation. */
    (void)samples; (void)count; (void)probability; (void)quantile;
    return UMATH_NOT_IMPLEMENTED;
}