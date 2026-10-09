/// @file include/uef/umath/lie/so3.h
/// @brief Planned SO(3) tangent, exponential/logarithm, and Jacobian operations.
#ifndef UEF_UMATH_LIE_SO3_H
#define UEF_UMATH_LIE_SO3_H
#include <uef/umath/config.h>
#include <uef/umath/quaternion.h>
#include <uef/umath/vector3.h>
#ifdef __cplusplus
extern "C" {
#endif
/// TODO(UMATH-SO3): Define skew signs and row-major [phi]x convention.
umath_status_t umath_so3_hat(umath_vec3_t tangent, umath_scalar_t skew_row_major[9]);
/// TODO(UMATH-SO3): Validate skew symmetry under the documented tolerance.
umath_status_t umath_so3_vee(const umath_scalar_t skew_row_major[9], umath_vec3_t *tangent);
/// TODO(UMATH-SO3): Use stable small-angle series and return normalized scalar-first quaternion.
umath_status_t umath_so3_exp(umath_vec3_t tangent, umath_quaternion_t *rotation);
/// TODO(UMATH-SO3): Normalize first, choose shortest principal rotation, and define behavior near zero and pi.
umath_status_t umath_so3_log(umath_quaternion_t rotation, umath_vec3_t *tangent);
/// TODO(UMATH-SO3): Implement left Jacobian and stable small-angle series.
umath_status_t umath_so3_left_jacobian(umath_vec3_t tangent, umath_scalar_t row_major[9]);
/// TODO(UMATH-SO3): Define singularity handling for the inverse left Jacobian.
umath_status_t umath_so3_left_jacobian_inverse(umath_vec3_t tangent, umath_scalar_t row_major[9]);
/// TODO(UMATH-SO3): Pin right-perturbation convention and confirm relation to the left Jacobian.
umath_status_t umath_so3_right_jacobian(umath_vec3_t tangent, umath_scalar_t row_major[9]);
/// TODO(UMATH-SO3): Implement the right-Jacobian inverse with bounded small-angle handling.
umath_status_t umath_so3_right_jacobian_inverse(umath_vec3_t tangent, umath_scalar_t row_major[9]);
#ifdef __cplusplus
}
#endif
#endif