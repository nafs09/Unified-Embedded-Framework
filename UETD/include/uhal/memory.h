/// @file include/uhal/memory.h
/// @brief Memory region definitions and MPU configuration.

#ifndef UHAL_MEMORY_H
#define UHAL_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"

/* Region properties and DMA accessibility come from exact memory metadata,
 * not a CPU-family name. Platform-specific IDs start at PLATFORM_BASE. */
typedef uetd_u32_t uhal_mem_region_t;
#define UHAL_MEM_SRAM ((uhal_mem_region_t)0u)
#define UHAL_MEM_FLASH ((uhal_mem_region_t)1u)
#define UHAL_MEM_DMA_COHERENT ((uhal_mem_region_t)2u)
#define UHAL_MEM_PLATFORM_BASE ((uhal_mem_region_t)0x100u)

/* Configure memory attributes before enabling caches; platform startup owns it. */
/// Configure the memory protection unit for the platform's memory regions.
void uhal_mpu_configure(void);

/* Platform retained-storage slots. Reset/sleep/power-loss guarantees and the
 * available slot count are platform capabilities; do not assume an RTC exists. */
/// Write a value to a platform retained-storage slot.
void uhal_backup_write(uetd_u8_t index, uetd_u32_t value);
/// Read a value from a platform retained-storage slot.
uetd_u32_t uhal_backup_read(uetd_u8_t index);

/* Conventional backup register indices */
#define UHAL_BACKUP_RESET_CAUSE 0u
#define UHAL_BACKUP_FAULT_PC 1u
#define UHAL_BACKUP_FAULT_LR 2u
#define UHAL_BACKUP_BOOT_FLAGS 3u
#define UHAL_BACKUP_FAULT_CODE 4u

#ifdef __cplusplus
}
#endif

#endif /* UHAL_MEMORY_H */
