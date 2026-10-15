/// @file include/umath/geometry/quaternion.h
/// @brief Scalar-first Hamilton quaternions for right-handed 3D rotations.
#ifndef UETD_UMATH_QUATERNION_H
#define UETD_UMATH_QUATERNION_H
#include <umath/config.h>
#include <umath/vector/vector3.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Storage is w,x,y,z. Unit q actively rotates vectors from local to parent.
typedef struct {
    umath_scalar_t w, x, y, z;
} umath_quaternion_t;

/// Matrix conversion uses an active row-major 3x3 proper rotation. Slerp follows the shortest path for t in [0,1].
/// Quaternion log uses the principal branch; a negative real input chooses the positive x axis at pi.
umath_quaternion_t umath_quaternion_identity(void);
umath_quaternion_t umath_quaternion_conjugate(umath_quaternion_t value);
umath_quaternion_t umath_quaternion_multiply(umath_quaternion_t left, umath_quaternion_t right);
umath_status_t umath_quaternion_normalize(umath_quaternion_t value, umath_quaternion_t* unit);
umath_status_t umath_quaternion_rotate(umath_quaternion_t rotation, umath_vec3_t input, umath_vec3_t* output);
umath_status_t umath_quaternion_from_axis_angle(umath_vec3_t axis, umath_scalar_t angle_radians, umath_quaternion_t* rotation);
umath_status_t umath_quaternion_to_axis_angle(umath_quaternion_t rotation, umath_vec3_t* axis, umath_scalar_t* angle_radians);
umath_status_t umath_quaternion_dot(umath_quaternion_t left, umath_quaternion_t right, umath_scalar_t* dot);
umath_status_t umath_quaternion_norm(umath_quaternion_t value, umath_scalar_t* norm);
umath_status_t umath_quaternion_from_matrix(const umath_scalar_t* row_major_3x3, umath_quaternion_t* rotation);
umath_status_t umath_quaternion_to_matrix(umath_quaternion_t rotation, umath_scalar_t* row_major_3x3);
umath_status_t umath_quaternion_slerp(umath_quaternion_t start, umath_quaternion_t end, umath_scalar_t t, umath_quaternion_t* result);
umath_status_t umath_quaternion_exp(umath_quaternion_t value, umath_quaternion_t* result);
umath_status_t umath_quaternion_log(umath_quaternion_t value, umath_quaternion_t* result);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_QUATERNION_H */
