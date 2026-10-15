/// @file include/ucore/checksum.h
/// @brief Incremental CRC functions with explicit initial register values.
#ifndef UETD_UCORE_CHECKSUM_H
#define UETD_UCORE_CHECKSUM_H

#include <ucore/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// CRC-8/SMBUS, polynomial 0x07, non-reflected, no final XOR.
uetd_u8_t uetd_crc8(const uetd_u8_t* buffer, uetd_u32_t length, uetd_u8_t initial);
/// CRC-16/CCITT-FALSE, polynomial 0x1021, non-reflected, no final XOR.
uetd_u16_t uetd_crc16_ccitt(const uetd_u8_t* buffer, uetd_u32_t length, uetd_u16_t initial);
/// CRC-32/IEEE, reflected polynomial 0xEDB88320, no final XOR.
uetd_u32_t uetd_crc32(const uetd_u8_t* buffer, uetd_u32_t length, uetd_u32_t initial);

#ifdef __cplusplus
}
#endif
#endif /* UETD_UCORE_CHECKSUM_H */
