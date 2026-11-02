/// @file include/umath/signal/fft.h
/// @brief Planned caller-workspace DFT/FFT and one-sided real spectrum.
#ifndef UETD_UMATH_SIGNAL_FFT_H
#define UETD_UMATH_SIGNAL_FFT_H
#include <stdbool.h>
#include <stddef.h>
#include <umath/complex.h>
#include <umath/config.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    size_t length;
    umath_complex_t* workspace;
    size_t workspace_count;
} umath_fft_plan_t;
/// @brief Initialize an FFT plan over caller-provided workspace for the requested length.
/// TODO(UMATH-FFT): Specify supported factor lengths, exact workspace/alignment, and plan reusability.
umath_status_t umath_fft_plan_init(umath_fft_plan_t* plan, size_t length, umath_complex_t* workspace, size_t workspace_count);
/// @brief Execute a forward or inverse complex FFT using an initialized plan.
/// TODO(UMATH-FFT): Pin sign, inverse normalization, in-place rules, and bounded radix/mixed-radix work.
umath_status_t umath_fft_execute(const umath_fft_plan_t* plan, const umath_complex_t* input, umath_complex_t* output,
                                 bool inverse);
/// @brief Compute the one-sided complex spectrum of real-valued input samples.
/// TODO(UMATH-FFT): Define packed one-sided length and DC/Nyquist scaling for real input.
umath_status_t umath_fft_real_forward(const umath_fft_plan_t* plan, const umath_scalar_t* input,
                                      umath_complex_t* one_sided_spectrum);
#ifdef __cplusplus
}
#endif
#endif
