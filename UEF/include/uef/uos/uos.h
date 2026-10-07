/// @file include/uef/uos/uos.h
/// @brief Public interface from the UEF Architecture and API Specification V1.2.

#ifndef UOS_H
#define UOS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uef/ucore/uef_types.h"
#include "uef/ucore/uef_status.h"
#include <stdalign.h>

/* Backend-neutral caller storage; target backends verify concrete handle fit. */
#define UOS_TASK_STORAGE_BYTES   512u
#define UOS_MUTEX_STORAGE_BYTES   80u
#define UOS_SEM_STORAGE_BYTES     80u
#define UOS_QUEUE_STORAGE_BYTES  112u
#define UOS_EVENT_STORAGE_BYTES   64u
#define UOS_STORAGE_ALIGN          8u

/* ── Time ──────────────────────────────────────────────────────── */
typedef uef_u64_t uos_time_t;      /* microseconds since scheduler start */
typedef uef_u64_t uos_dur_t;

/* Reserved duration value for APIs that support an unbounded wait. */
#define UOS_WAIT_FOREVER ((uos_dur_t)UINT64_MAX)

#define UOS_US(n)  ((uos_dur_t)(n))
#define UOS_MS(n)  ((uos_dur_t)((n) * 1000ULL))
#define UOS_S(n)   ((uos_dur_t)((n) * 1000000ULL))

uos_time_t uos_time_now(void);

/* ── Task ──────────────────────────────────────────────────────── */
typedef struct { UEF_ALIGNAS(UOS_STORAGE_ALIGN)
    uef_u8_t storage[UOS_TASK_STORAGE_BYTES]; } uos_task_t;

typedef void (*uos_task_fn_t)(void* ctx);

bool uos_task_create_static(uos_task_t* task, const char* name,
                              uos_task_fn_t fn, void* ctx,
                              uef_u32_t priority,
                              void* stack, uef_u32_t stack_bytes);

void uos_task_delay(uos_dur_t duration_us);
void uos_task_delay_until(uos_time_t* next_us, uos_dur_t period_us);
void uos_task_yield(void);

void uos_scheduler_start(void);  /* called once from application init */

/* ── Mutex ─────────────────────────────────────────────────────── */
typedef struct { UEF_ALIGNAS(UOS_STORAGE_ALIGN)
    uef_u8_t storage[UOS_MUTEX_STORAGE_BYTES]; } uos_mutex_t;

bool uos_mutex_create_static(uos_mutex_t* m);
bool uos_mutex_lock(uos_mutex_t* m, uos_dur_t timeout_us);
void uos_mutex_unlock(uos_mutex_t* m);

/* ── Semaphore ─────────────────────────────────────────────────── */
typedef struct { UEF_ALIGNAS(UOS_STORAGE_ALIGN)
    uef_u8_t storage[UOS_SEM_STORAGE_BYTES]; } uos_sem_t;

bool uos_sem_create_static(uos_sem_t* s, uef_u32_t initial, uef_u32_t max);
bool uos_sem_take(uos_sem_t* s, uos_dur_t timeout_us);
void uos_sem_give(uos_sem_t* s);
void uos_sem_give_from_isr(uos_sem_t* s, bool* higher_prio_woken);

/* ── Queue ─────────────────────────────────────────────────────── */
typedef struct { UEF_ALIGNAS(UOS_STORAGE_ALIGN)
    uef_u8_t storage[UOS_QUEUE_STORAGE_BYTES]; } uos_queue_t;

bool uos_queue_create_static(uos_queue_t* q, uef_u32_t item_size,
                               uef_u32_t depth, void* item_buf);
bool uos_queue_send(uos_queue_t* q, const void* item, uos_dur_t timeout_us);
bool uos_queue_recv(uos_queue_t* q, void* item, uos_dur_t timeout_us);
bool uos_queue_send_from_isr(uos_queue_t* q, const void* item,
                               bool* higher_prio_woken);
uef_u32_t uos_queue_waiting(const uos_queue_t* q);

/* ── Event flags ────────────────────────────────────────────────── */
typedef struct { UEF_ALIGNAS(UOS_STORAGE_ALIGN)
    uef_u8_t storage[UOS_EVENT_STORAGE_BYTES]; } uos_event_t;

bool     uos_event_create_static(uos_event_t* e);
uef_u32_t uos_event_wait(uos_event_t* e, uef_u32_t mask,
                           uos_dur_t timeout_us);
void     uos_event_set(uos_event_t* e, uef_u32_t flags);
void     uos_event_clear(uos_event_t* e, uef_u32_t flags);
void     uos_event_set_from_isr(uos_event_t* e, uef_u32_t flags,
                                  bool* higher_prio_woken);

#ifdef __cplusplus
}
#endif

#endif /* UOS_H */
