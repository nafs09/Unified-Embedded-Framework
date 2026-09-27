/// @file include/uef/uhal/uhal_memory.h
/// @brief Memory region definitions and MPU configuration.

#ifndef UHAL_MEMORY_H
#define UHAL_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"

typedef enum {
    UHAL_MEM_DTCM,          /* M7: tightly coupled, coherent, fastest */
    UHAL_MEM_ITCM,          /* M7: instruction TCM */
    UHAL_MEM_CCM,           /* F4: core-coupled, not DMA-able */
    UHAL_MEM_AXI_SRAM,      /* H7: cacheable, DMA-able with maintenance */
    UHAL_MEM_DMA_COHERENT,  /* non-cacheable (MPU-marked), always coherent */
    UHAL_MEM_SRAM,          /* general SRAM */
    UHAL_MEM_FLASH,         /* internal flash */
} uhal_mem_region_t;

/* MPU configuration — called by SystemInit after cache enable */
void uhal_mpu_configure(void);

/* RTC backup registers — persist through system reset and low-power modes */
void     uhal_backup_write(uef_u8_t index, uef_u32_t value);
uef_u32_t uhal_backup_read(uef_u8_t index);

/* Conventional backup register indices */
#define UHAL_BACKUP_RESET_CAUSE  0u
#define UHAL_BACKUP_FAULT_PC     1u
#define UHAL_BACKUP_FAULT_LR     2u
#define UHAL_BACKUP_BOOT_FLAGS   3u
#define UHAL_BACKUP_FAULT_CODE   4u

#ifdef __cplusplus
}
#endif

#endif /* UHAL_MEMORY_H */
