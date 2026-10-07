/// @file src/umath/transform3.c
/// @brief Composition and application of rigid frame transforms.
#include <uef/umath/transform3.h>

umath_transform3_t umath_transform3_identity(void)
{
    umath_transform3_t result;
    result.rotation = umath_quaternion_identity();
    result.translation.x = 0;
    result.translation.y = 0;
    result.translation.z = 0;
    return result;
}

umath_status_t umath_transform3_compose(const umath_transform3_t *parent_from_mid,
                                        const umath_transform3_t *mid_from_child,
                                        umath_transform3_t *parent_from_child)
{
    umath_transform3_t result;
    umath_vec3_t translated;
    if (parent_from_mid == NULL || mid_from_child == NULL ||
        parent_from_child == NULL) return UMATH_INVALID_ARGUMENT;
    if (umath_quaternion_rotate(parent_from_mid->rotation,
                                mid_from_child->translation, &translated) != UMATH_OK) {
        return UMATH_NUMERIC_FAILURE;
    }
    result.rotation = umath_quaternion_multiply(parent_from_mid->rotation,
                                                mid_from_child->rotation);
    if (umath_quaternion_normalize(result.rotation, &result.rotation) != UMATH_OK) {
        return UMATH_NUMERIC_FAILURE;
    }
    result.translation = umath_vec3_add(parent_from_mid->translation, translated);
    *parent_from_child = result;
    return UMATH_OK;
}

umath_status_t umath_transform3_inverse(const umath_transform3_t *parent_from_child,
                                        umath_transform3_t *child_from_parent)
{
    umath_transform3_t result;
    umath_vec3_t negative_translation;
    if (parent_from_child == NULL || child_from_parent == NULL) {
        return UMATH_INVALID_ARGUMENT;
    }
    result.rotation = umath_quaternion_conjugate(parent_from_child->rotation);
    negative_translation = umath_vec3_scale(parent_from_child->translation, -1);
    if (umath_quaternion_rotate(result.rotation, negative_translation,
                                &result.translation) != UMATH_OK) {
        return UMATH_NUMERIC_FAILURE;
    }
    *child_from_parent = result;
    return UMATH_OK;
}

umath_status_t umath_transform3_point(const umath_transform3_t *parent_from_child,
                                      umath_vec3_t point_child,
                                      umath_vec3_t *point_parent)
{
    umath_vec3_t rotated;
    if (parent_from_child == NULL || point_parent == NULL) {
        return UMATH_INVALID_ARGUMENT;
    }
    if (umath_quaternion_rotate(parent_from_child->rotation, point_child,
                                &rotated) != UMATH_OK) {
        return UMATH_NUMERIC_FAILURE;
    }
    *point_parent = umath_vec3_add(rotated, parent_from_child->translation);
    return UMATH_OK;
}