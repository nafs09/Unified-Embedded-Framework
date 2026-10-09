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
    /* TODO(uhal_mpu_configure):
 * 1) Load linker/board region definitions and validate alignment, size, priority, and
 *     *    attributes
 * 2) program regions in documented priority order under the startup privilege contract
 * 3) apply barriers and verify before enabling caches.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
}

void uhal_backup_write(
    uef_u8_t index,
    uef_u32_t value
) {
    /* TODO(uhal_backup_write):
 * 1) Validate index and backup-domain access
 * 2) write one register with target access width/synchronization
 * 3) report unsupported indices without aliasing another slot.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
    (void)index;
    (void)value;
}

uef_u32_t uhal_backup_read(
    uef_u8_t index
) {
    /* TODO(uhal_backup_read):
 * 1) Validate index and ensure backup-domain clock/access as required
 * 2) read one target-width register coherently
 * 3) return the documented neutral result for unsupported indices.
 * Keep target register mappings explicit, bound every hardware wait, and preserve unrelated peripheral state.
 */
    (void)index;
    return 0;
}
