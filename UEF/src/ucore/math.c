/// @file src/ucore/math.c
/// @brief Portable scalar reference implementations for UCORE math helpers.
///
/// Targets can replace these routines with a verified CMSIS-DSP or CORDIC
/// implementation. Keeping this version simple makes host simulation useful
/// as a behavioral reference; it is not a claim of constant-time execution.

#include "uef/ucore/uef_math.h"

#include <math.h>
#include <string.h>

uef_f32_t uef_fast_sin(uef_f32_t x_rad) {
    return sinf(x_rad);
}

uef_f32_t uef_fast_cos(uef_f32_t x_rad) {
    return cosf(x_rad);
}

uef_f32_t uef_fast_atan2(uef_f32_t y, uef_f32_t x) {
    return atan2f(y, x);
}

uef_f32_t uef_fast_sqrt(uef_f32_t x) {
    return sqrtf(x);
}

uef_f32_t uef_fast_invsqrt(uef_f32_t x) {
    if (isnan(x) || x < 0.0f) {
        return NAN;
    }
    if (x == 0.0f) {
        return INFINITY;
    }
    if (isinf(x)) {
        return 0.0f;
    }

    /* The classic bit estimate is followed by two Newton refinements. */
    uef_u32_t bits;
    memcpy(&bits, &x, sizeof(bits));
    bits = 0x5f375a86u - (bits >> 1u);
    uef_f32_t estimate;
    memcpy(&estimate, &bits, sizeof(estimate));

    const uef_f32_t half_x = 0.5f * x;
    estimate *= 1.5f - half_x * estimate * estimate;
    estimate *= 1.5f - half_x * estimate * estimate;
    return estimate;
}

uef_u32_t uef_next_pow2_u32(uef_u32_t value) {
    if (value == 0u) {
        return 0u;
    }
    if (value > 0x80000000u) {
        return 0u; /* The next power of two is not representable. */
    }

    --value;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return value + 1u;
}

uef_u32_t uef_log2_u32(uef_u32_t value) {
    uef_u32_t result = 0u;
    while (value > 1u) {
        value >>= 1u;
        ++result;
    }
    return result;
}

uef_u16_t uef_byteswap16(uef_u16_t value) {
    return (uef_u16_t)((value << 8u) | (value >> 8u));
}

uef_u32_t uef_byteswap32(uef_u32_t value) {
    return ((value & 0x000000ffu) << 24u)
         | ((value & 0x0000ff00u) << 8u)
         | ((value & 0x00ff0000u) >> 8u)
         | ((value & 0xff000000u) >> 24u);
}

uef_u8_t uef_crc8(const uef_u8_t* buffer, uef_u32_t length, uef_u8_t crc) {
    if (buffer == NULL && length != 0u) {
        return crc;
    }

    /* CRC-8/SMBUS polynomial 0x07, non-reflected, no final XOR. */
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

uef_u16_t uef_crc16_ccitt(const uef_u8_t* buffer, uef_u32_t length,
                         uef_u16_t crc) {
    if (buffer == NULL && length != 0u) {
        return crc;
    }

    /* CRC-16/CCITT-FALSE polynomial 0x1021, non-reflected, no final XOR. */
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

uef_u32_t uef_crc32(const uef_u8_t* buffer, uef_u32_t length, uef_u32_t crc) {
    if (buffer == NULL && length != 0u) {
        return crc;
    }

    /* Reflected IEEE polynomial 0xEDB88320; init/final XOR belong to caller. */
    for (uef_u32_t index = 0u; index < length; ++index) {
        crc ^= buffer[index];
        for (uef_u8_t bit = 0u; bit < 8u; ++bit) {
            const uef_u32_t mask = (uef_u32_t)-(uef_i32_t)(crc & 1u);
            crc = (crc >> 1u) ^ (0xedb88320u & mask);
        }
    }
    return crc;
}
