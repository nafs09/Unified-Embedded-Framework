/// @file include/uef/umath/lie/se3.h
/// @brief Planned spatial rigid-pose and tangent operations for robotics/estimation.
#ifndef UEF_UMATH_LIE_SE3_H
#define UEF_UMATH_LIE_SE3_H
#include <uef/umath/config.h>
#include <uef/umath/transform3.h>
#ifdef __cplusplus
extern "C" {
#endif
/// Tangent order is [rho_x,rho_y,rho_z,phi_x,phi_y,phi_z]; translation and angle units belong to the caller.
umath_status_t umath_se3_exp(const umath_scalar_t tangent[6], umath_transform3_t *transform);
/// TODO(UMATH-SE3): Pin transform direction, principal SO(3) branch, and translation inverse-Jacobian behavior.
umath_status_t umath_se3_log(const umath_transform3_t *transform, umath_scalar_t tangent[6]);
/// TODO(UMATH-SE3): Define row-major adjoint blocks for rho-then-phi tangent order.
umath_status_t umath_se3_adjoint(const umath_transform3_t *transform, umath_scalar_t adjoint_row_major[36]);
/// TODO(UMATH-SE3): Implement bounded left Jacobian using the selected SO(3) convention.
umath_status_t umath_se3_left_jacobian(const umath_scalar_t tangent[6], umath_scalar_t row_major[36]);
/// TODO(UMATH-SE3): Implement its stable inverse and define singularity behavior.
umath_status_t umath_se3_left_jacobian_inverse(const umath_scalar_t tangent[6], umath_scalar_t row_major[36]);
#ifdef __cplusplus
}
#endif
#endif