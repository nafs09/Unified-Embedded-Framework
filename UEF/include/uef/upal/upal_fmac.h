/// @file include/uef/upal/upal_fmac.h
/// @brief Public interface from the UEF Architecture and API Specification V1.1.

#ifndef UPAL_FMAC_H
#define UPAL_FMAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

#if UHAL_HAS_FMAC

typedef enum { UPAL_FMAC_FIR, UPAL_FMAC_IIR } upal_fmac_func_t;

typedef struct {
    upal_fmac_func_t  func;
    const uef_q15_t*  coefficients;
    uef_u32_t         n_coefficients;
    uef_u32_t         input_buf_size;
    uef_u32_t         output_buf_size;
} upal_fmac_cfg_t;

uef_status_t upal_fmac_init(const upal_fmac_cfg_t* cfg);
uef_status_t upal_fmac_write(const uef_q15_t* input, uef_u32_t count);
uef_u32_t    upal_fmac_read(uef_q15_t* output, uef_u32_t max_count);
void         upal_fmac_irq_handler(void);

#endif /* UHAL_HAS_FMAC */
#ifdef __cplusplus
}
#endif

#endif /* UPAL_FMAC_H */
