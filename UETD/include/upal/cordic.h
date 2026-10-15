/// @file include/upal/cordic.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_CORDIC_H
#define UPAL_CORDIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/constants.h"
#include "ucore/types.h"
#include "upal/dma.h"

#if UHAL_HAS_CORDIC

uetd_status_t upal_cordic_init(void);

/* Synchronous sin/cos — blocks ~6 cycles */
void upal_cordic_sincos_q31(uetd_q31_t angle, uetd_q31_t* s, uetd_q31_t* c);
uetd_q31_t upal_cordic_atan2_q31(uetd_q31_t x, uetd_q31_t y);
uetd_q31_t upal_cordic_modulus_q31(uetd_q31_t x, uetd_q31_t y);
uetd_q31_t upal_cordic_sqrt_q31(uetd_q31_t x);

/* DMA batch — for rotating reference frames on arrays */
uetd_status_t upal_cordic_batch_sincos_q31(const uetd_q31_t* angles, uetd_q31_t* sins, uetd_q31_t* coss, uetd_u32_t count,
                                          upal_dma_callback_t cb, void* ctx);

#else /* Fallback software implementations */
static inline void upal_cordic_sincos_q31(uetd_q31_t a, uetd_q31_t* s, uetd_q31_t* c) {
    float fa = (float)a * (float)(UETD_PI / (float)(1u << 31));
    *s = (uetd_q31_t)((float)sinf(fa) * (float)(1u << 31));
    *c = (uetd_q31_t)((float)cosf(fa) * (float)(1u << 31));
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* UPAL_CORDIC_H */
