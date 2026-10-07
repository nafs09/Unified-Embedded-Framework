/// @file src/umath/differentiation/jacobian.c
/// @brief Fail-closed finite-difference Jacobian outline.
#include <uef/umath/differentiation/jacobian.h>

umath_status_t umath_jacobian_workspace_count(size_t input_dimension,
                                               size_t output_dimension,
                                               umath_jacobian_scheme_t scheme,
                                               size_t *workspace_count)
{
    /* TODO(UMATH-JACOBIAN): Return checked scratch sizing for perturbed inputs and callback outputs. */
    (void)input_dimension;
    (void)output_dimension;
    (void)scheme;
    (void)workspace_count;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_jacobian_finite_difference(
    umath_vector_function_fn function, void *context,
    const umath_scalar_t *input, size_t input_dimension,
    const umath_scalar_t *base_output, size_t output_dimension,
    umath_scalar_t relative_step, umath_jacobian_scheme_t scheme,
    umath_scalar_t *workspace, size_t workspace_count,
    umath_scalar_t *jacobian_row_major)
{
    /* TODO(UMATH-JACOBIAN): Validate all storage, propagate callback failures, and preserve J on any failed perturbation/evaluation. */
    (void)function;
    (void)context;
    (void)input;
    (void)input_dimension;
    (void)base_output;
    (void)output_dimension;
    (void)relative_step;
    (void)scheme;
    (void)workspace;
    (void)workspace_count;
    (void)jacobian_row_major;
    return UMATH_NOT_IMPLEMENTED;
}
