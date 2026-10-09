/// @file include/uef/ucore/bitops.h
/// @brief Portable integer bit and byte-order helpers.
#ifndef UEF_UCORE_BITOPS_H
#define UEF_UCORE_BITOPS_H

#include <uef/ucore/uef_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Return the next representable power of two; zero means overflow or zero input.
uef_u32_t uef_next_pow2_u32(uef_u32_t value);
/// Floor(log2(value)); zero is returned for value zero.
uef_u32_t uef_log2_u32(uef_u32_t value);
uef_u16_t uef_byteswap16(uef_u16_t value);
uef_u32_t uef_byteswap32(uef_u32_t value);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UCORE_BITOPS_H */
