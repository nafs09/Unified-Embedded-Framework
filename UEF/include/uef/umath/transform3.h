/// @file include/uef/umath/transform3.h
/// @brief Rigid transform between named right-handed three-dimensional frames.
#ifndef UEF_UMATH_TRANSFORM3_H
#define UEF_UMATH_TRANSFORM3_H

#include <uef/umath/quaternion.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Maps a point in a child/local frame into its parent frame.
typedef struct {
    umath_quaternion_t rotation;
    umath_vec3_t translation;
} umath_transform3_t;

umath_transform3_t umath_transform3_identity(void);
umath_status_t umath_transform3_compose(const umath_transform3_t *parent_from_mid,
                                        const umath_transform3_t *mid_from_child,
                                        umath_transform3_t *parent_from_child);
umath_status_t umath_transform3_inverse(const umath_transform3_t *parent_from_child,
                                        umath_transform3_t *child_from_parent);
umath_status_t umath_transform3_point(const umath_transform3_t *parent_from_child,
                                      umath_vec3_t point_child,
                                      umath_vec3_t *point_parent);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UMATH_TRANSFORM3_H */