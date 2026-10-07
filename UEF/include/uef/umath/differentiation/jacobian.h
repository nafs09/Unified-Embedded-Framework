/// @file include/uef/umath/differentiation/jacobian.h
/// @brief Planned caller-workspace finite-difference Jacobian operations.
#ifndef UEF_UMATH_DIFFERENTIATION_JACOBIAN_H
#define UEF_UMATH_DIFFERENTIATION_JACOBIAN_H

#include <stddef.h>
#include <uef/umath/status.h>
#include <uef/umath/scalar.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UMATH_JACOBIAN_FORWARD = 0,
    UMATH_JACOBIAN_CENTRAL
} umath_jacobian_scheme_t;

/// Callback evaluates a fixed-shape vector function without retaining input/output pointers.
/// The function should be deterministic and side-effect-free for a fixed input/context.
typedef umath_status_t (*umath_vector_function_fn)(
    const umath_scalar_t *input, size_t input_dimension, void *context,
    umath_scalar_t *output, size_t output_dimension);

/// TODO(UMATH-JACOBIAN): Check dimensions/scheme and overflow in n + m or n + 2*m; define zero-size behavior.
umath_status_t umath_jacobian_workspace_count(size_t input_dimension,
                                               size_t output_dimension,
                                               umath_jacobian_scheme_t scheme,
                                               size_t *workspace_count);

/// Evaluate row-major J where J[row, column] = d(output[row])/d(input[column]).
/// `base_output` is f(input), avoiding an extra callback. Caller buffers must not overlap.
/// Workspace layout is perturbed input[n], plus output scratch[m] for forward differences
/// or output scratch[2*m] for central differences. `relative_step` must be positive.
/// TODO(UMATH-JACOBIAN): Define relative-step bounds and how a perturbation is chosen when scalar precision makes a requested delta unrepresentable.
umath_status_t umath_jacobian_finite_difference(
    umath_vector_function_fn function, void *context,
    const umath_scalar_t *input, size_t input_dimension,
    const umath_scalar_t *base_output, size_t output_dimension,
    umath_scalar_t relative_step, umath_jacobian_scheme_t scheme,
    umath_scalar_t *workspace, size_t workspace_count,
    umath_scalar_t *jacobian_row_major);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_DIFFERENTIATION_JACOBIAN_H */
