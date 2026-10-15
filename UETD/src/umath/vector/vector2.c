/// @file src/umath/vector/vector2.c
/// @brief Two-dimensional vector operations.
#include <math.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>
#include <umath/vector/vector2.h>

umath_vec2_t umath_vec2_add(umath_vec2_t a, umath_vec2_t b) {
    const umath_vec2_t result = {a.x + b.x, a.y + b.y};
    return result;
}

umath_vec2_t umath_vec2_sub(umath_vec2_t a, umath_vec2_t b) {
    const umath_vec2_t result = {a.x - b.x, a.y - b.y};
    return result;
}

umath_vec2_t umath_vec2_scale(umath_vec2_t value, umath_scalar_t scale) {
    const umath_vec2_t result = {value.x * scale, value.y * scale};
    return result;
}

umath_scalar_t umath_vec2_dot(umath_vec2_t a, umath_vec2_t b) { return a.x * b.x + a.y * b.y; }

umath_scalar_t umath_vec2_cross(umath_vec2_t a, umath_vec2_t b) { return a.x * b.y - a.y * b.x; }

umath_status_t umath_vec2_norm(umath_vec2_t value, umath_scalar_t* norm) {
    const umath_accumulator_t result = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.x,
                                                                   (umath_accumulator_t)value.y);
    if (norm == NULL) return UMATH_INVALID_ARGUMENT;
    if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite((umath_scalar_t)result)) { return UMATH_NUMERIC_FAILURE; }
    *norm = (umath_scalar_t)result;
    return UMATH_OK;
}

umath_status_t umath_vec2_normalize(umath_vec2_t value, umath_vec2_t* unit) {
    umath_scalar_t norm;
    umath_vec2_t result;
    umath_status_t status;
    if (unit == NULL) return UMATH_INVALID_ARGUMENT;
    status = umath_vec2_norm(value, &norm);
    if (status != UMATH_OK) return status;
    if (norm <= 0) return UMATH_INVALID_ARGUMENT;
    result.x = value.x / norm;
    result.y = value.y / norm;
    *unit = result;
    return UMATH_OK;
}

static bool vec2_accum(umath_vec2_t value, umath_accumulator_t out[2]) {
    if (!umath_scalar_is_finite(value.x) || !umath_scalar_is_finite(value.y)) return false;
    out[0] = (umath_accumulator_t)value.x; out[1] = (umath_accumulator_t)value.y;
    return true;
}

static umath_status_t vec2_commit(const umath_accumulator_t in[2], umath_vec2_t* result) {
    umath_vec2_t converted = {(umath_scalar_t)in[0], (umath_scalar_t)in[1]};
    if (!umath_accumulator_is_finite(in[0]) || !umath_accumulator_is_finite(in[1]) ||
        !umath_scalar_is_finite(converted.x) || !umath_scalar_is_finite(converted.y)) return UMATH_NUMERIC_FAILURE;
    *result = converted;
    return UMATH_OK;
}

umath_status_t umath_vec2_mul(umath_vec2_t left, umath_vec2_t right, umath_vec2_t* result) {
    umath_accumulator_t a[2], b[2], c[2];
    if (result == NULL || !vec2_accum(left, a) || !vec2_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    c[0] = a[0] * b[0]; c[1] = a[1] * b[1];
    return vec2_commit(c, result);
}

umath_status_t umath_vec2_div(umath_vec2_t left, umath_vec2_t right, umath_vec2_t* result) {
    umath_accumulator_t a[2], b[2], c[2];
    if (result == NULL || !vec2_accum(left, a) || !vec2_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    if (b[0] == 0 || b[1] == 0) return UMATH_SINGULAR;
    c[0] = a[0] / b[0]; c[1] = a[1] / b[1];
    return vec2_commit(c, result);
}

umath_status_t umath_vec2_distance(umath_vec2_t left, umath_vec2_t right, umath_scalar_t* distance) {
    umath_accumulator_t a[2], b[2];
    umath_scalar_t converted;
    umath_accumulator_t d;
    if (distance == NULL || !vec2_accum(left, a) || !vec2_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    d = UETD_UMATH_ACCUM_BINARY_MATH(hypot, a[0] - b[0], a[1] - b[1]);
    converted = (umath_scalar_t)d;
    if (!umath_accumulator_is_finite(d) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *distance = converted;
    return UMATH_OK;
}

umath_status_t umath_vec2_lerp(umath_vec2_t start, umath_vec2_t end, umath_scalar_t t, umath_vec2_t* result) {
    umath_accumulator_t a[2], b[2], c[2], weight;
    if (result == NULL || !umath_scalar_is_finite(t) || !vec2_accum(start, a) || !vec2_accum(end, b)) return UMATH_INVALID_ARGUMENT;
    weight = (umath_accumulator_t)t;
    c[0] = (UMATH_ACCUMULATOR_C(1) - weight) * a[0] + weight * b[0];
    c[1] = (UMATH_ACCUMULATOR_C(1) - weight) * a[1] + weight * b[1];
    return vec2_commit(c, result);
}

umath_status_t umath_vec2_signed_angle(umath_vec2_t left, umath_vec2_t right, umath_scalar_t* radians) {
    umath_accumulator_t a[2], b[2], scale_a, scale_b, cross, dot;
    umath_scalar_t converted;
    if (radians == NULL || !vec2_accum(left, a) || !vec2_accum(right, b)) return UMATH_INVALID_ARGUMENT;
    scale_a = UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_UNARY_MATH(fabs, a[0]), UETD_UMATH_ACCUM_UNARY_MATH(fabs, a[1]));
    scale_b = UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_UNARY_MATH(fabs, b[0]), UETD_UMATH_ACCUM_UNARY_MATH(fabs, b[1]));
    if (scale_a == 0 || scale_b == 0) return UMATH_SINGULAR;
    a[0] /= scale_a; a[1] /= scale_a; b[0] /= scale_b; b[1] /= scale_b;
    cross = a[0] * b[1] - a[1] * b[0]; dot = a[0] * b[0] + a[1] * b[1];
    converted = (umath_scalar_t)UETD_UMATH_ACCUM_BINARY_MATH(atan2, cross, dot);
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *radians = converted;
    return UMATH_OK;
}

static umath_status_t vec2_orthogonal(umath_vec2_t value, umath_vec2_t axis, umath_vec2_t* output, int mode) {
    umath_accumulator_t v[2], n[2], maximum, denom, factor, p[2];
    if (output == NULL || !vec2_accum(value, v) || !vec2_accum(axis, n)) return UMATH_INVALID_ARGUMENT;
    maximum = UETD_UMATH_ACCUM_BINARY_MATH(fmax, UETD_UMATH_ACCUM_UNARY_MATH(fabs, n[0]), UETD_UMATH_ACCUM_UNARY_MATH(fabs, n[1]));
    if (maximum == 0) return UMATH_SINGULAR;
    n[0] /= maximum; n[1] /= maximum;
    denom = n[0] * n[0] + n[1] * n[1];
    factor = (v[0] * n[0] + v[1] * n[1]) / denom;
    p[0] = factor * n[0]; p[1] = factor * n[1];
    if (mode == 1) { p[0] = v[0] - p[0]; p[1] = v[1] - p[1]; }
    else if (mode == 2) { p[0] = v[0] - UMATH_ACCUMULATOR_C(2) * p[0]; p[1] = v[1] - UMATH_ACCUMULATOR_C(2) * p[1]; }
    return vec2_commit(p, output);
}

umath_status_t umath_vec2_project(umath_vec2_t value, umath_vec2_t axis, umath_vec2_t* projection) { return vec2_orthogonal(value, axis, projection, 0); }
umath_status_t umath_vec2_reject(umath_vec2_t value, umath_vec2_t axis, umath_vec2_t* rejection) { return vec2_orthogonal(value, axis, rejection, 1); }
umath_status_t umath_vec2_reflect(umath_vec2_t value, umath_vec2_t normal, umath_vec2_t* reflected) { return vec2_orthogonal(value, normal, reflected, 2); }
