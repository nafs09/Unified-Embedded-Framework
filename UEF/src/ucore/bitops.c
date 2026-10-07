/// @file src/ucore/bitops.c
/// @brief Portable integer bit helpers.
#include <uef/ucore/bitops.h>

uef_u32_t uef_next_pow2_u32(uef_u32_t value)
{
    if (value == 0u || value > 0x80000000u) return 0u;
    --value;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return value + 1u;
}

uef_u32_t uef_log2_u32(uef_u32_t value)
{
    uef_u32_t result = 0u;
    while (value > 1u) {
        value >>= 1u;
        ++result;
    }
    return result;
}

uef_u16_t uef_byteswap16(uef_u16_t value)
{
    return (uef_u16_t)((value << 8u) | (value >> 8u));
}

uef_u32_t uef_byteswap32(uef_u32_t value)
{
    return ((value & 0x000000ffu) << 24u)
         | ((value & 0x0000ff00u) << 8u)
         | ((value & 0x00ff0000u) >> 8u)
         | ((value & 0xff000000u) >> 24u);
}