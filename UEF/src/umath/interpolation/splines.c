/// @file src/umath/interpolation/splines.c
/// @brief Fail-closed outlines for trajectory interpolation bases.
#include <uef/umath/interpolation/splines.h>

umath_status_t umath_cubic_hermite(umath_scalar_t p0, umath_scalar_t v0,
                                   umath_scalar_t p1, umath_scalar_t v1,
                                   umath_scalar_t duration, umath_scalar_t u,
                                   umath_scalar_t *position,
                                   umath_scalar_t *velocity,
                                   umath_scalar_t *acceleration)
{
    /* TODO(UMATH-HERMITE): Evaluate position and derivatives consistently,
     * converting normalized-time derivatives by duration and duration squared. */
    (void)p0; (void)v0; (void)p1; (void)v1; (void)duration; (void)u;
    (void)position; (void)velocity; (void)acceleration;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_bezier_evaluate(const umath_scalar_t *control_points,
                                     size_t point_count, size_t dimensions,
                                     umath_scalar_t u, umath_scalar_t *point)
{
    /* TODO(UMATH-BEZIER): Validate shape/overflow and implement de Casteljau
     * with explicit caller workspace and no allocation. */
    (void)control_points; (void)point_count; (void)dimensions; (void)u; (void)point;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_bspline_basis(size_t basis_index, size_t degree,
                                   const umath_scalar_t *knots, size_t knot_count,
                                   umath_scalar_t parameter, umath_scalar_t *value)
{
    /* TODO(UMATH-BSPLINE): Validate monotone knots and implement a bounded
     * iterative Cox-de Boor recurrence with repeated-knot guards. */
    (void)basis_index; (void)degree; (void)knots; (void)knot_count;
    (void)parameter; (void)value;
    return UMATH_NOT_IMPLEMENTED;
}

umath_status_t umath_bspline_derivative(size_t basis_index, size_t degree,
                                        const umath_scalar_t *knots, size_t knot_count,
                                        umath_scalar_t parameter, size_t order,
                                        umath_scalar_t *value)
{
    /* TODO(UMATH-BSPLINE): Reuse validated basis terms and implement derivative
     * recurrence with defined zero-span and order-above-degree behavior. */
    (void)basis_index; (void)degree; (void)knots; (void)knot_count;
    (void)parameter; (void)order; (void)value;
    return UMATH_NOT_IMPLEMENTED;
}