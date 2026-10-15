/// @file include/upal/flash.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UPAL_FLASH_H
#define UPAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"

/* Internal flash write — for parameter persistence and OTA staging */
uetd_status_t upal_flash_unlock(void);
void upal_flash_lock(void);
uetd_status_t upal_flash_erase_page(uetd_u32_t page_addr, uetd_u32_t timeout_ms);
uetd_status_t upal_flash_write_dword(uetd_u32_t addr, uetd_u64_t data);
uetd_status_t upal_flash_write(uetd_u32_t addr, const uetd_u8_t* data, uetd_u32_t len, uetd_u32_t timeout_ms);
bool upal_flash_verify(uetd_u32_t addr, const uetd_u8_t* expected, uetd_u32_t len);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_FLASH_H */
