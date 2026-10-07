/// @file src/umath/polynomial/roots.c
/// @brief Fail-closed outlines for polynomial root solvers.
#include <uef/umath/polynomial/roots.h>

umath_status_t umath_poly_roots_real(const umath_scalar_t *coefficients, size_t degree,
                                     umath_scalar_t *roots, size_t capacity,
                                     size_t *root_count)
{
    /* TODO(UMATH-POLY-ROOTS): Implement a bounded real-root method, normalize
     * coefficient scale, report distinct/multiple roots by the chosen contract. */
    (void)coefficients; (void)degree; (void)roots; (void)capacity; (void)root_count;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_poly_roots_complex(const umath_complex_t *coefficients, size_t degree,
                                        umath_complex_t *roots, size_t capacity,
                                        size_t *root_count)
{
    /* TODO(UMATH-POLY-ROOTS): Define an allocation-free complex solver, its
     * bounded iterations, stopping tolerance, and deterministic root ordering. */
    (void)coefficients; (void)degree; (void)roots; (void)capacity; (void)root_count;
    return UMATH_NOT_IMPLEMENTED;
}