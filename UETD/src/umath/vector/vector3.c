/// @file src/umath/vector/vector3.c
/// @brief Three-dimensional vector operations in a right-handed coordinate basis.
#include <math.h>
#include <stddef.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>
#include <umath/vector/vector3.h>

umath_vec3_t umath_vec3_add(umath_vec3_t a, umath_vec3_t b) {
    const umath_vec3_t result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

umath_vec3_t umath_vec3_sub(umath_vec3_t a, umath_vec3_t b) {
    const umath_vec3_t result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

umath_vec3_t umath_vec3_scale(umath_vec3_t value, umath_scalar_t scale) {
    const umath_vec3_t result = {value.x * scale, value.y * scale, value.z * scale};
    return result;
}

umath_scalar_t umath_vec3_dot(umath_vec3_t a, umath_vec3_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

umath_vec3_t umath_vec3_cross(umath_vec3_t a, umath_vec3_t b) {
    const umath_vec3_t result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
    return result;
}

umath_status_t umath_vec3_norm(umath_vec3_t value, umath_scalar_t* norm) {
    const umath_accumulator_t xy = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.x, (umath_accumulator_t)value.y);
    const umath_accumulator_t result = UETD_UMATH_ACCUM_BINARY_MATH(hypot, xy, (umath_accumulator_t)value.z);
    if (norm == NULL) return UMATH_INVALID_ARGUMENT;
    if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) { return UMATH_NUMERIC_FAILURE; }
    *norm = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_vec3_normalize(umath_vec3_t value, umath_vec3_t* unit) {
    umath_scalar_t norm;
    umath_vec3_t result;
    umath_status_t status;
    if (unit == NULL) return UMATH_INVALID_ARGUMENT;
    status = umath_vec3_norm(value, &norm);
    if (status != UMATH_OK) return status;
    if (norm <= 0) return UMATH_INVALID_ARGUMENT;
    result.x = value.x / norm;
    result.y = value.y / norm;
    result.z = value.z / norm;
    *unit = result;
    return UMATH_OK;
}

static bool vec3_accum(umath_vec3_t value, umath_accumulator_t out[3]) {
    if (!umath_scalar_is_finite(value.x) || !umath_scalar_is_finite(value.y) || !umath_scalar_is_finite(value.z)) return false;
    out[0] = (umath_accumulator_t)value.x; out[1] = (umath_accumulator_t)value.y; out[2] = (umath_accumulator_t)value.z;
    return true;
}

static umath_status_t vec3_commit(const umath_accumulator_t in[3], umath_vec3_t* result) {
    umath_vec3_t converted = {(umath_scalar_t)in[0], (umath_scalar_t)in[1], (umath_scalar_t)in[2]};
    if (!umath_accumulator_is_finite(in[0]) || !umath_accumulator_is_finite(in[1]) || !umath_accumulator_is_finite(in[2]) ||
        !umath_scalar_is_finite(converted.x) || !umath_scalar_is_finite(converted.y) || !umath_scalar_is_finite(converted.z))
        return UMATH_NUMERIC_FAILURE;
    *result = converted;
    return UMATH_OK;
}

umath_status_t umath_vec3_mul(umath_vec3_t left, umath_vec3_t right, umath_vec3_t* result) {
    umath_accumulator_t a[3], b[3], c[3]; size_t i;
    if (result == NULL || !vec3_accum(left, a) || !vec3_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < 3; ++i) c[i] = a[i] * b[i];
    return vec3_commit(c, result);
}

umath_status_t umath_vec3_div(umath_vec3_t left, umath_vec3_t right, umath_vec3_t* result) {
    umath_accumulator_t a[3], b[3], c[3]; size_t i;
    if (result == NULL || !vec3_accum(left, a) || !vec3_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < 3; ++i) if (b[i] == 0) return UMATH_SINGULAR;
    for (i = 0; i < 3; ++i) c[i] = a[i] / b[i];
    return vec3_commit(c, result);
}

umath_status_t umath_vec3_distance(umath_vec3_t left, umath_vec3_t right, umath_scalar_t* distance) {
    umath_accumulator_t a[3], b[3], d, xy; umath_scalar_t converted;
    if (distance == NULL || !vec3_accum(left, a) || !vec3_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    xy = UETD_UMATH_ACCUM_BINARY_MATH(hypot, a[0] - b[0], a[1] - b[1]);
    d = UETD_UMATH_ACCUM_BINARY_MATH(hypot, xy, a[2] - b[2]); converted = (umath_scalar_t)d;
    if (!umath_accumulator_is_finite(d) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *distance = converted;
    return UMATH_OK;
}

umath_status_t umath_vec3_lerp(umath_vec3_t start, umath_vec3_t end, umath_scalar_t t, umath_vec3_t* result) {
    umath_accumulator_t a[3], b[3], c[3], weight; size_t i;
    if (result == NULL || !umath_scalar_is_finite(t) || !vec3_accum(start, a) || !vec3_accum(end, b)) return UMATH_INVALID_ARGUMENT;
    weight = (umath_accumulator_t)t;
    for (i = 0; i < 3; ++i) c[i] = (UMATH_ACCUMULATOR_C(1) - weight) * a[i] + weight * b[i];
    return vec3_commit(c, result);
}

umath_status_t umath_vec3_angle(umath_vec3_t left, umath_vec3_t right, umath_scalar_t* radians) {
    umath_accumulator_t a[3], b[3], sa, sb, cross[3], cross_norm, dot; umath_scalar_t converted;
    if (radians == NULL || !vec3_accum(left, a) || !vec3_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    sa = UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_UNARY_MATH(fabs, a[0]), UETD_UMATH_ACCUM_UNARY_MATH(fabs, a[1])), UETD_UMATH_ACCUM_UNARY_MATH(fabs, a[2]));
    sb = UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_UNARY_MATH(fabs, b[0]), UETD_UMATH_ACCUM_UNARY_MATH(fabs, b[1])), UETD_UMATH_ACCUM_UNARY_MATH(fabs, b[2]));
    if (sa == 0 || sb == 0) return UMATH_SINGULAR;
    for (size_t i = 0; i < 3; ++i) { a[i] /= sa; b[i] /= sb; }
    cross[0] = a[1]*b[2] - a[2]*b[1]; cross[1] = a[2]*b[0] - a[0]*b[2]; cross[2] = a[0]*b[1] - a[1]*b[0];
    cross_norm = UETD_UMATH_ACCUM_BINARY_MATH(hypot, UETD_UMATH_ACCUM_BINARY_MATH(hypot, cross[0], cross[1]), cross[2]);
    dot = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
    converted = (umath_scalar_t)UETD_UMATH_ACCUM_BINARY_MATH(atan2, cross_norm, dot);
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *radians = converted;
    return UMATH_OK;
}

static umath_status_t vec3_orthogonal(umath_vec3_t value, umath_vec3_t axis, umath_vec3_t* output, int mode) {
    umath_accumulator_t v[3], n[3], maximum, denom, factor, p[3]; size_t i;
    if (output == NULL || !vec3_accum(value, v) || !vec3_accum(axis, n)) return UMATH_INVALID_ARGUMENT;
    maximum = UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_UNARY_MATH(fabs, n[0]), UETD_UMATH_ACCUM_UNARY_MATH(fabs, n[1])), UETD_UMATH_ACCUM_UNARY_MATH(fabs, n[2]));
    if (maximum == 0) return UMATH_SINGULAR;
    for (i = 0; i < 3; ++i) n[i] /= maximum;
    denom = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
    factor = (v[0]*n[0] + v[1]*n[1] + v[2]*n[2]) / denom;
    for (i = 0; i < 3; ++i) p[i] = factor * n[i];
    if (mode == 1) for (i = 0; i < 3; ++i) p[i] = v[i] - p[i];
    else if (mode == 2) for (i = 0; i < 3; ++i) p[i] = v[i] - UMATH_ACCUMULATOR_C(2) * p[i];
    return vec3_commit(p, output);
}

umath_status_t umath_vec3_project(umath_vec3_t value, umath_vec3_t axis, umath_vec3_t* projection) { return vec3_orthogonal(value, axis, projection, 0); }
umath_status_t umath_vec3_reject(umath_vec3_t value, umath_vec3_t axis, umath_vec3_t* rejection) { return vec3_orthogonal(value, axis, rejection, 1); }
umath_status_t umath_vec3_reflect(umath_vec3_t value, umath_vec3_t normal, umath_vec3_t* reflected) { return vec3_orthogonal(value, normal, reflected, 2); }
