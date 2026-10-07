/// @file src/umath/signal/windows.c
/// @brief Fail-closed outlines for finite-length window functions.
#include <uef/umath/signal/windows.h>

umath_status_t umath_window_hann(umath_scalar_t *output, size_t length, bool periodic)
{
    /* TODO(UMATH-WINDOW): Fill bounded coefficients using the documented
     * symmetric/periodic convention; validate before modifying output. */
    (void)output; (void)length; (void)periodic; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_window_hamming(umath_scalar_t *output, size_t length, bool periodic)
{
    /* TODO(UMATH-WINDOW): Apply standard coefficients using the shared endpoint
     * policy and preserve output when the requested length is invalid. */
    (void)output; (void)length; (void)periodic; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_window_blackman(umath_scalar_t *output, size_t length, bool periodic)
{
    /* TODO(UMATH-WINDOW): Implement the selected Blackman coefficients and
     * bound accumulated trigonometric error over the supported lengths. */
    (void)output; (void)length; (void)periodic; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_window_kaiser(umath_scalar_t *output, size_t length,
                                   umath_scalar_t beta, bool periodic)
{
    /* TODO(UMATH-WINDOW): Use the registered I0 approximation and define beta,
     * length, endpoint and overflow constraints. */
    (void)output; (void)length; (void)beta; (void)periodic;
    return UMATH_NOT_IMPLEMENTED;
}