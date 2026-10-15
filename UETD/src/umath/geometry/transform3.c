/// @file src/umath/geometry/transform3.c
/// @brief Composition and application of rigid frame transforms.
#include <float.h>
#include <math.h>
#include <umath/geometry/transform3.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>

static umath_accumulator_t transform_tolerance(void) {
    const umath_accumulator_t epsilon = sizeof(umath_scalar_t) <= sizeof(float) ||
                                                sizeof(umath_accumulator_t) <= sizeof(float)
                                            ? (umath_accumulator_t)FLT_EPSILON
                                            : (umath_accumulator_t)DBL_EPSILON;
    return UMATH_ACCUMULATOR_C(32) * epsilon;
}

static umath_accumulator_t transform_determinant3(const umath_accumulator_t matrix[16]) {
    const umath_accumulator_t a = matrix[0], b = matrix[1], c = matrix[2];
    const umath_accumulator_t d = matrix[4], e = matrix[5], f = matrix[6];
    const umath_accumulator_t g = matrix[8], h = matrix[9], i = matrix[10];
    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

static umath_status_t transform_add_translation(umath_vec3_t left, umath_vec3_t right, umath_vec3_t* result) {
    umath_accumulator_t x, y, z;
    umath_vec3_t converted;
    if (result == NULL || !umath_scalar_is_finite(left.x) || !umath_scalar_is_finite(left.y) ||
        !umath_scalar_is_finite(left.z) || !umath_scalar_is_finite(right.x) ||
        !umath_scalar_is_finite(right.y) || !umath_scalar_is_finite(right.z)) {
        return UMATH_INVALID_ARGUMENT;
    }
    x = (umath_accumulator_t)left.x + (umath_accumulator_t)right.x;
    y = (umath_accumulator_t)left.y + (umath_accumulator_t)right.y;
    z = (umath_accumulator_t)left.z + (umath_accumulator_t)right.z;
    converted.x = (umath_scalar_t)x;
    converted.y = (umath_scalar_t)y;
    converted.z = (umath_scalar_t)z;
    if (!umath_accumulator_is_finite(x) || !umath_accumulator_is_finite(y) || !umath_accumulator_is_finite(z) ||
        !umath_scalar_is_finite(converted.x) || !umath_scalar_is_finite(converted.y) ||
        !umath_scalar_is_finite(converted.z)) {
        return UMATH_NUMERIC_FAILURE;
    }
    *result = converted;
    return UMATH_OK;
}

umath_transform3_t umath_transform3_identity(void) {
    umath_transform3_t result;
    result.rotation = umath_quaternion_identity();
    result.translation.x = 0;
    result.translation.y = 0;
    result.translation.z = 0;
    return result;
}

umath_status_t umath_transform3_compose(const umath_transform3_t* parent_from_mid, const umath_transform3_t* mid_from_child,
                                        umath_transform3_t* parent_from_child) {
    umath_transform3_t result;
    umath_vec3_t translated;
    umath_status_t status;
    if (parent_from_mid == NULL || mid_from_child == NULL || parent_from_child == NULL) return UMATH_INVALID_ARGUMENT;
    if (umath_quaternion_rotate(parent_from_mid->rotation, mid_from_child->translation, &translated) != UMATH_OK) {
        return UMATH_NUMERIC_FAILURE;
    }
    result.rotation = umath_quaternion_multiply(parent_from_mid->rotation, mid_from_child->rotation);
    if (umath_quaternion_normalize(result.rotation, &result.rotation) != UMATH_OK) { return UMATH_NUMERIC_FAILURE; }
    status = transform_add_translation(parent_from_mid->translation, translated, &result.translation);
    if (status != UMATH_OK) return status;
    *parent_from_child = result;
    return UMATH_OK;
}

umath_status_t umath_transform3_inverse(const umath_transform3_t* parent_from_child, umath_transform3_t* child_from_parent) {
    umath_transform3_t result;
    umath_vec3_t negative_translation;
    if (parent_from_child == NULL || child_from_parent == NULL) { return UMATH_INVALID_ARGUMENT; }
    result.rotation = umath_quaternion_conjugate(parent_from_child->rotation);
    negative_translation = umath_vec3_scale(parent_from_child->translation, -1);
    if (umath_quaternion_rotate(result.rotation, negative_translation, &result.translation) != UMATH_OK) {
        return UMATH_NUMERIC_FAILURE;
    }
    *child_from_parent = result;
    return UMATH_OK;
}

umath_status_t umath_transform3_point(const umath_transform3_t* parent_from_child, umath_vec3_t point_child,
                                      umath_vec3_t* point_parent) {
    umath_vec3_t rotated;
    if (parent_from_child == NULL || point_parent == NULL) { return UMATH_INVALID_ARGUMENT; }
    if (umath_quaternion_rotate(parent_from_child->rotation, point_child, &rotated) != UMATH_OK) { return UMATH_NUMERIC_FAILURE; }
    return transform_add_translation(rotated, parent_from_child->translation, point_parent);
}

umath_status_t umath_transform3_from_matrix(const umath_scalar_t* row_major_4x4, umath_transform3_t* result) {
    umath_accumulator_t matrix[16];
    umath_accumulator_t trace, scale;
    umath_quaternion_t quaternion;
    umath_transform3_t converted;
    umath_status_t status;
    umath_accumulator_t tolerance;
    size_t index, row, column;
    if (row_major_4x4 == NULL || result == NULL) return UMATH_INVALID_ARGUMENT;
    tolerance = transform_tolerance();
    for (index = 0U; index < 16U; ++index) {
        if (!umath_scalar_is_finite(row_major_4x4[index])) return UMATH_INVALID_ARGUMENT;
        matrix[index] = (umath_accumulator_t)row_major_4x4[index];
    }
    if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, matrix[12]) > tolerance ||
        UETD_UMATH_ACCUM_UNARY_MATH(fabs, matrix[13]) > tolerance ||
        UETD_UMATH_ACCUM_UNARY_MATH(fabs, matrix[14]) > tolerance ||
        UETD_UMATH_ACCUM_UNARY_MATH(fabs, matrix[15] - UMATH_ACCUMULATOR_C(1)) > tolerance)
        return UMATH_INVALID_ARGUMENT;
    for (row = 0U; row < 3U; ++row) {
        for (column = row; column < 3U; ++column) {
            umath_accumulator_t dot = UMATH_ACCUMULATOR_C(0);
            size_t component;
            for (component = 0U; component < 3U; ++component)
                dot += matrix[row * 4U + component] * matrix[column * 4U + component];
            if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, dot - (row == column ? UMATH_ACCUMULATOR_C(1)
                                                                       : UMATH_ACCUMULATOR_C(0))) > tolerance)
                return UMATH_INVALID_ARGUMENT;
        }
    }
    if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, transform_determinant3(matrix) - UMATH_ACCUMULATOR_C(1)) >
        UMATH_ACCUMULATOR_C(4) * tolerance) return UMATH_INVALID_ARGUMENT;

    trace = matrix[0] + matrix[5] + matrix[10];
    if (trace > UMATH_ACCUMULATOR_C(0)) {
        scale = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, trace + UMATH_ACCUMULATOR_C(1));
        quaternion.w = (umath_scalar_t)(scale / UMATH_ACCUMULATOR_C(4));
        quaternion.x = (umath_scalar_t)((matrix[9] - matrix[6]) / scale);
        quaternion.y = (umath_scalar_t)((matrix[2] - matrix[8]) / scale);
        quaternion.z = (umath_scalar_t)((matrix[4] - matrix[1]) / scale);
    } else if (matrix[0] > matrix[5] && matrix[0] > matrix[10]) {
        scale = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(
                    sqrt, UMATH_ACCUMULATOR_C(1) + matrix[0] - matrix[5] - matrix[10]);
        quaternion.w = (umath_scalar_t)((matrix[9] - matrix[6]) / scale);
        quaternion.x = (umath_scalar_t)(scale / UMATH_ACCUMULATOR_C(4));
        quaternion.y = (umath_scalar_t)((matrix[1] + matrix[4]) / scale);
        quaternion.z = (umath_scalar_t)((matrix[2] + matrix[8]) / scale);
    } else if (matrix[5] > matrix[10]) {
        scale = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(
                    sqrt, UMATH_ACCUMULATOR_C(1) + matrix[5] - matrix[0] - matrix[10]);
        quaternion.w = (umath_scalar_t)((matrix[2] - matrix[8]) / scale);
        quaternion.x = (umath_scalar_t)((matrix[1] + matrix[4]) / scale);
        quaternion.y = (umath_scalar_t)(scale / UMATH_ACCUMULATOR_C(4));
        quaternion.z = (umath_scalar_t)((matrix[6] + matrix[9]) / scale);
    } else {
        scale = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_UNARY_MATH(
                    sqrt, UMATH_ACCUMULATOR_C(1) + matrix[10] - matrix[0] - matrix[5]);
        quaternion.w = (umath_scalar_t)((matrix[4] - matrix[1]) / scale);
        quaternion.x = (umath_scalar_t)((matrix[2] + matrix[8]) / scale);
        quaternion.y = (umath_scalar_t)((matrix[6] + matrix[9]) / scale);
        quaternion.z = (umath_scalar_t)(scale / UMATH_ACCUMULATOR_C(4));
    }
    if (!umath_accumulator_is_finite(scale) || scale <= UMATH_ACCUMULATOR_C(0) ||
        !umath_scalar_is_finite(quaternion.w) || !umath_scalar_is_finite(quaternion.x) ||
        !umath_scalar_is_finite(quaternion.y) || !umath_scalar_is_finite(quaternion.z)) return UMATH_NUMERIC_FAILURE;
    status = umath_quaternion_normalize(quaternion, &converted.rotation);
    if (status != UMATH_OK) return status;
    converted.translation.x = row_major_4x4[3];
    converted.translation.y = row_major_4x4[7];
    converted.translation.z = row_major_4x4[11];
    *result = converted;
    return UMATH_OK;
}

umath_status_t umath_transform3_to_matrix(const umath_transform3_t* transform, umath_scalar_t* row_major_4x4) {
    umath_quaternion_t q;
    umath_scalar_t converted[16];
    umath_accumulator_t matrix[16];
    umath_status_t status;
    umath_accumulator_t w, x, y, z;
    size_t index;
    if (transform == NULL || row_major_4x4 == NULL) return UMATH_INVALID_ARGUMENT;
    if (!umath_scalar_is_finite(transform->translation.x) || !umath_scalar_is_finite(transform->translation.y) ||
        !umath_scalar_is_finite(transform->translation.z)) return UMATH_INVALID_ARGUMENT;
    status = umath_quaternion_normalize(transform->rotation, &q);
    if (status != UMATH_OK) return status;
    w = (umath_accumulator_t)q.w;
    x = (umath_accumulator_t)q.x;
    y = (umath_accumulator_t)q.y;
    z = (umath_accumulator_t)q.z;
    for (index = 0U; index < 16U; ++index) matrix[index] = UMATH_ACCUMULATOR_C(0);
    matrix[0] = UMATH_ACCUMULATOR_C(1) - UMATH_ACCUMULATOR_C(2) * (y * y + z * z);
    matrix[1] = UMATH_ACCUMULATOR_C(2) * (x * y - z * w);
    matrix[2] = UMATH_ACCUMULATOR_C(2) * (x * z + y * w);
    matrix[3] = (umath_accumulator_t)transform->translation.x;
    matrix[4] = UMATH_ACCUMULATOR_C(2) * (x * y + z * w);
    matrix[5] = UMATH_ACCUMULATOR_C(1) - UMATH_ACCUMULATOR_C(2) * (x * x + z * z);
    matrix[6] = UMATH_ACCUMULATOR_C(2) * (y * z - x * w);
    matrix[7] = (umath_accumulator_t)transform->translation.y;
    matrix[8] = UMATH_ACCUMULATOR_C(2) * (x * z - y * w);
    matrix[9] = UMATH_ACCUMULATOR_C(2) * (y * z + x * w);
    matrix[10] = UMATH_ACCUMULATOR_C(1) - UMATH_ACCUMULATOR_C(2) * (x * x + y * y);
    matrix[11] = (umath_accumulator_t)transform->translation.z;
    matrix[15] = UMATH_ACCUMULATOR_C(1);
    for (index = 0U; index < 16U; ++index) {
        if (!umath_accumulator_is_finite(matrix[index])) return UMATH_NUMERIC_FAILURE;
        converted[index] = (umath_scalar_t)matrix[index];
        if (!umath_scalar_is_finite(converted[index])) return UMATH_NUMERIC_FAILURE;
    }
    for (index = 0U; index < 16U; ++index) row_major_4x4[index] = converted[index];
    return UMATH_OK;
}

umath_status_t umath_transform3_interpolate(const umath_transform3_t* start, const umath_transform3_t* end, umath_scalar_t t, umath_transform3_t* result) {
    umath_transform3_t converted;
    umath_quaternion_t q0, q1, q;
    umath_status_t status;
    umath_accumulator_t dot, theta, denominator, weight0, weight1, parameter;
    umath_accumulator_t components[4];
    umath_scalar_t* q_components[4];
    size_t index;
    if (start == NULL || end == NULL || result == NULL || !umath_scalar_is_finite(t) ||
        t < UMATH_SCALAR_C(0) || t > UMATH_SCALAR_C(1)) return UMATH_INVALID_ARGUMENT;
    if (!umath_scalar_is_finite(start->translation.x) || !umath_scalar_is_finite(start->translation.y) ||
        !umath_scalar_is_finite(start->translation.z) || !umath_scalar_is_finite(end->translation.x) ||
        !umath_scalar_is_finite(end->translation.y) || !umath_scalar_is_finite(end->translation.z))
        return UMATH_INVALID_ARGUMENT;
    status = umath_quaternion_normalize(start->rotation, &q0);
    if (status != UMATH_OK) return status;
    status = umath_quaternion_normalize(end->rotation, &q1);
    if (status != UMATH_OK) return status;
    dot = (umath_accumulator_t)q0.w * q1.w + (umath_accumulator_t)q0.x * q1.x +
          (umath_accumulator_t)q0.y * q1.y + (umath_accumulator_t)q0.z * q1.z;
    if (!umath_accumulator_is_finite(dot)) return UMATH_NUMERIC_FAILURE;
    if (dot < UMATH_ACCUMULATOR_C(0)) {
        q1.w = -q1.w; q1.x = -q1.x; q1.y = -q1.y; q1.z = -q1.z;
        dot = -dot;
    }
    if (dot > UMATH_ACCUMULATOR_C(1)) dot = UMATH_ACCUMULATOR_C(1);
    parameter = (umath_accumulator_t)t;
    if (dot > UMATH_ACCUMULATOR_C(0.9995)) {
        weight0 = UMATH_ACCUMULATOR_C(1) - parameter;
        weight1 = parameter;
    } else {
        theta = UETD_UMATH_ACCUM_UNARY_MATH(acos, dot);
        denominator = UETD_UMATH_ACCUM_UNARY_MATH(sin, theta);
        if (!umath_accumulator_is_finite(denominator) || denominator == UMATH_ACCUMULATOR_C(0))
            return UMATH_NUMERIC_FAILURE;
        weight0 = UETD_UMATH_ACCUM_UNARY_MATH(sin, (UMATH_ACCUMULATOR_C(1) - parameter) * theta) / denominator;
        weight1 = UETD_UMATH_ACCUM_UNARY_MATH(sin, parameter * theta) / denominator;
    }
    components[0] = weight0 * q0.w + weight1 * q1.w;
    components[1] = weight0 * q0.x + weight1 * q1.x;
    components[2] = weight0 * q0.y + weight1 * q1.y;
    components[3] = weight0 * q0.z + weight1 * q1.z;
    q_components[0] = &q.w; q_components[1] = &q.x; q_components[2] = &q.y; q_components[3] = &q.z;
    for (index = 0U; index < 4U; ++index) {
        if (!umath_accumulator_is_finite(components[index])) return UMATH_NUMERIC_FAILURE;
        *q_components[index] = (umath_scalar_t)components[index];
        if (!umath_scalar_is_finite(*q_components[index])) return UMATH_NUMERIC_FAILURE;
    }
    status = umath_quaternion_normalize(q, &converted.rotation);
    if (status != UMATH_OK) return status;
    converted.translation.x = (umath_scalar_t)UETD_UMATH_ACCUM_FMA(
        parameter, (umath_accumulator_t)end->translation.x - start->translation.x, start->translation.x);
    converted.translation.y = (umath_scalar_t)UETD_UMATH_ACCUM_FMA(
        parameter, (umath_accumulator_t)end->translation.y - start->translation.y, start->translation.y);
    converted.translation.z = (umath_scalar_t)UETD_UMATH_ACCUM_FMA(
        parameter, (umath_accumulator_t)end->translation.z - start->translation.z, start->translation.z);
    if (!umath_scalar_is_finite(converted.translation.x) || !umath_scalar_is_finite(converted.translation.y) ||
        !umath_scalar_is_finite(converted.translation.z)) return UMATH_NUMERIC_FAILURE;
    *result = converted;
    return UMATH_OK;
}

