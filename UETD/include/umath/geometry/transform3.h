/// @file include/umath/geometry/transform3.h
/// @brief Rigid transform between named right-handed three-dimensional frames.
#ifndef UETD_UMATH_TRANSFORM3_H
#define UETD_UMATH_TRANSFORM3_H

#include <umath/geometry/quaternion.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Maps a point in a child/local frame into its parent frame.
typedef struct {
    umath_quaternion_t rotation;
    umath_vec3_t translation;
} umath_transform3_t;

/// Homogeneous conversion checks the bottom row and rotation orthogonality within 32 epsilons of the
/// less-precise configured scalar/accumulator type, and determinant one within 128 such epsilons.
/// Interpolation uses shortest-path rotation slerp and requires t in [0,1].

umath_transform3_t umath_transform3_identity(void);
umath_status_t umath_transform3_compose(const umath_transform3_t* parent_from_mid, const umath_transform3_t* mid_from_child,
                                        umath_transform3_t* parent_from_child);
umath_status_t umath_transform3_inverse(const umath_transform3_t* parent_from_child, umath_transform3_t* child_from_parent);
umath_status_t umath_transform3_point(const umath_transform3_t* parent_from_child, umath_vec3_t point_child,
                                      umath_vec3_t* point_parent);

/// Convert to/from the active row-major homogeneous transform [R t; 0 1].

umath_status_t umath_transform3_from_matrix(const umath_scalar_t* row_major_4x4, umath_transform3_t* result);
umath_status_t umath_transform3_to_matrix(const umath_transform3_t* transform, umath_scalar_t* row_major_4x4);
umath_status_t umath_transform3_interpolate(const umath_transform3_t* start, const umath_transform3_t* end, umath_scalar_t t, umath_transform3_t* result);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UMATH_TRANSFORM3_H */
