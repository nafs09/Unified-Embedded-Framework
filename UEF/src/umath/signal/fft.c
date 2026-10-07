/// @file src/umath/signal/fft.c
/// @brief Fail-closed outlines for bounded Fourier transforms.
#include <uef/umath/signal/fft.h>

umath_status_t umath_fft_plan_init(umath_fft_plan_t *plan, size_t length,
                                   umath_complex_t *workspace, size_t workspace_count)
{
    /* TODO(UMATH-FFT): Validate supported lengths/workspace before recording a
     * reusable plan; keep all scratch caller-owned and deterministic. */
    (void)plan; (void)length; (void)workspace; (void)workspace_count;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_fft_execute(const umath_fft_plan_t *plan,
                                 const umath_complex_t *input,
                                 umath_complex_t *output, bool inverse)
{
    /* TODO(UMATH-FFT): Implement the selected radix schedule and define scale,
     * input/output overlap, finite checks, and no-partial-output behavior. */
    (void)plan; (void)input; (void)output; (void)inverse;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_fft_real_forward(const umath_fft_plan_t *plan,
                                      const umath_scalar_t *input,
                                      umath_complex_t *one_sided_spectrum)
{
    /* TODO(UMATH-FFT): Use Hermitian symmetry and document the one-sided bin
     * count plus special DC and Nyquist bin treatment. */
    (void)plan; (void)input; (void)one_sided_spectrum;
    return UMATH_NOT_IMPLEMENTED;
}