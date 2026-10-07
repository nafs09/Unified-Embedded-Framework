/// @file src/umath/special/functions.c
/// @brief Fail-closed outlines for special scalar functions.
#include <uef/umath/special/functions.h>

umath_status_t umath_erf(umath_scalar_t value, umath_scalar_t *result)
{
    /* TODO(UMATH-SPECIAL): Add a bounded approximation and verify absolute error
     * across the full supported domain, including saturation in the tails. */
    (void)value; (void)result; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_log_gamma(umath_scalar_t value, umath_scalar_t *result)
{
    /* TODO(UMATH-SPECIAL): Implement log-gamma with a documented positive
     * domain, stable asymptotics, and finite output checks. */
    (void)value; (void)result; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_bessel_i0(umath_scalar_t value, umath_scalar_t *result)
{
    /* TODO(UMATH-SPECIAL): Implement a bounded I0 approximation and scale
     * large magnitudes to avoid overflow while preserving declared error. */
    (void)value; (void)result; return UMATH_NOT_IMPLEMENTED;
}