/// @file include/uef/umath/interpolation/splines.h
/// @brief Planned Hermite, Bezier, and B-spline basis operations.
#ifndef UEF_UMATH_INTERPOLATION_SPLINES_H
#define UEF_UMATH_INTERPOLATION_SPLINES_H
#include <stddef.h>
#include <uef/ucore/uef_types.h>
#include <uef/umath/status.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-HERMITE): Specify endpoint derivative units, normalized-time interval, and duration validation.
umath_status_t umath_cubic_hermite(umath_scalar_t p0, umath_scalar_t v0,
                                   umath_scalar_t p1, umath_scalar_t v1,
                                   umath_scalar_t duration, umath_scalar_t u,
                                   umath_scalar_t *position,
                                   umath_scalar_t *velocity,
                                   umath_scalar_t *acceleration);
/// TODO(UMATH-BEZIER): Specify bounded de Casteljau workspace and parameter clamping.
umath_status_t umath_bezier_evaluate(const umath_scalar_t *control_points,
                                     size_t point_count, size_t dimensions,
                                     umath_scalar_t u, umath_scalar_t *point);
/// TODO(UMATH-BSPLINE): Define knot validity, repeated-knot handling, endpoint convention, and maximum degree.
umath_status_t umath_bspline_basis(size_t basis_index, size_t degree,
                                   const umath_scalar_t *knots, size_t knot_count,
                                   umath_scalar_t parameter, umath_scalar_t *value);
/// TODO(UMATH-BSPLINE): Define derivatives above degree and bounded recurrence/workspace.
umath_status_t umath_bspline_derivative(size_t basis_index, size_t degree,
                                        const umath_scalar_t *knots, size_t knot_count,
                                        umath_scalar_t parameter, size_t order,
                                        umath_scalar_t *value);
#ifdef __cplusplus
}
#endif
#endif