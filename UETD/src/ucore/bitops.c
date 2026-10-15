/// @file src/ucore/bitops.c
/// @brief Portable integer bit helpers.
#include <ucore/bitops.h>

uetd_u32_t uetd_next_pow2_u32(uetd_u32_t value) {
    if (value == 0u || value > 0x80000000u) return 0u;
    --value;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return value + 1u;
}

uetd_u32_t uetd_log2_u32(uetd_u32_t value) {
    uetd_u32_t result = 0u;
    while (value > 1u) {
        value >>= 1u;
        ++result;
    }
    return result;
}

uetd_u16_t uetd_byteswap16(uetd_u16_t value) { return (uetd_u16_t)((value << 8u) | (value >> 8u)); }

uetd_u32_t uetd_byteswap32(uetd_u32_t value) {
    return ((value & 0x000000ffu) << 24u) | ((value & 0x0000ff00u) << 8u) | ((value & 0x00ff0000u) >> 8u) |
           ((value & 0xff000000u) >> 24u);
}