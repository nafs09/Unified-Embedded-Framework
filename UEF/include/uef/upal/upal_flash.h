/// @file include/uef/upal/upal_flash.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UPAL_FLASH_H
#define UPAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"

/* Internal flash write — for parameter persistence and OTA staging */
uef_status_t upal_flash_unlock(void);
void         upal_flash_lock(void);
uef_status_t upal_flash_erase_page(uef_u32_t page_addr,
                                     uef_u32_t timeout_ms);
uef_status_t upal_flash_write_dword(uef_u32_t addr, uef_u64_t data);
uef_status_t upal_flash_write(uef_u32_t addr, const uef_u8_t* data,
                                uef_u32_t len, uef_u32_t timeout_ms);
bool         upal_flash_verify(uef_u32_t addr, const uef_u8_t* expected,
                                uef_u32_t len);

#ifdef __cplusplus
}
#endif

#endif /* UPAL_FLASH_H */
