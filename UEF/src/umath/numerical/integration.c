/// @file src/umath/numerical/integration.c
/// @brief Fail-closed outlines for quadrature and ODE stepping.
#include <uef/umath/numerical/integration.h>

umath_status_t umath_integrate_trapezoid(const umath_scalar_t *samples, size_t count,
                                         umath_scalar_t step, umath_scalar_t *integral)
{
    /* TODO(UMATH-INTEGRATION): Validate finite samples and signed spacing, then
     * accumulate the composite trapezoid in wider precision. */
    (void)samples; (void)count; (void)step; (void)integral;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_integrate_simpson(const umath_scalar_t *samples, size_t count,
                                       umath_scalar_t step, umath_scalar_t *integral)
{
    /* TODO(UMATH-INTEGRATION): Implement the documented composite Simpson
     * panel rule and avoid silently dropping a final interval. */
    (void)samples; (void)count; (void)step; (void)integral;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_ode_rk4_step(umath_ode_rhs_fn rhs, void *context,
                                  umath_scalar_t time, umath_scalar_t step,
                                  const umath_scalar_t *state, size_t dimension,
                                  umath_scalar_t *workspace, size_t workspace_count,
                                  umath_scalar_t *next_state)
{
    /* TODO(UMATH-RK4): Use the exact four-stage tableau in caller workspace and
     * publish next_state only after all callbacks and finite checks succeed. */
    (void)rhs; (void)context; (void)time; (void)step; (void)state;
    (void)dimension; (void)workspace; (void)workspace_count; (void)next_state;
    return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_ode_dopri5_step(umath_ode_rhs_fn rhs, void *context,
                                     umath_scalar_t time, umath_scalar_t step,
                                     const umath_scalar_t *state, size_t dimension,
                                     umath_scalar_t abs_tol, umath_scalar_t rel_tol,
                                     size_t maximum_rhs_evaluations,
                                     umath_scalar_t *workspace, size_t workspace_count,
                                     umath_scalar_t *next_state,
                                     umath_scalar_t *suggested_step, bool *accepted)
{
    /* TODO(UMATH-DOPRI5): Implement embedded 5(4) stages with bounded RHS calls,
     * scaled error norm, reject/accept decision, and a finite suggested step. */
    (void)rhs; (void)context; (void)time; (void)step; (void)state; (void)dimension;
    (void)abs_tol; (void)rel_tol; (void)maximum_rhs_evaluations;
    (void)workspace; (void)workspace_count; (void)next_state;
    (void)suggested_step; (void)accepted;
    return UMATH_NOT_IMPLEMENTED;
}