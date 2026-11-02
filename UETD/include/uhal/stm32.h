/// @brief STM32-specific clock IDs. Not part of the CPU-independent contract.
#ifndef UHAL_PLATFORM_STM32_H
#define UHAL_PLATFORM_STM32_H
#include <uhal/clock.h>
#include <uhal/memory.h>
#define UHAL_CLK_SYSCLK UHAL_CLK_SYSTEM
#define UHAL_CLK_HCLK (UHAL_CLK_PLATFORM_BASE + 0u)
#define UHAL_CLK_APB1 (UHAL_CLK_PLATFORM_BASE + 1u)
#define UHAL_CLK_APB2 (UHAL_CLK_PLATFORM_BASE + 2u)
#define UHAL_CLK_APB3 (UHAL_CLK_PLATFORM_BASE + 3u)
#define UHAL_CLK_AHB (UHAL_CLK_PLATFORM_BASE + 4u)
#define UHAL_CLK_ADC (UHAL_CLK_PLATFORM_BASE + 5u)
#define UHAL_CLK_USB (UHAL_CLK_PLATFORM_BASE + 6u)
#define UHAL_CLK_PLL_Q (UHAL_CLK_PLATFORM_BASE + 7u)
/* Presence, DMA visibility and cache attributes still depend on the exact part. */
#define UHAL_MEM_DTCM (UHAL_MEM_PLATFORM_BASE + 0u)
#define UHAL_MEM_ITCM (UHAL_MEM_PLATFORM_BASE + 1u)
#define UHAL_MEM_CCM (UHAL_MEM_PLATFORM_BASE + 2u)
#define UHAL_MEM_AXI_SRAM (UHAL_MEM_PLATFORM_BASE + 3u)
#endif
