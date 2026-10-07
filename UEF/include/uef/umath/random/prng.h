/// @file include/uef/umath/random/prng.h
/// @brief Planned deterministic, caller-owned pseudo-random streams.
#ifndef UEF_UMATH_RANDOM_PRNG_H
#define UEF_UMATH_RANDOM_PRNG_H
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// Planned non-cryptographic stream. Exact generator family and split rules remain TODO.
typedef struct { uef_u64_t state; uef_u64_t stream; } umath_prng_t;
umath_status_t umath_prng_seed(umath_prng_t *generator, uef_u64_t seed, uef_u64_t stream);
umath_status_t umath_prng_u32(umath_prng_t *generator, uef_u32_t *value);
umath_status_t umath_prng_uniform(umath_prng_t *generator, umath_scalar_t *value);
/// TODO(UMATH-PRNG): Define bounded normal sampler, cached-state layout, and deterministic number of stream draws.
umath_status_t umath_prng_normal(umath_prng_t *generator, umath_scalar_t mean,
                                 umath_scalar_t standard_deviation,
                                 umath_scalar_t *value);
#ifdef __cplusplus
}
#endif
#endif