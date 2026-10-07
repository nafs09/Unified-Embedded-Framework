/// @file include/uef/umath/signal/windows.h
/// @brief Planned finite-length window coefficient generators.
#ifndef UEF_UMATH_SIGNAL_WINDOWS_H
#define UEF_UMATH_SIGNAL_WINDOWS_H
#include <stdbool.h>
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-WINDOW): Define symmetric/periodic denominator and length-one behavior.
umath_status_t umath_window_hann(umath_scalar_t *output, size_t length, bool periodic);
umath_status_t umath_window_hamming(umath_scalar_t *output, size_t length, bool periodic);
/// TODO(UMATH-WINDOW): Pin the coefficient definition, endpoint convention, and roundoff limits.
umath_status_t umath_window_blackman(umath_scalar_t *output, size_t length, bool periodic);
/// TODO(UMATH-WINDOW): Bound beta, specify I0 approximation and endpoint convention.
umath_status_t umath_window_kaiser(umath_scalar_t *output, size_t length,
                                   umath_scalar_t beta, bool periodic);
#ifdef __cplusplus
}
#endif
#endif