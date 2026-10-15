/// @file src/uhal/x86/memory.c
/// @brief Host-backed stand-in for persistent backup registers.

#include "uhal/memory.h"

#define UETD_HOST_BACKUP_REGISTER_COUNT 8u
static uetd_u32_t g_backup_registers[UETD_HOST_BACKUP_REGISTER_COUNT];

void uhal_mpu_configure(void) { /* Desktop address spaces are managed by the host operating system. */ }

void uhal_backup_write(uetd_u8_t index, uetd_u32_t value) {
    if (index < UETD_HOST_BACKUP_REGISTER_COUNT) { g_backup_registers[index] = value; }
}

uetd_u32_t uhal_backup_read(uetd_u8_t index) { return index < UETD_HOST_BACKUP_REGISTER_COUNT ? g_backup_registers[index] : 0u; }
