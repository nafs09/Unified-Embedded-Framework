/// @file include/ucore/constants.h
/// @brief Canonical mathematical constants shared across UETD layers.
#ifndef UETD_CORE_CONSTANTS_H
#define UETD_CORE_CONSTANTS_H

/* Keep numeric source values here. Derived constants are expressions so they
 * remain consistent with the governing value and can be narrowed by callers. */
#define UETD_PI 3.14159265358979323846264338327950288
#define UETD_TAU (2.0 * UETD_PI)
#define UETD_HALF_PI (UETD_PI / 2.0)

#define UETD_EULER_E 2.71828182845904523536028747135266250
#define UETD_SQRT2 1.41421356237309504880168872420969808
#define UETD_INV_SQRT2 (UETD_SQRT2 / 2.0)
#define UETD_LN2 0.69314718055994530941723212145817657
#define UETD_LN10 2.30258509299404568401799145468436421

#endif /* UETD_CORE_CONSTANTS_H */
