/// @file include/uef/ucore/uef_math.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UEF_MATH_H
#define UEF_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

/* Basic approximations — used when libm is unavailable or CORDIC preferred */
uef_f32_t uef_fast_sin(uef_f32_t x_rad);
uef_f32_t uef_fast_cos(uef_f32_t x_rad);
uef_f32_t uef_fast_atan2(uef_f32_t y, uef_f32_t x);
uef_f32_t uef_fast_sqrt(uef_f32_t x);
uef_f32_t uef_fast_invsqrt(uef_f32_t x);   /* Quake-style, refined Newton */

/* Integer utilities */
uef_u32_t uef_next_pow2_u32(uef_u32_t v);
uef_u32_t uef_log2_u32(uef_u32_t v);
uef_u16_t uef_byteswap16(uef_u16_t v);
uef_u32_t uef_byteswap32(uef_u32_t v);

/* CRC helpers use the named standard polynomials. The caller supplies the
 * initial register value; functions return the running value without a final
 * XOR so they can be used incrementally. A NULL buffer with non-zero length
 * leaves the supplied initial value unchanged. */
uef_u8_t  uef_crc8(const uef_u8_t* buf, uef_u32_t len, uef_u8_t init);
uef_u16_t uef_crc16_ccitt(const uef_u8_t* buf, uef_u32_t len, uef_u16_t init);
uef_u32_t uef_crc32(const uef_u8_t* buf, uef_u32_t len, uef_u32_t init);

#ifdef __cplusplus
}
#endif

#endif /* UEF_MATH_H */
