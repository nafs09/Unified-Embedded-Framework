/// @file include/upal/storage/flash.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_FLASH_H
#define UPAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

/* Internal flash write — for parameter persistence and OTA staging */
/// Enable write and erase access to internal flash.
uetd_status_t upal_flash_unlock(void);
/// Disable write and erase access to internal flash.
void upal_flash_lock(void);
/// Erase the flash page containing the supplied address.
uetd_status_t upal_flash_erase_page(uetd_u32_t page_addr, uetd_u32_t timeout_ms);
/// Write a 64-bit double word to internal flash.
uetd_status_t upal_flash_write_dword(uetd_u32_t addr, uetd_u64_t data);
/// Write a byte range to internal flash.
uetd_status_t upal_flash_write(uetd_u32_t addr, const uetd_u8_t* data, uetd_u32_t len, uetd_u32_t timeout_ms);
/// Compare flash bytes at the address with the expected data.
bool upal_flash_verify(uetd_u32_t addr, const uetd_u8_t* expected, uetd_u32_t len);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_FLASH_H */
