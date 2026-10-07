/// @file src/ucore/checksum.c
/// @brief Incremental software CRC reference implementations.
#include <uef/ucore/checksum.h>

uef_u8_t uef_crc8(const uef_u8_t *buffer, uef_u32_t length, uef_u8_t crc)
{
    if (buffer == NULL && length != 0u) return crc;
    for (uef_u32_t index = 0u; index < length; ++index) {
        crc ^= buffer[index];
        for (uef_u8_t bit = 0u; bit < 8u; ++bit) {
            crc = (crc & 0x80u) != 0u
                ? (uef_u8_t)((crc << 1u) ^ 0x07u)
                : (uef_u8_t)(crc << 1u);
        }
    }
    return crc;
}

uef_u16_t uef_crc16_ccitt(const uef_u8_t *buffer, uef_u32_t length,
                          uef_u16_t crc)
{
    if (buffer == NULL && length != 0u) return crc;
    for (uef_u32_t index = 0u; index < length; ++index) {
        crc ^= (uef_u16_t)((uef_u16_t)buffer[index] << 8u);
        for (uef_u8_t bit = 0u; bit < 8u; ++bit) {
            crc = (crc & 0x8000u) != 0u
                ? (uef_u16_t)((crc << 1u) ^ 0x1021u)
                : (uef_u16_t)(crc << 1u);
        }
    }
    return crc;
}

uef_u32_t uef_crc32(const uef_u8_t *buffer, uef_u32_t length, uef_u32_t crc)
{
    if (buffer == NULL && length != 0u) return crc;
    for (uef_u32_t index = 0u; index < length; ++index) {
        crc ^= buffer[index];
        for (uef_u8_t bit = 0u; bit < 8u; ++bit) {
            const uef_u32_t mask = (uef_u32_t)-(uef_i32_t)(crc & 1u);
            crc = (crc >> 1u) ^ (0xedb88320u & mask);
        }
    }
    return crc;
}