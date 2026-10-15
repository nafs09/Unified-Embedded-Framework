/// @file src/umath/geometry/quaternion.c
/// @brief Quaternion operations using scalar-first Hamilton convention.
#include <math.h>
#include <float.h>
#include <umath/constants.h>
#include <umath/geometry/quaternion.h>
#include <umath/internal/scalar_math.h>
#include <umath/scalar.h>

umath_quaternion_t umath_quaternion_identity(void) {
    const umath_quaternion_t identity = {(umath_scalar_t)1, 0, 0, 0};
    return identity;
}

umath_quaternion_t umath_quaternion_conjugate(umath_quaternion_t value) {
    const umath_quaternion_t result = {value.w, -value.x, -value.y, -value.z};
    return result;
}

umath_quaternion_t umath_quaternion_multiply(umath_quaternion_t a, umath_quaternion_t b) {
    const umath_quaternion_t result = {
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z, a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x, a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w};
    return result;
}

umath_status_t umath_quaternion_normalize(umath_quaternion_t value, umath_quaternion_t* unit) {
    const umath_accumulator_t wx = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.w, (umath_accumulator_t)value.x);
    const umath_accumulator_t yz = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)value.y, (umath_accumulator_t)value.z);
    const umath_accumulator_t norm = UETD_UMATH_ACCUM_BINARY_MATH(hypot, wx, yz);
    umath_quaternion_t result;
    if (unit == NULL || !umath_accumulator_is_finite(norm) || norm <= UMATH_ACCUMULATOR_C(0)) { return UMATH_INVALID_ARGUMENT; }
    result.w = (umath_scalar_t)(value.w / norm);
    result.x = (umath_scalar_t)(value.x / norm);
    result.y = (umath_scalar_t)(value.y / norm);
    result.z = (umath_scalar_t)(value.z / norm);
    *unit = result;
    return UMATH_OK;
}

umath_status_t umath_quaternion_rotate(umath_quaternion_t rotation, umath_vec3_t input, umath_vec3_t* output) {
    umath_quaternion_t q, p, rotated;
    umath_vec3_t result;
    umath_status_t status;
    if (output == NULL) return UMATH_INVALID_ARGUMENT;
    status = umath_quaternion_normalize(rotation, &q);
    if (status != UMATH_OK) return status;
    p.w = 0;
    p.x = input.x;
    p.y = input.y;
    p.z = input.z;
    rotated = umath_quaternion_multiply(umath_quaternion_multiply(q, p), umath_quaternion_conjugate(q));
    result.x = rotated.x;
    result.y = rotated.y;
    result.z = rotated.z;
    if (!umath_scalar_is_finite(result.x) || !umath_scalar_is_finite(result.y) || !umath_scalar_is_finite(result.z))
        return UMATH_NUMERIC_FAILURE;
    *output = result;
    return UMATH_OK;
}

umath_status_t umath_quaternion_from_axis_angle(umath_vec3_t axis, umath_scalar_t angle_radians, umath_quaternion_t* rotation) {
    umath_vec3_t unit_axis;
    umath_quaternion_t result;
    umath_status_t status;
    const umath_accumulator_t half = UMATH_ACCUMULATOR_C(0.5) * (umath_accumulator_t)angle_radians;
    umath_accumulator_t sine;
    if (rotation == NULL || !umath_scalar_is_finite(angle_radians)) { return UMATH_INVALID_ARGUMENT; }
    status = umath_vec3_normalize(axis, &unit_axis);
    if (status != UMATH_OK) return status;
    sine = UETD_UMATH_ACCUM_UNARY_MATH(sin, half);
    result.w = (umath_scalar_t)UETD_UMATH_ACCUM_UNARY_MATH(cos, half);
    result.x = (umath_scalar_t)((umath_accumulator_t)unit_axis.x * sine);
    result.y = (umath_scalar_t)((umath_accumulator_t)unit_axis.y * sine);
    result.z = (umath_scalar_t)((umath_accumulator_t)unit_axis.z * sine);
    if (!umath_scalar_is_finite(result.w) || !umath_scalar_is_finite(result.x) || !umath_scalar_is_finite(result.y) ||
        !umath_scalar_is_finite(result.z)) { return UMATH_NUMERIC_FAILURE; }
    *rotation = result;
    return UMATH_OK;
}

umath_status_t umath_quaternion_to_axis_angle(umath_quaternion_t rotation, umath_vec3_t* axis, umath_scalar_t* angle_radians) {
    umath_quaternion_t q;
    umath_vec3_t result_axis;
    umath_status_t status;
    umath_accumulator_t vector_norm, angle;
    if (axis == NULL || angle_radians == NULL) { return UMATH_INVALID_ARGUMENT; }
    status = umath_quaternion_normalize(rotation, &q);
    if (status != UMATH_OK) return status;
    if (q.w < 0) {
        q.w = -q.w;
        q.x = -q.x;
        q.y = -q.y;
        q.z = -q.z;
    }
    vector_norm = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)q.x, (umath_accumulator_t)q.y);
    vector_norm = UETD_UMATH_ACCUM_BINARY_MATH(hypot, vector_norm, (umath_accumulator_t)q.z);
    angle = UMATH_ACCUMULATOR_C(2) * UETD_UMATH_ACCUM_BINARY_MATH(atan2, vector_norm, (umath_accumulator_t)q.w);
    if (vector_norm <= UMATH_ACCUMULATOR_C(1.0e-12)) {
        result_axis.x = 1;
        result_axis.y = 0;
        result_axis.z = 0;
    } else {
        result_axis.x = (umath_scalar_t)(q.x / vector_norm);
        result_axis.y = (umath_scalar_t)(q.y / vector_norm);
        result_axis.z = (umath_scalar_t)(q.z / vector_norm);
    }
    *axis = result_axis;
    *angle_radians = (umath_scalar_t)angle;
    return UMATH_OK;
}

static bool quaternion_finite(umath_quaternion_t q) {
    return umath_scalar_is_finite(q.w) && umath_scalar_is_finite(q.x) && umath_scalar_is_finite(q.y) && umath_scalar_is_finite(q.z);
}

static umath_accumulator_t quaternion_norm_accum(umath_quaternion_t q) {
    const umath_accumulator_t wx = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)q.w, (umath_accumulator_t)q.x);
    const umath_accumulator_t yz = UETD_UMATH_ACCUM_BINARY_MATH(hypot, (umath_accumulator_t)q.y, (umath_accumulator_t)q.z);
    return UETD_UMATH_ACCUM_BINARY_MATH(hypot, wx, yz);
}

static umath_status_t quaternion_commit(umath_accumulator_t w, umath_accumulator_t x, umath_accumulator_t y,
                                        umath_accumulator_t z, umath_quaternion_t* result) {
    umath_quaternion_t converted = {(umath_scalar_t)w, (umath_scalar_t)x, (umath_scalar_t)y, (umath_scalar_t)z};
    if (!umath_accumulator_is_finite(w) || !umath_accumulator_is_finite(x) || !umath_accumulator_is_finite(y) ||
        !umath_accumulator_is_finite(z) || !quaternion_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *result = converted;
    return UMATH_OK;
}

umath_status_t umath_quaternion_dot(umath_quaternion_t left, umath_quaternion_t right, umath_scalar_t* dot) {
    umath_accumulator_t value; umath_scalar_t converted;
    if (dot == NULL || !quaternion_finite(left) || !quaternion_finite(right)) return UMATH_INVALID_ARGUMENT;
    value = (umath_accumulator_t)left.w*right.w + (umath_accumulator_t)left.x*right.x +
            (umath_accumulator_t)left.y*right.y + (umath_accumulator_t)left.z*right.z;
    converted = (umath_scalar_t)value;
    if (!umath_accumulator_is_finite(value) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *dot = converted;
    return UMATH_OK;
}

umath_status_t umath_quaternion_norm(umath_quaternion_t value, umath_scalar_t* norm) {
    umath_accumulator_t result; umath_scalar_t converted;
    if (norm == NULL || !quaternion_finite(value)) return UMATH_INVALID_ARGUMENT;
    result = quaternion_norm_accum(value); converted = (umath_scalar_t)result;
    if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *norm = converted;
    return UMATH_OK;
}

static umath_accumulator_t quaternion_rotation_tolerance(void) {
    return UMATH_ACCUMULATOR_C(64) * ((sizeof(umath_scalar_t) == sizeof(float) || sizeof(umath_accumulator_t) == sizeof(float)) ?
            (umath_accumulator_t)FLT_EPSILON : (umath_accumulator_t)DBL_EPSILON);
}

umath_status_t umath_quaternion_from_matrix(const umath_scalar_t* row_major_3x3, umath_quaternion_t* rotation) {
    umath_accumulator_t m[9], tolerance, det, trace, s; size_t i;
    umath_quaternion_t q;
    if (row_major_3x3 == NULL || rotation == NULL) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < 9; ++i) { if (!umath_scalar_is_finite(row_major_3x3[i])) return UMATH_INVALID_ARGUMENT; m[i] = row_major_3x3[i]; }
    tolerance = quaternion_rotation_tolerance();
    for (i = 0; i < 3; ++i) {
        size_t j;
        for (j = i; j < 3; ++j) {
            const umath_accumulator_t dot = m[i*3]*m[j*3] + m[i*3+1]*m[j*3+1] + m[i*3+2]*m[j*3+2];
            const umath_accumulator_t expected = i == j ? UMATH_ACCUMULATOR_C(1) : UMATH_ACCUMULATOR_C(0);
            if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, dot - expected) > tolerance) return UMATH_INVALID_ARGUMENT;
        }
    }
    det = m[0]*(m[4]*m[8]-m[5]*m[7]) - m[1]*(m[3]*m[8]-m[5]*m[6]) + m[2]*(m[3]*m[7]-m[4]*m[6]);
    if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, det - UMATH_ACCUMULATOR_C(1)) > tolerance) return UMATH_INVALID_ARGUMENT;
    trace = m[0] + m[4] + m[8];
    if (trace > 0) {
        s = 2 * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, trace + 1);
        q.w=(umath_scalar_t)(0.25*s); q.x=(umath_scalar_t)((m[7]-m[5])/s); q.y=(umath_scalar_t)((m[2]-m[6])/s); q.z=(umath_scalar_t)((m[3]-m[1])/s);
    } else if (m[0] > m[4] && m[0] > m[8]) {
        s = 2 * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, 1+m[0]-m[4]-m[8]);
        q.w=(umath_scalar_t)((m[7]-m[5])/s); q.x=(umath_scalar_t)(0.25*s); q.y=(umath_scalar_t)((m[1]+m[3])/s); q.z=(umath_scalar_t)((m[2]+m[6])/s);
    } else if (m[4] > m[8]) {
        s = 2 * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, 1+m[4]-m[0]-m[8]);
        q.w=(umath_scalar_t)((m[2]-m[6])/s); q.x=(umath_scalar_t)((m[1]+m[3])/s); q.y=(umath_scalar_t)(0.25*s); q.z=(umath_scalar_t)((m[5]+m[7])/s);
    } else {
        s = 2 * UETD_UMATH_ACCUM_UNARY_MATH(sqrt, 1+m[8]-m[0]-m[4]);
        q.w=(umath_scalar_t)((m[3]-m[1])/s); q.x=(umath_scalar_t)((m[2]+m[6])/s); q.y=(umath_scalar_t)((m[5]+m[7])/s); q.z=(umath_scalar_t)(0.25*s);
    }
    if (!quaternion_finite(q)) return UMATH_NUMERIC_FAILURE;
    return umath_quaternion_normalize(q, rotation);
}

umath_status_t umath_quaternion_to_matrix(umath_quaternion_t rotation, umath_scalar_t* row_major_3x3) {
    umath_quaternion_t q; umath_accumulator_t w, x, y, z; umath_scalar_t m[9]; size_t i;
    if (row_major_3x3 == NULL || !quaternion_finite(rotation)) return UMATH_INVALID_ARGUMENT;
    if (umath_quaternion_normalize(rotation, &q) != UMATH_OK) return UMATH_INVALID_ARGUMENT;
    w=q.w; x=q.x; y=q.y; z=q.z;
    m[0]=(umath_scalar_t)(1-2*(y*y+z*z)); m[1]=(umath_scalar_t)(2*(x*y-z*w)); m[2]=(umath_scalar_t)(2*(x*z+y*w));
    m[3]=(umath_scalar_t)(2*(x*y+z*w)); m[4]=(umath_scalar_t)(1-2*(x*x+z*z)); m[5]=(umath_scalar_t)(2*(y*z-x*w));
    m[6]=(umath_scalar_t)(2*(x*z-y*w)); m[7]=(umath_scalar_t)(2*(y*z+x*w)); m[8]=(umath_scalar_t)(1-2*(x*x+y*y));
    for (i=0; i<9; ++i) if (!umath_scalar_is_finite(m[i])) return UMATH_NUMERIC_FAILURE;
    for (i=0; i<9; ++i) row_major_3x3[i]=m[i];
    return UMATH_OK;
}

umath_status_t umath_quaternion_slerp(umath_quaternion_t start, umath_quaternion_t end, umath_scalar_t t, umath_quaternion_t* result) {
    umath_quaternion_t a, b, blended; umath_accumulator_t dot, wa, wb, theta, sine, u; umath_status_t status;
    if (result == NULL || !umath_scalar_is_finite(t) || t < 0 || t > 1) return UMATH_INVALID_ARGUMENT;
    status=umath_quaternion_normalize(start,&a); if (status!=UMATH_OK) return status;
    status=umath_quaternion_normalize(end,&b); if (status!=UMATH_OK) return status;
    dot=(umath_accumulator_t)a.w*b.w+(umath_accumulator_t)a.x*b.x+(umath_accumulator_t)a.y*b.y+(umath_accumulator_t)a.z*b.z;
    if (dot<0) { b.w=-b.w; b.x=-b.x; b.y=-b.y; b.z=-b.z; dot=-dot; }
    if (dot>1) dot=1;
    u=(umath_accumulator_t)t;
    if (dot>UMATH_ACCUMULATOR_C(0.9995)) {
        status=quaternion_commit((1-u)*a.w+u*b.w,(1-u)*a.x+u*b.x,(1-u)*a.y+u*b.y,(1-u)*a.z+u*b.z,&blended);
        if (status!=UMATH_OK) return status;
        return umath_quaternion_normalize(blended,result);
    }
    theta=UETD_UMATH_ACCUM_UNARY_MATH(acos,dot); sine=UETD_UMATH_ACCUM_UNARY_MATH(sin,theta);
    if (sine==0) return UMATH_SINGULAR;
    wa=UETD_UMATH_ACCUM_UNARY_MATH(sin,(1-u)*theta)/sine; wb=UETD_UMATH_ACCUM_UNARY_MATH(sin,u*theta)/sine;
    status=quaternion_commit(wa*a.w+wb*b.w,wa*a.x+wb*b.x,wa*a.y+wb*b.y,wa*a.z+wb*b.z,&blended);
    if (status!=UMATH_OK) return status;
    return umath_quaternion_normalize(blended,result);
}

umath_status_t umath_quaternion_exp(umath_quaternion_t value, umath_quaternion_t* result) {
    umath_accumulator_t vector_norm, scale, factor;
    if (result==NULL || !quaternion_finite(value)) return UMATH_INVALID_ARGUMENT;
    vector_norm=UETD_UMATH_ACCUM_BINARY_MATH(hypot,(umath_accumulator_t)value.x,(umath_accumulator_t)value.y);
    vector_norm=UETD_UMATH_ACCUM_BINARY_MATH(hypot,vector_norm,(umath_accumulator_t)value.z);
    scale=UETD_UMATH_ACCUM_UNARY_MATH(exp,(umath_accumulator_t)value.w);
    factor=vector_norm==0 ? UMATH_ACCUMULATOR_C(1) : UETD_UMATH_ACCUM_UNARY_MATH(sin,vector_norm)/vector_norm;
    return quaternion_commit(scale*UETD_UMATH_ACCUM_UNARY_MATH(cos,vector_norm),scale*factor*value.x,
                             scale*factor*value.y,scale*factor*value.z,result);
}

umath_status_t umath_quaternion_log(umath_quaternion_t value, umath_quaternion_t* result) {
    umath_accumulator_t norm, vector_norm, angle, factor, x, y, z;
    if (result==NULL || !quaternion_finite(value)) return UMATH_INVALID_ARGUMENT;
    norm=quaternion_norm_accum(value);
    if (!umath_accumulator_is_finite(norm)) return UMATH_NUMERIC_FAILURE;
    if (norm==0) return UMATH_SINGULAR;
    vector_norm=UETD_UMATH_ACCUM_BINARY_MATH(hypot,(umath_accumulator_t)value.x,(umath_accumulator_t)value.y);
    vector_norm=UETD_UMATH_ACCUM_BINARY_MATH(hypot,vector_norm,(umath_accumulator_t)value.z);
    angle=UETD_UMATH_ACCUM_BINARY_MATH(atan2,vector_norm,(umath_accumulator_t)value.w);
    if (vector_norm==0) {
        x=value.w<0 ? UMATH_PI : UMATH_ACCUMULATOR_C(0);
        y=z=UMATH_ACCUMULATOR_C(0);
    } else {
        factor=angle/vector_norm; x=factor*value.x; y=factor*value.y; z=factor*value.z;
    }
    return quaternion_commit(UETD_UMATH_ACCUM_UNARY_MATH(log,norm),x,y,z,result);
}

