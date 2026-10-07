/// @file include/uef/ucore/checksum.h
/// @brief Incremental CRC functions with explicit initial register values.
#ifndef UEF_UCORE_CHECKSUM_H
#define UEF_UCORE_CHECKSUM_H

#include <uef/ucore/uef_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// CRC-8/SMBUS, polynomial 0x07, non-reflected, no final XOR.
uef_u8_t uef_crc8(const uef_u8_t *buffer, uef_u32_t length, uef_u8_t initial);
/// CRC-16/CCITT-FALSE, polynomial 0x1021, non-reflected, no final XOR.
uef_u16_t uef_crc16_ccitt(const uef_u8_t *buffer, uef_u32_t length, uef_u16_t initial);
/// CRC-32/IEEE, reflected polynomial 0xEDB88320, no final XOR.
uef_u32_t uef_crc32(const uef_u8_t *buffer, uef_u32_t length, uef_u32_t initial);

#ifdef __cplusplus
}
#endif
#endif /* UEF_UCORE_CHECKSUM_H */