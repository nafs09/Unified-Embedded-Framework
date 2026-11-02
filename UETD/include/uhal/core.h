/// @file include/uhal/core.h
/// @brief CPU identification, reset, and core utilities.

#ifndef UHAL_CORE_H
#define UHAL_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include "uhal/target.h"

/* Execution-core index, not a silicon serial number or CPU model identifier. */
/// Return the zero-based index of the executing core.
uetd_u32_t uhal_core_id(void);
/// Return the current CPU clock frequency in hertz.
uetd_u32_t uhal_cpu_freq_hz(void); /* current SYSCLK in Hz */
/// Request an immediate system reset.
void uhal_system_reset(void);     /* software reset */

/* Optional CPU counter; zero means unavailable. This is not the portable timebase.
 * Platforms document per-core scope, clock changes and sleep behavior. */
/// Return the low 32 bits of the CPU cycle counter.
uetd_u32_t uhal_cycle_count(void);    /* wraps at 2^32 cycles */
/// Return an extended counter if the platform supports it, otherwise zero.
uetd_u64_t uhal_cycle_count_64(void);
/// Return the number of CPU cycles per microsecond.
uetd_u32_t uhal_cycles_per_us(void);  /* cpu_freq_hz / 1e6 */

/* Memory barriers */
void uhal_data_memory_barrier(void);
void uhal_data_sync_barrier(void);
void uhal_instruction_sync_barrier(void);
#define UHAL_DMB() uhal_data_memory_barrier()
#define UHAL_DSB() uhal_data_sync_barrier()
#define UHAL_ISB() uhal_instruction_sync_barrier()

/* NOP — for timing alignment */
void uhal_nop(void);
#define UHAL_NOP() uhal_nop()

#ifdef __cplusplus
}
#endif

#endif /* UHAL_CORE_H */
