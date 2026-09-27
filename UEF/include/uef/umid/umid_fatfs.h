/// @file include/uef/umid/umid_fatfs.h
/// @brief Declares the optional FatFS diskio binding for a UPAL SDMMC instance.

#ifndef UEF_UMID_FATFS_H
#define UEF_UMID_FATFS_H

#include <uef/ucore/uef_status.h>
#include <uef/ucore/uef_types.h>
#include <uef/upal/upal_sdmmc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if UHAL_HAS_SDMMC

/// Register one initialized SDMMC device as a FatFS logical drive before calling f_mount().
/// `drive_num` is zero-based and must be smaller than FatFS's configured FF_VOLUMES.
uef_status_t umid_fatfs_register_sdmmc(uef_u8_t drive_num, upal_sdmmc_t *sdmmc);

#endif /* UHAL_HAS_SDMMC */

#ifdef __cplusplus
}
#endif

#endif /* UEF_UMID_FATFS_H */
