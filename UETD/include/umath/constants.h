/// @file include/umath/constants.h
/// @brief Mathematical constants expressed in the configured UMATH precision.
#ifndef UETD_UMATH_CONSTANTS_H
#define UETD_UMATH_CONSTANTS_H

#include <ucore/constants.h>
#include <umath/config.h>

/* Unqualified UMATH constants use accumulator precision. The explicit scalar
 * forms are available when an API requires the public value type instead. */
#define UMATH_PI (UMATH_ACCUMULATOR_C(UETD_PI))
#define UMATH_TAU (UMATH_ACCUMULATOR_C(UETD_TAU))
#define UMATH_HALF_PI (UMATH_ACCUMULATOR_C(UETD_HALF_PI))
#define UMATH_EULER_E (UMATH_ACCUMULATOR_C(UETD_EULER_E))
#define UMATH_SQRT2 (UMATH_ACCUMULATOR_C(UETD_SQRT2))
#define UMATH_INV_SQRT2 (UMATH_ACCUMULATOR_C(UETD_INV_SQRT2))
#define UMATH_LN2 (UMATH_ACCUMULATOR_C(UETD_LN2))
#define UMATH_LN10 (UMATH_ACCUMULATOR_C(UETD_LN10))

#define UMATH_PI_SCALAR (UMATH_SCALAR_C(UETD_PI))
#define UMATH_TAU_SCALAR (UMATH_SCALAR_C(UETD_TAU))
#define UMATH_HALF_PI_SCALAR (UMATH_SCALAR_C(UETD_HALF_PI))
#define UMATH_EULER_E_SCALAR (UMATH_SCALAR_C(UETD_EULER_E))
#define UMATH_SQRT2_SCALAR (UMATH_SCALAR_C(UETD_SQRT2))
#define UMATH_INV_SQRT2_SCALAR (UMATH_SCALAR_C(UETD_INV_SQRT2))
#define UMATH_LN2_SCALAR (UMATH_SCALAR_C(UETD_LN2))
#define UMATH_LN10_SCALAR (UMATH_SCALAR_C(UETD_LN10))

#endif /* UETD_UMATH_CONSTANTS_H */
