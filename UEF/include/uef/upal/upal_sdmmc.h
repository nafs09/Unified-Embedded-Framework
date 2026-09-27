/// @file include/uef/upal/upal_sdmmc.h
/// @brief Declares the Phase 0 SD/MMC block-transfer boundary.

#ifndef UEF_UPAL_SDMMC_H
#define UEF_UPAL_SDMMC_H

#include <uef/ucore/uef_status.h>
#include <uef/ucore/uef_types.h>
#include <uef/uhal/target.h>
#include <uef/uhal/uhal_gpio.h>
#include <uef/upal/upal_dma.h>

#ifdef __cplusplus
extern "C" {
#endif

#if UHAL_HAS_SDMMC

#define UPAL_SDMMC_BLOCK_BYTES 512U

typedef enum {
  UPAL_SDMMC_BUS_WIDTH_1 = 1,
  UPAL_SDMMC_BUS_WIDTH_4 = 4,
} upal_sdmmc_width_t;

typedef enum {
  UPAL_SDMMC_CARD_NONE = 0,
  UPAL_SDMMC_CARD_SD_V1,
  UPAL_SDMMC_CARD_SD_V2_SC,
  UPAL_SDMMC_CARD_SD_V2_HC,
  UPAL_SDMMC_CARD_MMC,
  UPAL_SDMMC_CARD_EMMC,
} upal_sdmmc_card_type_t;

/// Information read from the card during the identification sequence.
typedef struct {
  upal_sdmmc_card_type_t type;
  uef_u32_t block_count;
  uef_u32_t block_size;
  uef_u32_t speed_hz;
  uef_u8_t cid[16];
} upal_sdmmc_card_info_t;

/// Board-owned pin, interrupt, and DMA wiring for one SDMMC controller instance.
typedef struct {
  void *instance;
  upal_dma_t *dma;
  uef_u32_t dma_request;
  IRQn_Type irqn;
  uhal_gpio_pin_t clk_pin;
  uhal_gpio_pin_t cmd_pin;
  uhal_gpio_pin_t d0_pin;
  uhal_gpio_pin_t d1_pin;
  uhal_gpio_pin_t d2_pin;
  uhal_gpio_pin_t d3_pin;
  uef_u8_t gpio_af;
  uhal_gpio_pin_t detect_pin;
  bool detect_active_low;
} upal_sdmmc_hw_t;

/// Caller-owned runtime state for a single initialized card/controller pair.
typedef struct {
  const upal_sdmmc_hw_t *hw;
  uef_u32_t clock_hz;
  upal_sdmmc_width_t bus_width;
  bool dma_enabled;
  upal_sdmmc_card_info_t card;
  volatile uef_u32_t state;
  upal_dma_callback_t transfer_cb;
  void *transfer_ctx;
} upal_sdmmc_t;

/// Configure the controller and run the card identification sequence.
uef_status_t upal_sdmmc_init(upal_sdmmc_t *sdmmc, const upal_sdmmc_hw_t *hw,
                             uef_u32_t max_clock_hz, upal_sdmmc_width_t bus_width);

/// Return card-detect state when a detect pin is configured; otherwise report the initialized card.
bool upal_sdmmc_card_present(const upal_sdmmc_t *sdmmc);

/// Return card identity/capacity after successful initialization, or NULL while unavailable.
const upal_sdmmc_card_info_t *upal_sdmmc_card_info(const upal_sdmmc_t *sdmmc);

/// Start an asynchronous transfer of whole 512-byte blocks. Buffers must be DMA-safe and aligned.
uef_status_t upal_sdmmc_read_blocks(upal_sdmmc_t *sdmmc, uef_u32_t block_addr, uef_u8_t *buffer,
                                    uef_u32_t block_count, upal_dma_callback_t callback,
                                    void *context);
uef_status_t upal_sdmmc_write_blocks(upal_sdmmc_t *sdmmc, uef_u32_t block_addr,
                                     const uef_u8_t *buffer, uef_u32_t block_count,
                                     upal_dma_callback_t callback, void *context);

/// Blocking transfer variants for initialization sequences and FatFS diskio integration.
uef_status_t upal_sdmmc_read_blocking(upal_sdmmc_t *sdmmc, uef_u32_t block_addr,
                                      uef_u8_t *buffer, uef_u32_t block_count,
                                      uef_u32_t timeout_ms);
uef_status_t upal_sdmmc_write_blocking(upal_sdmmc_t *sdmmc, uef_u32_t block_addr,
                                       const uef_u8_t *buffer, uef_u32_t block_count,
                                       uef_u32_t timeout_ms);

/// Erase an inclusive range of card blocks, subject to the card's erase alignment requirements.
uef_status_t upal_sdmmc_erase(upal_sdmmc_t *sdmmc, uef_u32_t start_block,
                              uef_u32_t end_block, uef_u32_t timeout_ms);

/// Dispatch controller and DMA interrupts from the board's vector table.
void upal_sdmmc_irq_handler(upal_sdmmc_t *sdmmc);
void upal_sdmmc_dma_handler(upal_sdmmc_t *sdmmc);

#endif /* UHAL_HAS_SDMMC */

#ifdef __cplusplus
}
#endif

#endif /* UEF_UPAL_SDMMC_H */
