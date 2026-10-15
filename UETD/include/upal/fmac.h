/// @file include/upal/fmac.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_FMAC_H
#define UPAL_FMAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

#if UHAL_HAS_FMAC

typedef enum { UPAL_FMAC_FIR, UPAL_FMAC_IIR } upal_fmac_func_t;

typedef struct {
    upal_fmac_func_t func;
    const uetd_q15_t* coefficients;
    uetd_u32_t n_coefficients;
    uetd_u32_t input_buf_size;
    uetd_u32_t output_buf_size;
} upal_fmac_cfg_t;

uetd_status_t upal_fmac_init(const upal_fmac_cfg_t* cfg);
uetd_status_t upal_fmac_write(const uetd_q15_t* input, uetd_u32_t count);
uetd_u32_t upal_fmac_read(uetd_q15_t* output, uetd_u32_t max_count);
void upal_fmac_irq_handler(void);

#endif /* UHAL_HAS_FMAC */
#ifdef __cplusplus
}
#endif

#endif /* UPAL_FMAC_H */
