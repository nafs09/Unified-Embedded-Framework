/// @file src/ucore/checksum.c
/// @brief Incremental software CRC reference implementations.
#include <ucore/checksum.h>

uetd_u8_t uetd_crc8(const uetd_u8_t* buffer, uetd_u32_t length, uetd_u8_t crc) {
    if (buffer == NULL && length != 0u) return crc;
    for (uetd_u32_t index = 0u; index < length; ++index) {
        crc ^= buffer[index];
        for (uetd_u8_t bit = 0u; bit < 8u; ++bit) {
            crc = (crc & 0x80u) != 0u ? (uetd_u8_t)((crc << 1u) ^ 0x07u) : (uetd_u8_t)(crc << 1u);
        }
    }
    return crc;
}

uetd_u16_t uetd_crc16_ccitt(const uetd_u8_t* buffer, uetd_u32_t length, uetd_u16_t crc) {
    if (buffer == NULL && length != 0u) return crc;
    for (uetd_u32_t index = 0u; index < length; ++index) {
        crc ^= (uetd_u16_t)((uetd_u16_t)buffer[index] << 8u);
        for (uetd_u8_t bit = 0u; bit < 8u; ++bit) {
            crc = (crc & 0x8000u) != 0u ? (uetd_u16_t)((crc << 1u) ^ 0x1021u) : (uetd_u16_t)(crc << 1u);
        }
    }
    return crc;
}

uetd_u32_t uetd_crc32(const uetd_u8_t* buffer, uetd_u32_t length, uetd_u32_t crc) {
    if (buffer == NULL && length != 0u) return crc;
    for (uetd_u32_t index = 0u; index < length; ++index) {
        crc ^= buffer[index];
        for (uetd_u8_t bit = 0u; bit < 8u; ++bit) {
            const uetd_u32_t mask = (uetd_u32_t) - (uetd_i32_t)(crc & 1u);
            crc = (crc >> 1u) ^ (0xedb88320u & mask);
        }
    }
    return crc;
}