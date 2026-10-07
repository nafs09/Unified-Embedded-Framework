/// @file src/umath/random/prng.c
/// @brief Fail-closed outlines for deterministic pseudo-random APIs.
#include <uef/umath/random/prng.h>

umath_status_t umath_prng_seed(umath_prng_t *generator, uef_u64_t seed, uef_u64_t stream)
{
    /* TODO(UMATH-PRNG): Pin the generator and independent-stream semantics,
     * then initialize a reproducible non-degenerate state. */
    (void)generator; (void)seed; (void)stream; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_prng_u32(umath_prng_t *generator, uef_u32_t *value)
{
    /* TODO(UMATH-PRNG): Advance one documented state transition and preserve
     * generator/value when arguments are invalid. */
    (void)generator; (void)value; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_prng_uniform(umath_prng_t *generator, umath_scalar_t *value)
{
    /* TODO(UMATH-PRNG): Map generator bits to the documented half-open interval
     * with a stated precision and bias bound. */
    (void)generator; (void)value; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_prng_normal(umath_prng_t *generator, umath_scalar_t mean,
                                 umath_scalar_t standard_deviation,
                                 umath_scalar_t *value)
{
    /* TODO(UMATH-PRNG): Implement the selected bounded normal transform and
     * define positive-scale, cache, range, and stream-consumption behavior. */
    (void)generator; (void)mean; (void)standard_deviation; (void)value;
    return UMATH_NOT_IMPLEMENTED;
}