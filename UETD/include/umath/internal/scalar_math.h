/// @file include/umath/internal/scalar_math.h
/// @brief C11 dispatch to the libm function matching UMATH accumulator type.
#ifndef UETD_UMATH_INTERNAL_SCALAR_MATH_H
#define UETD_UMATH_INTERNAL_SCALAR_MATH_H

#include <math.h>
#include <umath/config.h>

#if defined(UETD_UMATH_TARGET_SCALAR_MATH_HEADER)
#include UETD_UMATH_TARGET_SCALAR_MATH_HEADER
#endif

#ifndef UETD_UMATH_ACCUM_UNARY_MATH
#define UETD_UMATH_ACCUM_UNARY_MATH(function, value) _Generic(+(value), uetd_f32_t: function##f, uetd_f64_t: function)((value))
#endif

#ifndef UETD_UMATH_ACCUM_BINARY_MATH
#define UETD_UMATH_ACCUM_BINARY_MATH(function, left, right)                                                                       \
    _Generic(+(left), uetd_f32_t: function##f, uetd_f64_t: function)((left), (right))
#endif

#ifndef UETD_UMATH_ACCUM_TERNARY_MATH
#define UETD_UMATH_ACCUM_TERNARY_MATH(function, first, second, third)                                                             \
    _Generic(+(first), uetd_f32_t: function##f, uetd_f64_t: function)((first), (second), (third))
#endif

/* A target math header may replace this with an intrinsic that has the same
 * fused, single-rounding semantics. The default selects fmaf or fma by type. */
#ifndef UETD_UMATH_ACCUM_FMA
#define UETD_UMATH_ACCUM_FMA(left, right, addend) UETD_UMATH_ACCUM_TERNARY_MATH(fma, (left), (right), (addend))
#endif

#ifndef UETD_UMATH_SCALAR_UNARY_MATH
#define UETD_UMATH_SCALAR_UNARY_MATH(function, value) _Generic(+(value), uetd_f32_t: function##f, uetd_f64_t: function)((value))
#endif

#ifndef UETD_UMATH_SCALAR_BINARY_MATH
#define UETD_UMATH_SCALAR_BINARY_MATH(function, left, right) \
    _Generic(+(left), uetd_f32_t: function##f, uetd_f64_t: function)((left), (right))
#endif

#endif /* UETD_UMATH_INTERNAL_SCALAR_MATH_H */
