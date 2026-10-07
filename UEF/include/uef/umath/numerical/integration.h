/// @file include/uef/umath/numerical/integration.h
/// @brief Planned fixed-step quadrature and bounded ODE integration routines.
#ifndef UEF_UMATH_NUMERICAL_INTEGRATION_H
#define UEF_UMATH_NUMERICAL_INTEGRATION_H
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
umath_status_t umath_integrate_trapezoid(const umath_scalar_t *samples, size_t count,
                                         umath_scalar_t step, umath_scalar_t *integral);
/// TODO(UMATH-INTEGRATION): Define even/odd panel policy and endpoint handling for composite Simpson integration.
umath_status_t umath_integrate_simpson(const umath_scalar_t *samples, size_t count,
                                       umath_scalar_t step, umath_scalar_t *integral);
typedef umath_status_t (*umath_ode_rhs_fn)(umath_scalar_t time,
                                           const umath_scalar_t *state,
                                           size_t dimension,
                                           umath_scalar_t *derivative,
                                           void *context);
/// TODO(UMATH-RK4): Specify scratch layout/size, callback errors, overlap rules, and all-or-nothing output semantics.
umath_status_t umath_ode_rk4_step(umath_ode_rhs_fn rhs, void *context,
                                  umath_scalar_t time, umath_scalar_t step,
                                  const umath_scalar_t *state, size_t dimension,
                                  umath_scalar_t *workspace, size_t workspace_count,
                                  umath_scalar_t *next_state);
/// TODO(UMATH-DOPRI5): Pin tableau, adaptive error norm, rejection budget, tolerance scaling, workspace, and step bounds.
umath_status_t umath_ode_dopri5_step(umath_ode_rhs_fn rhs, void *context,
                                     umath_scalar_t time, umath_scalar_t step,
                                     const umath_scalar_t *state, size_t dimension,
                                     umath_scalar_t abs_tol, umath_scalar_t rel_tol,
                                     size_t maximum_rhs_evaluations,
                                     umath_scalar_t *workspace, size_t workspace_count,
                                     umath_scalar_t *next_state,
                                     umath_scalar_t *suggested_step, bool *accepted);
#ifdef __cplusplus
}
#endif
#endif