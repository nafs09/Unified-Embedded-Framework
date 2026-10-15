/// @file include/uapp/statemachine.h
/// @brief Bounded, allocation-free run-to-completion application state machine.

#ifndef UAPP_STATEMACHINE_H
#define UAPP_STATEMACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ucore/status.h"
#include "ucore/types.h"
#include <stddef.h>

#ifndef UAPP_SM_MAX_STATES
#define UAPP_SM_MAX_STATES 16u
#endif
#ifndef UAPP_SM_MAX_TRANSITIONS
#define UAPP_SM_MAX_TRANSITIONS 32u
#endif
#ifndef UAPP_SM_EVENT_QUEUE_CAPACITY
#define UAPP_SM_EVENT_QUEUE_CAPACITY 8u
#endif
#ifndef UAPP_SM_EVENT_PAYLOAD_CAPACITY
#define UAPP_SM_EVENT_PAYLOAD_CAPACITY 16u
#endif

/* These shape public structs and must match in every translation unit. */

#if UAPP_SM_MAX_STATES < 1 || UAPP_SM_MAX_STATES > 255
#error "UAPP_SM_MAX_STATES must be in [1, 255]"
#endif
#if UAPP_SM_MAX_TRANSITIONS < 0 || UAPP_SM_MAX_TRANSITIONS > 255
#error "UAPP_SM_MAX_TRANSITIONS must be in [0, 255]"
#endif
#if UAPP_SM_EVENT_QUEUE_CAPACITY < 1 || UAPP_SM_EVENT_QUEUE_CAPACITY > 255
#error "UAPP_SM_EVENT_QUEUE_CAPACITY must be in [1, 255]"
#endif
#if UAPP_SM_EVENT_PAYLOAD_CAPACITY < 1 || UAPP_SM_EVENT_PAYLOAD_CAPACITY > 255
#error "UAPP_SM_EVENT_PAYLOAD_CAPACITY must be in [1, 255]"
#endif

/* Initialize every machine object before its first uapp_sm_init call. */
#define UAPP_SM_INITIALIZER {0}

typedef uetd_u8_t uapp_sm_state_t;

/// Events own a bounded payload copy. Bytes beyond size are zeroed by make_event
/// and post; no caller storage or pointer lifetime is retained by the queue.
typedef struct {
    uetd_u8_t id;
    uetd_u8_t size;
    uetd_u8_t payload[UAPP_SM_EVENT_PAYLOAD_CAPACITY];
} uapp_sm_event_t;

/// Guards must be side-effect-free; the first matching row with a passing guard wins.
/// A row whose destination equals its source still exits and re-enters that state.
typedef bool (*uapp_sm_guard_fn)(const uapp_sm_event_t* event, void* ctx);
typedef uetd_status_t (*uapp_sm_action_fn)(const uapp_sm_event_t* event, void* ctx);
typedef uetd_status_t (*uapp_sm_state_hook_fn)(uapp_sm_state_t state, const uapp_sm_event_t* event, void* ctx);
typedef void (*uapp_sm_unhandled_fn)(uapp_sm_state_t state, const uapp_sm_event_t* event, void* ctx);
typedef void (*uapp_sm_fault_fn)(uapp_sm_state_t state, const uapp_sm_event_t* event,
                                 uetd_status_t status, void* ctx);

typedef struct {
    uapp_sm_state_t from;
    uetd_u8_t event_id;
    uapp_sm_state_t to;
    uapp_sm_action_fn action;
    uapp_sm_guard_fn guard;
} uapp_sm_transition_t;

typedef struct {
    uapp_sm_state_hook_fn on_enter;
    uapp_sm_state_hook_fn on_exit;
} uapp_sm_state_hooks_t;

typedef struct {
    const uapp_sm_transition_t* transitions;
    uetd_u8_t transition_count;
    const uapp_sm_state_hooks_t* state_hooks;
    uetd_u8_t state_count;
    uapp_sm_state_t initial_state;
    uapp_sm_unhandled_fn on_unhandled;
    uapp_sm_fault_fn on_fault;
    void* ctx;
} uapp_sm_config_t;

/// The machine is single-owner and non-thread-safe. Initialize fresh storage
/// with UAPP_SM_INITIALIZER before its first uapp_sm_init call. Init rejects a
/// live instance with UETD_BUSY; use reset for recovery. The recommended
/// deployment has one owner call every API and drains task/ISR ingress through
/// a platform queue.
/// Direct calls from other contexts require caller-provided synchronization.
/// Processing does not synchronize concurrent calls to post.
/// Callbacks run synchronously on the owner. State/action/unhandled callbacks
/// may post while the machine is healthy; on_fault cannot post because the
/// machine is already latched. Callbacks must not call init, process
/// recursively, block, allocate dynamically, or retain event pointers. Guards
/// must be side-effect-free. Treat all fields as private after init.
typedef struct {
    uapp_sm_state_t current;
    uapp_sm_state_t initial;
    const uapp_sm_transition_t* transitions;
    const uapp_sm_state_hooks_t* state_hooks;
    uetd_u8_t transition_count;
    uetd_u8_t state_count;
    uapp_sm_event_t event_queue[UAPP_SM_EVENT_QUEUE_CAPACITY];
    uetd_u8_t queue_head;
    uetd_u8_t queue_tail;
    uetd_u8_t queue_count;
    uapp_sm_unhandled_fn on_unhandled;
    uapp_sm_fault_fn on_fault;
    void* ctx;
    uetd_status_t fault_status;
    bool initialized;
    bool processing;
    bool faulted;
    bool state_active;
} uapp_statemachine_t;

/// Validate and initialize fresh machine storage; the initial state's entry hook runs once.
/// The caller must initialize storage with UAPP_SM_INITIALIZER before the first
/// call. This precondition lets init detect a live instance without reading
/// indeterminate storage. Reinitializing a live instance returns UETD_BUSY.
/// The config and borrowed tables must not overlap machine storage. The
/// transition table has transition_count rows; a non-null hook table has
/// state_count rows. Tables must remain immutable and tables/ctx must outlive
/// the machine. A failed initial entry leaves an initialized, fault-latched
/// machine so the fault hook can act.
uetd_status_t uapp_sm_init(uapp_statemachine_t* sm, const uapp_sm_config_t* config);
/// Copy an event into the bounded queue. Full queues reject the newest event;
/// callers must handle UETD_OVERFLOW. Critical events need an application-owned
/// reliable mailbox/latch or an independent safety path; never ignore overflow.
uetd_status_t uapp_sm_post(uapp_statemachine_t* sm, const uapp_sm_event_t* event);
/// Process at most one queued event. NOT_READY means uninitialized, fault-latched,
/// or empty; NOT_SUPPORTED means the event was consumed with no matching transition.
uetd_status_t uapp_sm_process_one(uapp_statemachine_t* sm);
/// Reset to the initial state, discard queued events, and run its entry hook.
/// Returns BUSY if called from a callback. The caller's fault path must make
/// hardware safe before reset; reset cannot undo failed callback side effects.
uetd_status_t uapp_sm_reset(uapp_statemachine_t* sm);
/// Return the last committed state, including when a callback fault left it inactive.
uetd_status_t uapp_sm_get_state(const uapp_statemachine_t* sm, uapp_sm_state_t* state);
/// After init accepts the config, true only when the current state's entry hook
/// completed; false during transition callbacks and after callback failure.
/// A failed initial entry still leaves a queryable, faulted machine.
bool uapp_sm_is_active(const uapp_statemachine_t* sm);
bool uapp_sm_is_faulted(const uapp_statemachine_t* sm);
/// Return the callback failure that latched the machine, if currently faulted.
uetd_status_t uapp_sm_get_fault_status(const uapp_statemachine_t* sm, uetd_status_t* status);
uetd_status_t uapp_sm_make_event(uapp_sm_event_t* event, uetd_u8_t id,
                                 const void* payload, size_t payload_size);

#ifdef __cplusplus
}
#endif

#endif /* UAPP_STATEMACHINE_H */
