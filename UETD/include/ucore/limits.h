/// @file include/ucore/limits.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UETD_LIMITS_H
#define UETD_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"

static inline uetd_f32_t uetd_clampf(uetd_f32_t v, uetd_f32_t lo, uetd_f32_t hi) { return v < lo ? lo : v > hi ? hi : v; }

static inline uetd_i32_t uetd_clampi(uetd_i32_t v, uetd_i32_t lo, uetd_i32_t hi) { return v < lo ? lo : v > hi ? hi : v; }

static inline uetd_f32_t uetd_absf(uetd_f32_t x) { return x < 0.f ? -x : x; }

static inline uetd_f32_t uetd_signf(uetd_f32_t x) { return x > 0.f ? 1.f : x < 0.f ? -1.f : 0.f; }

static inline uetd_f32_t uetd_minf(uetd_f32_t a, uetd_f32_t b) { return a < b ? a : b; }

static inline uetd_f32_t uetd_maxf(uetd_f32_t a, uetd_f32_t b) { return a > b ? a : b; }

#ifdef __cplusplus
}
#endif

#endif /* UETD_LIMITS_H */
