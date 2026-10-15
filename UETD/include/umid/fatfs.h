/// @file include/umid/fatfs.h
/// @brief Declares the optional FatFS diskio binding for a UPAL SDMMC instance.

#ifndef UETD_UMID_FATFS_H
#define UETD_UMID_FATFS_H

#include <ucore/status.h>
#include <ucore/types.h>
#include <upal/sdmmc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if UHAL_HAS_SDMMC

/// Register one initialized SDMMC device as a FatFS logical drive before calling f_mount().
/// `drive_num` is zero-based and must be smaller than FatFS's configured FF_VOLUMES.
uetd_status_t umid_fatfs_register_sdmmc(uetd_u8_t drive_num, upal_sdmmc_t* sdmmc);

#endif /* UHAL_HAS_SDMMC */

#ifdef __cplusplus
}
#endif

#endif /* UETD_UMID_FATFS_H */
