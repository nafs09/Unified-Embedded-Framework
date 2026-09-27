/// @file include/uef/upal/upal_dma.h
/// @brief DMA is a first-class resource allocated by this abstraction, not hidden in peripheral drivers.

#ifndef UPAL_DMA_H
#define UPAL_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include "uef/uhal/uhal_cache.h"
#include <uef/uhal/target.h>

typedef enum {
    UPAL_DMA_EVENT_COMPLETE,
    UPAL_DMA_EVENT_HALF_COMPLETE,   /* for double-buffer / ping-pong */
    UPAL_DMA_EVENT_ERROR_TRANSFER,
    UPAL_DMA_EVENT_ERROR_FIFO,
} upal_dma_event_t;

typedef void (*upal_dma_callback_t)(upal_dma_event_t event, void* ctx);

typedef enum {
    UPAL_DMA_DIR_MEM_TO_PERIPH,
    UPAL_DMA_DIR_PERIPH_TO_MEM,
    UPAL_DMA_DIR_MEM_TO_MEM,
} upal_dma_dir_t;

typedef enum {
    UPAL_DMA_WIDTH_BYTE  = 0,
    UPAL_DMA_WIDTH_HALF  = 1,
    UPAL_DMA_WIDTH_WORD  = 2,
} upal_dma_width_t;

/* Static hardware descriptor — const, in flash */
typedef struct {
    void*     controller;       /* DMA1, DMA2 base */
    uef_u8_t  channel;          /* channel / stream number */
    uef_u8_t  request;          /* DMAMUX request on G4/H7 */
    IRQn_Type irqn;
    uef_u8_t  irq_priority;
} upal_dma_hw_t;

/* Transfer descriptor */
typedef struct {
    upal_dma_dir_t    direction;
    upal_dma_width_t  data_width;
    bool              circular;          /* ADC continuous, UART-RX ring */
    bool              periph_increment;  /* almost always false */
    bool              mem_increment;     /* almost always true */
    bool              double_buffer;
    void*             buf0;
    void*             buf1;              /* double_buffer only */
    uef_u32_t         length;            /* in data_width units */
    uef_u32_t         periph_addr;       /* peripheral data register */
} upal_dma_xfer_t;

/* Runtime state */
typedef struct {
    const upal_dma_hw_t*  hw;
    upal_dma_callback_t   callback;
    void*                 callback_ctx;
    volatile uef_u32_t    state;
} upal_dma_t;

uef_status_t upal_dma_init(upal_dma_t* dma, const upal_dma_hw_t* hw,
                             upal_dma_callback_t cb, void* cb_ctx);

/* Start a transfer. On M7: automatically performs cache maintenance
 * for cacheable buffers (clean before MEM→PERIPH, invalidate after
 * PERIPH→MEM). Buffers in UHAL_ATTR_DMA_BUF: no maintenance needed. */
uef_status_t upal_dma_start(upal_dma_t* dma, const upal_dma_xfer_t* xfer);

uef_status_t upal_dma_abort(upal_dma_t* dma);

/* Blocking wait — init sequences only; never in ISR or control loop */
uef_status_t upal_dma_wait(upal_dma_t* dma, uef_u32_t timeout_ms);

uef_u32_t upal_dma_remaining(const upal_dma_t* dma);

/* Call from the DMA stream/channel ISR */
void upal_dma_irq_handler(upal_dma_t* dma);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_DMA_H */
