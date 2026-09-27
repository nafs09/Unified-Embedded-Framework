/// @file src/uhal/x86/memory.c
/// @brief Host-backed stand-in for persistent backup registers.

#include "uef/uhal/uhal_memory.h"

#define UEF_HOST_BACKUP_REGISTER_COUNT 8u
static uef_u32_t g_backup_registers[UEF_HOST_BACKUP_REGISTER_COUNT];

void uhal_mpu_configure(void) {
    /* Desktop address spaces are managed by the host operating system. */
}

void uhal_backup_write(uef_u8_t index, uef_u32_t value) {
    if (index < UEF_HOST_BACKUP_REGISTER_COUNT) {
        g_backup_registers[index] = value;
    }
}

uef_u32_t uhal_backup_read(uef_u8_t index) {
    return index < UEF_HOST_BACKUP_REGISTER_COUNT
        ? g_backup_registers[index] : 0u;
}
