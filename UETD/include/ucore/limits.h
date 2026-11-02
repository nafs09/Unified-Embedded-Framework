/// @file include/ucore/limits.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UETD_LIMITS_H
#define UETD_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"

/// Clamp a floating-point value to the inclusive bounds.
static inline uetd_f32_t uetd_clampf(uetd_f32_t v, uetd_f32_t lo, uetd_f32_t hi) { return v < lo ? lo : v > hi ? hi : v; }

/// Clamp an integer value to the inclusive bounds.
static inline uetd_i32_t uetd_clampi(uetd_i32_t v, uetd_i32_t lo, uetd_i32_t hi) { return v < lo ? lo : v > hi ? hi : v; }

/// Return the absolute value of a floating-point number.
static inline uetd_f32_t uetd_absf(uetd_f32_t x) { return x < 0.f ? -x : x; }

/// Return -1, 0, or 1 according to the sign of the value.
static inline uetd_f32_t uetd_signf(uetd_f32_t x) { return x > 0.f ? 1.f : x < 0.f ? -1.f : 0.f; }

/// Return the smaller of two floating-point values.
static inline uetd_f32_t uetd_minf(uetd_f32_t a, uetd_f32_t b) { return a < b ? a : b; }

/// Return the larger of two floating-point values.
static inline uetd_f32_t uetd_maxf(uetd_f32_t a, uetd_f32_t b) { return a > b ? a : b; }

#ifdef __cplusplus
}
#endif

#endif /* UETD_LIMITS_H */
