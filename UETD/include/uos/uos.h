/// @file include/uos/uos.h
/// @brief Static OS services with explicit failures and microsecond time.
#ifndef UOS_H
#define UOS_H
#include "ucore/types.h"
#if defined(UOS_FREERTOS_NATIVE_STORAGE)
#include "FreeRTOS.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UOS_OK = 0, UOS_TIMEOUT, UOS_EMPTY, UOS_FULL, UOS_BUSY,
    UOS_INVALID_PARAM, UOS_NOT_READY, UOS_NOT_SUPPORTED, UOS_OVERFLOW,
    UOS_NOT_OWNER, UOS_BACKEND_ERROR
} uos_status_t;
typedef uetd_u64_t uos_time_t;
typedef uetd_u64_t uos_dur_t;
#define UOS_WAIT_FOREVER ((uos_dur_t)UINT64_MAX)
/* Constant-expression helpers only; use checked conversion for computed values. */
#define UOS_US(n) ((uos_dur_t)(n))
#define UOS_MS(n) ((uos_dur_t)(n) * 1000ULL)
#define UOS_S(n) ((uos_dur_t)(n) * 1000000ULL)

/* These ABI settings must match all translation units. Concrete ports assert fit. */
#ifndef UOS_TASK_STORAGE_BYTES
#define UOS_TASK_STORAGE_BYTES 1024u
#endif
#ifndef UOS_MUTEX_STORAGE_BYTES
#define UOS_MUTEX_STORAGE_BYTES 256u
#endif
#ifndef UOS_SEM_STORAGE_BYTES
#define UOS_SEM_STORAGE_BYTES 256u
#endif
#ifndef UOS_QUEUE_STORAGE_BYTES
#define UOS_QUEUE_STORAGE_BYTES 256u
#endif
#ifndef UOS_EVENT_STORAGE_BYTES
#define UOS_EVENT_STORAGE_BYTES 128u
#endif
#ifndef UOS_STORAGE_ALIGN
#define UOS_STORAGE_ALIGN 8u
#endif
#define UOS_OBJECT_INITIALIZER {0}

/* Native members avoid type-punning a declared byte array. CMake publishes
 * UOS_FREERTOS_NATIVE_STORAGE and kernel includes to all consumers. A manual
 * build must set that definition consistently in every translation unit. */
#if defined(UOS_FREERTOS_NATIVE_STORAGE)
#define UOS_NATIVE_MEMBER(type) type native;
#define UOS_STACK_WORD_TYPE StackType_t
#else
#define UOS_NATIVE_MEMBER(type)
#define UOS_STACK_WORD_TYPE uintptr_t
#endif
typedef UOS_STACK_WORD_TYPE uos_stack_word_t;
#undef UOS_STACK_WORD_TYPE
typedef struct { union { UOS_NATIVE_MEMBER(StaticTask_t) UETD_ALIGNAS(UOS_STORAGE_ALIGN) uetd_u8_t reserved[UOS_TASK_STORAGE_BYTES]; } control; void* handle; } uos_task_t;
typedef struct { union { UOS_NATIVE_MEMBER(StaticSemaphore_t) UETD_ALIGNAS(UOS_STORAGE_ALIGN) uetd_u8_t reserved[UOS_MUTEX_STORAGE_BYTES]; } control; void* handle; } uos_mutex_t;
typedef struct { union { UOS_NATIVE_MEMBER(StaticSemaphore_t) UETD_ALIGNAS(UOS_STORAGE_ALIGN) uetd_u8_t reserved[UOS_SEM_STORAGE_BYTES]; } control; void* handle; } uos_sem_t;
typedef struct { union { UOS_NATIVE_MEMBER(StaticQueue_t) UETD_ALIGNAS(UOS_STORAGE_ALIGN) uetd_u8_t reserved[UOS_QUEUE_STORAGE_BYTES]; } control; void* handle; } uos_queue_t;
typedef struct { union { UOS_NATIVE_MEMBER(StaticEventGroup_t) UETD_ALIGNAS(UOS_STORAGE_ALIGN) uetd_u8_t reserved[UOS_EVENT_STORAGE_BYTES]; } control; void* handle; } uos_event_t;
#undef UOS_NATIVE_MEMBER
typedef void (*uos_task_fn_t)(void* ctx);

typedef struct {
    uos_status_t (*read)(void* context, uos_time_t* now_us);
    /* Required only by bare-metal delays. Must return normally with a bounded
     * wait, use the same epoch, and report clock/backend failures. */
    uos_status_t (*delay)(void* context, uos_dur_t duration_us);
    void* context;
} uos_clock_provider_t;
/* Bind once during single-threaded startup. Read must be thread-safe and
 * monotonic across counter wrap/sleep. No CPU-frequency assumption is made.
 * Until bound, time/delay report NOT_READY. Provider context outlives UOS. */
uos_status_t uos_clock_bind(const uos_clock_provider_t* provider);
/* Convenience adapter after the platform UCORE clock has been initialized.
 * This asserts platform readiness; it cannot probe a value-only UCORE clock. */
uos_status_t uos_clock_use_platform(void);

/* Zero-initialize objects. Create once; keep object/stack/buffer at fixed addresses.
 * No delete, copying, or recreation of live objects. Creation is startup/task-only.
 * Only named _from_isr APIs may run in qualified kernel-callable ISRs.
 * Other operations run in task/main context. RTOS waits require a running scheduler.
 * Outputs stay unchanged on error except optional ISR wake outputs, set false first.
 * ISR wrappers perform the port yield themselves; do not yield again at the caller.
 * timeout=0 polls; positive finite waits either complete or time out, never become
 * infinite. FOREVER requires backend support. Failed polls distinguish empty/full/busy.
 */
uos_status_t uos_time_now(uos_time_t* now_us);
uos_status_t uos_duration_from_ms(uetd_u64_t ms, uos_dur_t* duration);
uos_status_t uos_duration_from_s(uetd_u64_t seconds, uos_dur_t* duration);
/* Allocate the stack as an aligned array of uos_stack_word_t; pass sizeof(array). */
uos_status_t uos_task_create_static(uos_task_t* task, const char* name, uos_task_fn_t fn, void* ctx,
                                    uetd_u32_t priority, void* stack, uetd_u32_t stack_bytes);
uos_status_t uos_task_delay(uos_dur_t duration_us);
/* next is the next release, not the previous wake. Skip missed releases while
 * preserving cadence; on overflow leave next unchanged. FOREVER is invalid. */
uos_status_t uos_task_delay_until(uos_time_t* next_us, uos_dur_t period_us);
uos_status_t uos_task_yield(void);
uos_status_t uos_scheduler_start(void);
uos_status_t uos_mutex_create_static(uos_mutex_t* mutex);
uos_status_t uos_mutex_lock(uos_mutex_t* mutex, uos_dur_t timeout_us);
uos_status_t uos_mutex_unlock(uos_mutex_t* mutex);
uos_status_t uos_sem_create_static(uos_sem_t* sem, uetd_u32_t initial, uetd_u32_t maximum);
uos_status_t uos_sem_take(uos_sem_t* sem, uos_dur_t timeout_us);
uos_status_t uos_sem_give(uos_sem_t* sem);
uos_status_t uos_sem_give_from_isr(uos_sem_t* sem, bool* higher_priority_woken);
/* item_buffer_bytes is checked before the backend receives the buffer. */
uos_status_t uos_queue_create_static(uos_queue_t* queue, uetd_u32_t item_size, uetd_u32_t depth,
                                     void* item_buffer, size_t item_buffer_bytes);
uos_status_t uos_queue_send(uos_queue_t* queue, const void* item, uos_dur_t timeout_us);
uos_status_t uos_queue_recv(uos_queue_t* queue, void* item, uos_dur_t timeout_us);
uos_status_t uos_queue_send_from_isr(uos_queue_t* queue, const void* item, bool* higher_priority_woken);
uos_status_t uos_queue_waiting(const uos_queue_t* queue, uetd_u32_t* count);
uos_status_t uos_event_create_static(uos_event_t* event);
/* Wait-any, clear matched bits on success; unrelated flags remain untouched.
 * Masks must be nonzero subsets of uos_event_valid_mask(). */
uetd_u32_t uos_event_valid_mask(void);
uos_status_t uos_event_wait(uos_event_t* event, uetd_u32_t mask, uos_dur_t timeout_us, uetd_u32_t* matched);
uos_status_t uos_event_set(uos_event_t* event, uetd_u32_t flags);
uos_status_t uos_event_clear(uos_event_t* event, uetd_u32_t flags);
uos_status_t uos_event_set_from_isr(uos_event_t* event, uetd_u32_t flags, bool* higher_priority_woken);
#ifdef __cplusplus
}
#endif
#endif
