/// @file src/umath/lie/se2.c
/// @brief Fail-closed outlines for planar Lie-group operations.
#include <uef/umath/lie/se2.h>

umath_status_t umath_se2_exp(const umath_scalar_t tangent[3], umath_pose2_t *pose)
{
    /* TODO(UMATH-SE2): Apply the documented rho-then-theta exponential and
     * stable V(theta) series without allocating scratch memory. */
    (void)tangent; (void)pose; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_se2_log(const umath_pose2_t *pose, umath_scalar_t tangent[3])
{
    /* TODO(UMATH-SE2): Invert V(theta), normalize angle to its principal range,
     * and define behavior at the branch cut and near zero. */
    (void)pose; (void)tangent; return UMATH_NOT_IMPLEMENTED;
}
umath_status_t umath_se2_adjoint(const umath_pose2_t *pose, umath_scalar_t adjoint_row_major[9])
{
    /* TODO(UMATH-SE2): Fill the 3x3 adjoint for the same transform direction
     * and tangent ordering used by exp/log. */
    (void)pose; (void)adjoint_row_major; return UMATH_NOT_IMPLEMENTED;
}