/// @file src/umath/autodiff/dual.c
/// @brief Fail-closed outlines for forward-mode automatic differentiation.
#include <uef/umath/autodiff/dual.h>

umath_status_t umath_dual_add(umath_dual_t a, umath_dual_t b, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): Validate both value/derivative pairs and add into a
     * temporary before publishing a finite output pair. */
    (void)a; (void)b; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_sub(umath_dual_t a, umath_dual_t b, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): Apply component-wise subtraction with overflow checks. */
    (void)a; (void)b; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_mul(umath_dual_t a, umath_dual_t b, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): Apply product rule d(ab)=a' b + a b' and check intermediates. */
    (void)a; (void)b; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_div(umath_dual_t a, umath_dual_t b, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): Apply quotient rule and reject zero or non-finite denominator. */
    (void)a; (void)b; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_sin(umath_dual_t input, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): d(sin(x))=cos(x)x'; inputs are radians. */
    (void)input; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_cos(umath_dual_t input, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): d(cos(x))=-sin(x)x'; inputs are radians. */
    (void)input; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_exp(umath_dual_t input, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): d(exp(x))=exp(x)x' with overflow checks. */
    (void)input; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_log(umath_dual_t input, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): Require x>0 and compute derivative x'/x. */
    (void)input; (void)out; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_dual_sqrt(umath_dual_t input, umath_dual_t *out)
{
    /* TODO(UMATH-DUAL): Define x=0 derivative policy and compute x'/(2 sqrt(x)). */
    (void)input; (void)out; return UMATH_NOT_IMPLEMENTED;
}