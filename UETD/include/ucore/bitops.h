/// @file include/ucore/bitops.h
/// @brief Portable integer bit and byte-order helpers.
#ifndef UETD_UCORE_BITOPS_H
#define UETD_UCORE_BITOPS_H

#include <ucore/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Return the next representable power of two; zero means overflow or zero input.
uetd_u32_t uetd_next_pow2_u32(uetd_u32_t value);
/// Floor(log2(value)); zero is returned for value zero.
uetd_u32_t uetd_log2_u32(uetd_u32_t value);
uetd_u16_t uetd_byteswap16(uetd_u16_t value);
uetd_u32_t uetd_byteswap32(uetd_u32_t value);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UCORE_BITOPS_H */
