/// @file src/umath/vector/vector4.c
/// @brief Four-dimensional vector operations.
#include <math.h>
#include <stddef.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>
#include <umath/vector/vector4.h>

umath_vec4_t umath_vec4_add(umath_vec4_t a, umath_vec4_t b) {
    const umath_vec4_t result = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    return result;
}

umath_vec4_t umath_vec4_sub(umath_vec4_t a, umath_vec4_t b) {
    const umath_vec4_t result = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    return result;
}

umath_vec4_t umath_vec4_scale(umath_vec4_t value, umath_scalar_t scale) {
    const umath_vec4_t result = {value.x * scale, value.y * scale, value.z * scale, value.w * scale};
    return result;
}

umath_scalar_t umath_vec4_dot(umath_vec4_t a, umath_vec4_t b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

umath_status_t umath_vec4_norm(umath_vec4_t value, umath_scalar_t* norm) {
    const umath_accumulator_t x = (umath_accumulator_t)value.x;
    const umath_accumulator_t y = (umath_accumulator_t)value.y;
    const umath_accumulator_t z = (umath_accumulator_t)value.z;
    const umath_accumulator_t w = (umath_accumulator_t)value.w;
    const umath_accumulator_t xy = UETD_UMATH_ACCUM_BINARY_MATH(hypot, x, y);
    const umath_accumulator_t zw = UETD_UMATH_ACCUM_BINARY_MATH(hypot, z, w);
    const umath_accumulator_t result = UETD_UMATH_ACCUM_BINARY_MATH(hypot, xy, zw);
    if (norm == NULL) return UMATH_INVALID_ARGUMENT;
    if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) { return UMATH_NUMERIC_FAILURE; }
    *norm = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_vec4_normalize(umath_vec4_t value, umath_vec4_t* unit) {
    umath_scalar_t norm;
    umath_vec4_t result;
    umath_status_t status;
    if (unit == NULL) return UMATH_INVALID_ARGUMENT;
    status = umath_vec4_norm(value, &norm);
    if (status != UMATH_OK) return status;
    if (norm <= 0) return UMATH_INVALID_ARGUMENT;
    result.x = value.x / norm;
    result.y = value.y / norm;
    result.z = value.z / norm;
    result.w = value.w / norm;
    *unit = result;
    return UMATH_OK;
}

static bool vec4_accum(umath_vec4_t value, umath_accumulator_t out[4]) {
    if (!umath_scalar_is_finite(value.x) || !umath_scalar_is_finite(value.y) || !umath_scalar_is_finite(value.z) || !umath_scalar_is_finite(value.w)) return false;
    out[0] = (umath_accumulator_t)value.x; out[1] = (umath_accumulator_t)value.y;
    out[2] = (umath_accumulator_t)value.z; out[3] = (umath_accumulator_t)value.w;
    return true;
}

static umath_status_t vec4_commit(const umath_accumulator_t in[4], umath_vec4_t* result) {
    umath_vec4_t converted = {(umath_scalar_t)in[0], (umath_scalar_t)in[1], (umath_scalar_t)in[2], (umath_scalar_t)in[3]};
    if (!umath_accumulator_is_finite(in[0]) || !umath_accumulator_is_finite(in[1]) || !umath_accumulator_is_finite(in[2]) ||
        !umath_accumulator_is_finite(in[3]) || !umath_scalar_is_finite(converted.x) || !umath_scalar_is_finite(converted.y) ||
        !umath_scalar_is_finite(converted.z) || !umath_scalar_is_finite(converted.w)) return UMATH_NUMERIC_FAILURE;
    *result = converted;
    return UMATH_OK;
}

umath_status_t umath_vec4_mul(umath_vec4_t left, umath_vec4_t right, umath_vec4_t* result) {
    umath_accumulator_t a[4], b[4], c[4]; size_t i;
    if (result == NULL || !vec4_accum(left, a) || !vec4_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < 4; ++i) c[i] = a[i] * b[i];
    return vec4_commit(c, result);
}

umath_status_t umath_vec4_div(umath_vec4_t left, umath_vec4_t right, umath_vec4_t* result) {
    umath_accumulator_t a[4], b[4], c[4]; size_t i;
    if (result == NULL || !vec4_accum(left, a) || !vec4_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < 4; ++i) if (b[i] == 0) return UMATH_SINGULAR;
    for (i = 0; i < 4; ++i) c[i] = a[i] / b[i];
    return vec4_commit(c, result);
}

umath_status_t umath_vec4_distance(umath_vec4_t left, umath_vec4_t right, umath_scalar_t* distance) {
    umath_accumulator_t a[4], b[4], xy, zw, d; umath_scalar_t converted;
    if (distance == NULL || !vec4_accum(left, a) || !vec4_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    xy = UETD_UMATH_ACCUM_BINARY_MATH(hypot, a[0] - b[0], a[1] - b[1]);
    zw = UETD_UMATH_ACCUM_BINARY_MATH(hypot, a[2] - b[2], a[3] - b[3]);
    d = UETD_UMATH_ACCUM_BINARY_MATH(hypot, xy, zw); converted = (umath_scalar_t)d;
    if (!umath_accumulator_is_finite(d) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *distance = converted;
    return UMATH_OK;
}

umath_status_t umath_vec4_lerp(umath_vec4_t start, umath_vec4_t end, umath_scalar_t t, umath_vec4_t* result) {
    umath_accumulator_t a[4], b[4], c[4], weight; size_t i;
    if (result == NULL || !umath_scalar_is_finite(t) || !vec4_accum(start, a) || !vec4_accum(end, b)) return UMATH_INVALID_ARGUMENT;
    weight = (umath_accumulator_t)t;
    for (i = 0; i < 4; ++i) c[i] = (UMATH_ACCUMULATOR_C(1) - weight) * a[i] + weight * b[i];
    return vec4_commit(c, result);
}

umath_status_t umath_vec4_angle(umath_vec4_t left, umath_vec4_t right, umath_scalar_t* radians) {
    umath_accumulator_t a[4], b[4], scale_a, scale_b, norm_a, norm_b, dot; umath_scalar_t converted; size_t i;
    if (radians == NULL || !vec4_accum(left, a) || !vec4_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    scale_a = scale_b = 0;
    for (i = 0; i < 4; ++i) {
        scale_a = UETD_UMATH_ACCUM_BINARY_MATH(fmax, scale_a, UETD_UMATH_ACCUM_UNARY_MATH(fabs, a[i]));
        scale_b = UETD_UMATH_ACCUM_BINARY_MATH(fmax, scale_b, UETD_UMATH_ACCUM_UNARY_MATH(fabs, b[i]));
    }
    if (scale_a == 0 || scale_b == 0) return UMATH_SINGULAR;
    for (i = 0; i < 4; ++i) { a[i] /= scale_a; b[i] /= scale_b; }
    norm_a = UETD_UMATH_ACCUM_BINARY_MATH(hypot, UETD_UMATH_ACCUM_BINARY_MATH(hypot, a[0], a[1]), UETD_UMATH_ACCUM_BINARY_MATH(hypot, a[2], a[3]));
    norm_b = UETD_UMATH_ACCUM_BINARY_MATH(hypot, UETD_UMATH_ACCUM_BINARY_MATH(hypot, b[0], b[1]), UETD_UMATH_ACCUM_BINARY_MATH(hypot, b[2], b[3]));
    dot = 0;
    for (i = 0; i < 4; ++i) dot += (a[i] / norm_a) * (b[i] / norm_b);
    if (dot > 1) dot = 1; else if (dot < -1) dot = -1;
    converted = (umath_scalar_t)UETD_UMATH_ACCUM_UNARY_MATH(acos, dot);
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *radians = converted;
    return UMATH_OK;
}

static umath_status_t vec4_orthogonal(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* output, int mode) {
    umath_accumulator_t v[4], n[4], maximum = 0, denom = 0, dot = 0, factor, p[4]; size_t i;
    if (output == NULL || !vec4_accum(value, v) || !vec4_accum(axis, n)) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < 4; ++i) maximum = UETD_UMATH_ACCUM_BINARY_MATH(fmax, maximum, UETD_UMATH_ACCUM_UNARY_MATH(fabs, n[i]));
    if (maximum == 0) return UMATH_SINGULAR;
    for (i = 0; i < 4; ++i) { n[i] /= maximum; denom += n[i] * n[i]; dot += v[i] * n[i]; }
    factor = dot / denom;
    for (i = 0; i < 4; ++i) p[i] = factor * n[i];
    if (mode == 1) for (i = 0; i < 4; ++i) p[i] = v[i] - p[i];
    else if (mode == 2) for (i = 0; i < 4; ++i) p[i] = v[i] - UMATH_ACCUMULATOR_C(2) * p[i];
    return vec4_commit(p, output);
}

umath_status_t umath_vec4_project(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* projection) { return vec4_orthogonal(value, axis, projection, 0); }
umath_status_t umath_vec4_reject(umath_vec4_t value, umath_vec4_t axis, umath_vec4_t* rejection) { return vec4_orthogonal(value, axis, rejection, 1); }
umath_status_t umath_vec4_reflect(umath_vec4_t value, umath_vec4_t normal, umath_vec4_t* reflected) { return vec4_orthogonal(value, normal, reflected, 2); }
