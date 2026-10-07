/// @file src/uhal/arm_cm/memory.c
/// @brief Source scaffold for the V1.2 public contract in uef/uhal/uhal_memory.h.
///
/// Implementation intent: Describe verified memory regions and configure MPU/backup registers
///   from the selected linker map and MCU reference manual.
///
/// Every public function in the paired header has a linkable definition below.
/// Unimplemented status-returning functions report UEF_NOT_SUPPORTED; value functions
/// return a neutral value until the target behavior is implemented.
#include <uef/uhal/uhal_memory.h>

void uhal_mpu_configure(void) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
}

void uhal_backup_write(
    uef_u8_t index,
    uef_u32_t value
) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    (void)index;
    (void)value;
}

uef_u32_t uhal_backup_read(
    uef_u8_t index
) {
    /* TODO(UEF Cortex-M):
     * Implement this contract for the selected Cortex-M CMSIS device without assuming a
     * particular vendor register map. Keep interrupt and register side effects documented,
     * bounded, and safe for the active target.
     */
    (void)index;
    return 0;
}
