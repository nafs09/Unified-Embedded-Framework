/// @file include/uef/umath/quaternion.h
/// @brief Scalar-first Hamilton quaternions for right-handed 3D rotations.
#ifndef UEF_UMATH_QUATERNION_H
#define UEF_UMATH_QUATERNION_H

#include <uef/umath/vector3.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Storage order is w,x,y,z. Unit q actively rotates a vector from local to parent.
typedef struct { umath_scalar_t w, x, y, z; } umath_quaternion_t;

umath_quaternion_t umath_quaternion_identity(void);
umath_quaternion_t umath_quaternion_conjugate(umath_quaternion_t value);
umath_quaternion_t umath_quaternion_multiply(umath_quaternion_t left,
                                            umath_quaternion_t right);
umath_status_t umath_quaternion_normalize(umath_quaternion_t value,
                                          umath_quaternion_t *unit);
umath_status_t umath_quaternion_rotate(umath_quaternion_t rotation,
                                       umath_vec3_t input,
                                       umath_vec3_t *output);
umath_status_t umath_quaternion_from_axis_angle(umath_vec3_t axis,
                                                umath_scalar_t angle_radians,
                                                umath_quaternion_t *rotation);
umath_status_t umath_quaternion_to_axis_angle(umath_quaternion_t rotation,
                                              umath_vec3_t *axis,
                                              umath_scalar_t *angle_radians);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_QUATERNION_H */