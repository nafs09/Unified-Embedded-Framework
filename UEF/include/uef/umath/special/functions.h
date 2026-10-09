/// @file include/uef/umath/special/functions.h
/// @brief Planned bounded special functions required by probability and signal modules.
#ifndef UEF_UMATH_SPECIAL_FUNCTIONS_H
#define UEF_UMATH_SPECIAL_FUNCTIONS_H
#include <uef/umath/config.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-SPECIAL): State supported range and maximum approximation error, including tail behavior.
umath_status_t umath_erf(umath_scalar_t value, umath_scalar_t *result);
/// TODO(UMATH-SPECIAL): Define supported positive domain, reflection policy, and overflow behavior.
umath_status_t umath_log_gamma(umath_scalar_t value, umath_scalar_t *result);
/// TODO(UMATH-SPECIAL): Pin Bessel I0 approximation, large-input scaling, range, and error target.
umath_status_t umath_bessel_i0(umath_scalar_t value, umath_scalar_t *result);
#ifdef __cplusplus
}
#endif
#endif