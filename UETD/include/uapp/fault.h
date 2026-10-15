/// @file include/uapp/fault.h
/// @brief Public interface from the UETD Architecture and API Specification V1.2.

#ifndef UAPP_FAULT_H
#define UAPP_FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/types.h"
#include "uhal/fault.h"

#ifndef UAPP_FAULT_QUEUE_CAPACITY
#define UAPP_FAULT_QUEUE_CAPACITY 8u
#endif
#ifndef UAPP_FAULT_DETAIL_CAPACITY
#define UAPP_FAULT_DETAIL_CAPACITY 64u
#endif

enum {
    UAPP_FAULT_RECORD_DETAIL_TRUNCATED = 1u << 0,
};

/* Recoverable records live in a bounded RAM queue. They are not persistent
 * across reset; durable crash history is a separate future service.
 */
typedef struct {
    uetd_u32_t sequence;
    uetd_u32_t fault_code;
    uetd_u32_t flags;
    uetd_u64_t occurred_at_us;
    char detail[UAPP_FAULT_DETAIL_CAPACITY];
} uapp_fault_record_t;

/* Fault codes — application-specific upper bits; UETD uses lower 16 bits */
#define UAPP_FAULT_STACK_OVERFLOW 0x0001u
#define UAPP_FAULT_SENSOR_TIMEOUT 0x0002u
#define UAPP_FAULT_ACTUATOR_FAULT 0x0003u
#define UAPP_FAULT_COMM_LOSS 0x0004u
#define UAPP_FAULT_OVER_TEMPERATURE 0x0005u
#define UAPP_FAULT_OVER_CURRENT 0x0006u
#define UAPP_FAULT_UNDER_VOLTAGE 0x0007u
#define UAPP_FAULT_OVER_VOLTAGE 0x0008u
/* 0x1000 and above: application-defined */

/* Enqueue a recoverable fault without allocating or blocking. */
void uapp_fault_report(uetd_u32_t fault_code, const char* detail);

/* Task-context consumer API. Reports are copied into the queue, so detail
 * storage may be released when uapp_fault_report returns. A full queue drops
 * the newest report and increments the saturating dropped counter.
 */
bool uapp_fault_try_pop(uapp_fault_record_t* out);
uetd_u32_t uapp_fault_dropped_count(void);
void uapp_fault_clear_recoverable(void);

typedef void (*uapp_fault_recovery_fn)(const uapp_fault_record_t* record, void* context);
void uapp_fault_set_recovery_handler(uapp_fault_recovery_fn handler, void* context);
/* Dispatch at most max_records in task/main context. The handler runs after
 * each record is removed from the queue and must not retain the record pointer.
 */
uetd_u32_t uapp_fault_dispatch(uetd_u32_t max_records);

/* Trigger an unrecoverable fault — stores minimal backup metadata and asks
 * UHAL to reset. The current portable path has no captured PC/LR context and
 * does not directly refresh or start a watchdog.
 */
UETD_NORETURN void uapp_fault_fatal(uetd_u32_t fault_code, const char* detail);

/* Read fault log from last boot (via RTC backup registers) */
bool uapp_fault_log_available(void);
uetd_u32_t uapp_fault_log_code(void);
uetd_u32_t uapp_fault_log_pc(void);
uetd_u32_t uapp_fault_log_lr(void);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_FAULT_H */
