/// @file include/uef/upal/upal_cordic.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_CORDIC_H
#define UPAL_CORDIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/upal/upal_dma.h"

#if UHAL_HAS_CORDIC

uef_status_t upal_cordic_init(void);

/* Synchronous sin/cos — blocks ~6 cycles */
void    upal_cordic_sincos_q31(uef_q31_t angle, uef_q31_t* s, uef_q31_t* c);
uef_q31_t upal_cordic_atan2_q31(uef_q31_t x, uef_q31_t y);
uef_q31_t upal_cordic_modulus_q31(uef_q31_t x, uef_q31_t y);
uef_q31_t upal_cordic_sqrt_q31(uef_q31_t x);

/* DMA batch — for rotating reference frames on arrays */
uef_status_t upal_cordic_batch_sincos_q31(const uef_q31_t* angles,
                                            uef_q31_t* sins, uef_q31_t* coss,
                                            uef_u32_t count,
                                            upal_dma_callback_t cb, void* ctx);

#else   /* Fallback software implementations */
static inline void upal_cordic_sincos_q31(uef_q31_t a, uef_q31_t* s, uef_q31_t* c) {
    float fa = (float)a * (float)(3.14159265f / (float)(1u << 31));
    *s = (uef_q31_t)((float)sinf(fa) * (float)(1u << 31));
    *c = (uef_q31_t)((float)cosf(fa) * (float)(1u << 31));
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* UPAL_CORDIC_H */
