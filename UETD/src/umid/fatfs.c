/// @file src/umid/fatfs.c
/// @brief Optional Phase 1 FatFS diskio adapter over the UPAL SDMMC block interface.

#include <umid/fatfs.h>

#if UHAL_HAS_SDMMC && defined(UETD_ENABLE_FATFS) && UETD_ENABLE_FATFS

#include "diskio.h"
#include "ff.h"

#define UETD_FATFS_TRANSFER_TIMEOUT_MS 5000U

/* FatFS calls these diskio functions by logical drive number. Registration is expected during
 * single-threaded board startup before f_mount(), so the table does not need a runtime lock. */
static upal_sdmmc_t* g_sdmmc_drives[FF_VOLUMES];

static DRESULT fatfs_result(uetd_status_t status) {
    switch (status) {
    case UETD_OK: return RES_OK;
    case UETD_BUSY:
    case UETD_NOT_READY: return RES_NOTRDY;
    case UETD_INVALID_ARG: return RES_PARERR;
    default: return RES_ERROR;
    }
}

static DRESULT fatfs_validate_range(BYTE drive_num, LBA_t first_block, UINT block_count, upal_sdmmc_t** sdmmc_out,
                                    uetd_u32_t* count_out) {
    if (drive_num >= FF_VOLUMES || g_sdmmc_drives[drive_num] == NULL || block_count == 0U) return RES_PARERR;
    upal_sdmmc_t* sdmmc = g_sdmmc_drives[drive_num];
    const upal_sdmmc_card_info_t* card = upal_sdmmc_card_info(sdmmc);
    const uetd_u64_t first = (uetd_u64_t)first_block;
    const uetd_u64_t count = (uetd_u64_t)block_count;
    if (card == NULL || !upal_sdmmc_card_present(sdmmc)) return RES_NOTRDY;
    if (first >= card->block_count || count > card->block_count - first) return RES_PARERR;
    if (first > UINT32_MAX) return RES_PARERR;
    *sdmmc_out = sdmmc;
    *count_out = (uetd_u32_t)count;
    return RES_OK;
}

uetd_status_t umid_fatfs_register_sdmmc(uetd_u8_t drive_num, upal_sdmmc_t* sdmmc) {
    if (sdmmc == NULL || drive_num >= FF_VOLUMES) return UETD_INVALID_ARG;
    if (g_sdmmc_drives[drive_num] != NULL && g_sdmmc_drives[drive_num] != sdmmc) return UETD_CONFLICT;
    // FatFS owns the logical-drive lifecycle; the board initializes the controller/card beforehand.
    g_sdmmc_drives[drive_num] = sdmmc;
    return UETD_OK;
}

DSTATUS disk_initialize(BYTE drive_num) {
    if (drive_num >= FF_VOLUMES || g_sdmmc_drives[drive_num] == NULL) return STA_NOINIT;
    /* TODO(fatfs-disk-initialize): verify that board startup completed the UPAL identification
     * sequence, card info is valid, and the card is present. If this callback owns initialization,
     * retain the board-provided hardware descriptor and call the bounded UPAL init sequence here. */
    return upal_sdmmc_card_present(g_sdmmc_drives[drive_num]) && upal_sdmmc_card_info(g_sdmmc_drives[drive_num]) != NULL
               ? 0
               : STA_NOINIT;
}

DSTATUS disk_status(BYTE drive_num) {
    if (drive_num >= FF_VOLUMES || g_sdmmc_drives[drive_num] == NULL || !upal_sdmmc_card_present(g_sdmmc_drives[drive_num]))
        return STA_NOINIT;
    return 0;
}

DRESULT disk_read(BYTE drive_num, BYTE* buffer, LBA_t first_block, UINT block_count) {
    if (buffer == NULL) return RES_PARERR;
    upal_sdmmc_t* sdmmc = NULL;
    uetd_u32_t transfer_count = 0U;
    DRESULT validation = fatfs_validate_range(drive_num, first_block, block_count, &sdmmc, &transfer_count);
    if (validation != RES_OK) return validation;
    /* TODO(fatfs-disk-read): handle FatFS buffers that do not meet DMA alignment/cache requirements
     * with a bounded aligned bounce buffer. Do not report success after a partial block transfer. */
    return fatfs_result(
        upal_sdmmc_read_blocking(sdmmc, (uetd_u32_t)first_block, buffer, transfer_count, UETD_FATFS_TRANSFER_TIMEOUT_MS));
}

DRESULT disk_write(BYTE drive_num, const BYTE* buffer, LBA_t first_block, UINT block_count) {
    if (buffer == NULL) return RES_PARERR;
    upal_sdmmc_t* sdmmc = NULL;
    uetd_u32_t transfer_count = 0U;
    DRESULT validation = fatfs_validate_range(drive_num, first_block, block_count, &sdmmc, &transfer_count);
    if (validation != RES_OK) return validation;
    /* TODO(fatfs-disk-write): preserve the source buffer until programming completes, handle
     * write-protected cards, and retain timeout/card-removal detail in diagnostics. */
    return fatfs_result(
        upal_sdmmc_write_blocking(sdmmc, (uetd_u32_t)first_block, buffer, transfer_count, UETD_FATFS_TRANSFER_TIMEOUT_MS));
}

DRESULT disk_ioctl(BYTE drive_num, BYTE command, void* buffer) {
    if (drive_num >= FF_VOLUMES || g_sdmmc_drives[drive_num] == NULL) return RES_PARERR;
    upal_sdmmc_t* sdmmc = g_sdmmc_drives[drive_num];
    const upal_sdmmc_card_info_t* card = upal_sdmmc_card_info(sdmmc);
    switch (command) {
    case CTRL_SYNC:
        // FatFS writes use the blocking UPAL path, which must wait until card programming completes.
        return upal_sdmmc_card_present(sdmmc) ? RES_OK : RES_NOTRDY;
    case GET_SECTOR_COUNT:
        if (buffer == NULL || card == NULL) return RES_NOTRDY;
        {
            const LBA_t sector_count = (LBA_t)card->block_count;
            if ((uetd_u64_t)sector_count != card->block_count) return RES_PARERR;
            *(LBA_t*)buffer = sector_count;
        }
        return RES_OK;
    case GET_SECTOR_SIZE:
        if (buffer == NULL || card == NULL) return RES_NOTRDY;
        if (card->block_size > 0xFFFFU) return RES_PARERR;
        *(WORD*)buffer = (WORD)card->block_size;
        return RES_OK;
    case GET_BLOCK_SIZE:
        /* The current card record has no erase-group field; avoid inventing a geometry value. */
        return RES_PARERR;
    case CTRL_TRIM:
        if (buffer == NULL) return RES_PARERR;
        /* TODO(fatfs-trim): confirm the selected FatFS LBA_t width and range convention, validate
         * the inclusive start/end against card capacity, and honor the configured erase timeout. */
        {
            const LBA_t* range = (const LBA_t*)buffer;
            if (range[0] > UINT32_MAX || range[1] > UINT32_MAX || range[0] > range[1] ||
                (card != NULL && range[1] >= card->block_count))
                return RES_PARERR;
            return fatfs_result(upal_sdmmc_erase(sdmmc, (uetd_u32_t)range[0], (uetd_u32_t)range[1], UETD_FATFS_TRANSFER_TIMEOUT_MS));
        }
    default: return RES_PARERR;
    }
}

#elif UHAL_HAS_SDMMC

uetd_status_t umid_fatfs_register_sdmmc(uetd_u8_t drive_num, upal_sdmmc_t* sdmmc) {
    /* FatFS headers/library are optional; this keeps the UPAL-only build linkable. */
    (void)drive_num;
    (void)sdmmc;
    return UETD_NOT_SUPPORTED;
}

#endif /* UHAL_HAS_SDMMC && UETD_ENABLE_FATFS */
