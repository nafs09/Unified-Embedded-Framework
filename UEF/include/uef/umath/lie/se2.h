/// @file include/uef/umath/lie/se2.h
/// @brief Planned planar rigid-pose and tangent operations.
#ifndef UEF_UMATH_LIE_SE2_H
#define UEF_UMATH_LIE_SE2_H
#include <uef/umath/vector2.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { umath_vec2_t translation; umath_scalar_t angle_radians; } umath_pose2_t;
/// Tangent order is [rho_x,rho_y,theta]; pose maps local coordinates actively into the parent frame.
umath_status_t umath_se2_exp(const umath_scalar_t tangent[3], umath_pose2_t *pose);
/// TODO(UMATH-SE2): Define angle branch, small-angle translation Jacobian, and output preservation.
umath_status_t umath_se2_log(const umath_pose2_t *pose, umath_scalar_t tangent[3]);
/// TODO(UMATH-SE2): Emit row-major adjoint in [rho_x,rho_y,theta] order.
umath_status_t umath_se2_adjoint(const umath_pose2_t *pose, umath_scalar_t adjoint_row_major[9]);
#ifdef __cplusplus
}
#endif
#endif