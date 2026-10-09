/// @file include/uef/umath/internal/scalar_math.h
/// @brief C11 dispatch to the libm function matching UMATH accumulator type.
#ifndef UEF_UMATH_INTERNAL_SCALAR_MATH_H
#define UEF_UMATH_INTERNAL_SCALAR_MATH_H

#include <math.h>
#include <uef/umath/config.h>

#define UEF_UMATH_ACCUM_UNARY_MATH(function, value) \
    _Generic((value), \
        uef_f32_t: function##f, \
        uef_f64_t: function)((value))

#define UEF_UMATH_ACCUM_BINARY_MATH(function, left, right) \
    _Generic((left), \
        uef_f32_t: function##f, \
        uef_f64_t: function)((left), (right))

#define UEF_UMATH_SCALAR_UNARY_MATH(function, value) \
    _Generic((value), \
        uef_f32_t: function##f, \
        uef_f64_t: function)((value))

#endif /* UEF_UMATH_INTERNAL_SCALAR_MATH_H */
