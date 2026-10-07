/// @file include/uef/umath/statistics/streaming.h
/// @brief Planned bounded sample statistics for calibration and estimation.
#ifndef UEF_UMATH_STATISTICS_STREAMING_H
#define UEF_UMATH_STATISTICS_STREAMING_H
#include <stdbool.h>
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
umath_status_t umath_stats_mean(const umath_scalar_t *samples, size_t count, umath_scalar_t *mean);
/// TODO(UMATH-STATS): Specify sample/population denominator, minimum count, accumulation precision, and roundoff policy.
umath_status_t umath_stats_variance(const umath_scalar_t *samples, size_t count,
                                    bool sample_variance, umath_scalar_t *variance);
/// TODO(UMATH-STATS): Specify row-major sample layout, PSD/symmetry policy, and caller-owned workspace for vector covariance.
umath_status_t umath_stats_covariance(const umath_scalar_t *row_major_samples,
                                      size_t sample_count, size_t dimension,
                                      bool sample_covariance,
                                      umath_scalar_t *row_major_covariance);
/// TODO(UMATH-STATS): Pick a quantile convention and define bounded selection workspace and endpoint behavior.
umath_status_t umath_stats_quantile(const umath_scalar_t *samples, size_t count,
                                    umath_scalar_t probability, umath_scalar_t *quantile);
#ifdef __cplusplus
}
#endif
#endif