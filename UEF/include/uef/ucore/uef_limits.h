/// @file include/uef/ucore/uef_limits.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UEF_LIMITS_H
#define UEF_LIMITS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

    static inline uef_f32_t uef_clampf(uef_f32_t v, uef_f32_t lo, uef_f32_t hi) {
        return v < lo ? lo : v > hi ? hi : v;
    }
    static inline uef_i32_t uef_clampi(uef_i32_t v, uef_i32_t lo, uef_i32_t hi) {
        return v < lo ? lo : v > hi ? hi : v;
    }
    static inline uef_f32_t uef_absf(uef_f32_t x) { return x < 0.f ? -x : x; }
    static inline uef_f32_t uef_signf(uef_f32_t x) {
        return x > 0.f ? 1.f : x < 0.f ? -1.f : 0.f;
    }
    static inline uef_f32_t uef_minf(uef_f32_t a, uef_f32_t b) {
        return a < b ? a : b;
    }
    static inline uef_f32_t uef_maxf(uef_f32_t a, uef_f32_t b) {
        return a > b ? a : b;
    }

#ifdef __cplusplus
}
#endif

#endif /* UEF_LIMITS_H */
