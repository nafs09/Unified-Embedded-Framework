# UAPP state-machine and lifecycle contract

## Deployed FSM recommendation

| Area | Policy |
|---|---|
| Execution ownership | One owner task calls the state-machine API and executes transitions. Transition callbacks run serially; the core does not execute transitions concurrently. |
| Event posting | The embedded queue is for same-owner posting. Other tasks and ISRs should send through a platform-owned synchronized ingress queue/notification that the FSM owner drains. Direct multi-context API use is unsupported unless the application serializes every access. |
| Queue overflow | `uapp_sm_post` returns `UETD_OVERFLOW` and rejects the newest event. Every caller checks the result. Critical events use an application-owned reliable mailbox/sticky latch or independent safety path; hardware emergency action must not depend on this FIFO. |
| Lifecycle status | After init accepts the config, `uapp_sm_get_state` returns the last committed state; `uapp_sm_is_active` says its entry hook completed; `uapp_sm_is_faulted` reports a latched callback failure. Do not query fresh, uninitialized storage. |
| Fault handling | Any failed exit/action/entry latches a fault and clears pending events. The failing callback may have partially changed hardware or external state; reset cannot roll that work back. The application fault path establishes a safe condition before reset. |
| Memory and timing | Storage is fixed per machine and derived from compile-time capacities. Transition search is linear in the configured transition count; callback work is synchronous and must be bounded and non-blocking. |
| Hierarchy | Defer parent/child states, history, orthogonal regions, deferred events, and state-local timers until a target application demonstrates the need and supplies bounded semantics. |

## Scope

UAPP provides a bounded, allocation-free, flat state machine for one owner
task. It is a run-to-completion event dispatcher, not a hierarchical statechart
or a scheduler. Hierarchical/orthogonal regions can be added as a separate
composition layer if a real application needs them. Timeouts are ordinary
events produced by the application's timer or supervisor.

The core uses at most 16 states, 32 transition rows, an 8-event queue, and a
16-byte copied payload per event by default. These capacities can be overridden
at compile time. The transition table has `transition_count` rows; a non-null
hook table has `state_count` rows. Configuration tables and the context are
borrowed, immutable after init, and must outlive the machine; the config/tables
must not overlap the machine storage. Event payloads are copied by
`uapp_sm_make_event` and again
when posted; the queue never retains caller pointers. Capacity macros change
public struct layout and therefore must have the same values in every
translation unit. CMake users should set the `UAPP_SM_*` cache values; the
`UETD::Core` target publishes those values to linked code. Initialize fresh
storage once; use `uapp_sm_reset` for recovery rather than calling init again.

Every machine object must start with `UAPP_SM_INITIALIZER` before its first
`uapp_sm_init` call (for example, `uapp_statemachine_t machine =
UAPP_SM_INITIALIZER;`). Init reads the initialized marker only after this
caller-owned initialization, so it can reject a second init of a live instance
with `UETD_BUSY` without probing indeterminate storage. A failed config
validation leaves fresh storage available for a corrected first init. Once
accepted, init is never used again on that instance; use reset for recovery.

The queue occupies `UAPP_SM_EVENT_QUEUE_CAPACITY * sizeof(uapp_sm_event_t)` bytes
inside each machine. With defaults it carries 144 bytes of event data (8 copies
of id, size, and 16 payload bytes); actual occupied bytes are the formula above
and include any target-specific padding. This excludes the rest of
`uapp_statemachine_t`, borrowed transition/hook tables, and callback stack use.
`sizeof(uapp_statemachine_t)` gives the target-specific full object size. No
dynamic allocation is performed.

## Event and transition behavior

1. `uapp_sm_init` requires caller-initialized fresh storage, rejects a live
   machine with `UETD_BUSY`, validates state and transition bounds before
   changing fresh storage, rejects rows shadowed by an earlier unconditional
   duplicate, sets the initial state, and calls that state's entry hook with a
   null event.
2. `uapp_sm_post` copies the event ID and declared payload bytes to the ring
   queue, clearing unused payload bytes. A full queue returns
   `UETD_OVERFLOW` and rejects the new event; it never overwrites older work.
   Every producer checks the result. For a critical event, the application
   retains a sticky pending bit/cause, uses a separately provisioned reliable
   mailbox with back-pressure or retry, or takes an independent safe action.
   Do not use an unbounded retry loop, and do not make an emergency stop depend
   on this ordinary FIFO.
3. `uapp_sm_process_one` removes at most one event and scans matching rows in
   declaration order. Guards must be side-effect-free. The first passing guard
   wins; an event with no eligible transition is consumed, reported to
   `on_unhandled`, and returns `UETD_NOT_SUPPORTED`.
4. For a selected row, UAPP calls the source state's exit hook, the transition
   action, commits the destination state, and calls its entry hook. Hooks and
   actions receive the event and run synchronously. They may post follow-up
   events; those run only after the current callback sequence finishes. A
   callback that posts must handle its own `uapp_sm_post` result. A same-state
   row still performs exit, action, and entry; persistent guards can therefore
   repeat side effects and entry work on each matching event.
5. Any non-`UETD_OK` exit, action, or entry result latches the machine faulted,
   clears queued events, calls `on_fault` exactly once for that failure, and
   returns the callback's status. The first fault status remains available until
   explicit reset recovery begins.
   The fault callback runs after the fault latch is set and therefore cannot
   post back to this machine; `uapp_sm_post` returns `UETD_NOT_READY` there.
   If exit/action fails, `current` remains the source state and is marked
   inactive. If entry fails, `current` is the destination and is marked
   inactive. UAPP does not roll back partial callback effects: a failing
   callback must clean up its partial work or rely on a complete system-level
   fault handler. `uapp_sm_get_state` reports the last committed state even if
   it is inactive; `uapp_sm_get_fault_status` exposes the latched callback status.
6. Recursive processing and init from callbacks return `UETD_BUSY`. The
   machine and queue are not thread-safe; processing does not synchronize a
   concurrent `uapp_sm_post`. The deployed policy is one owner task for all FSM
   calls; other tasks and ISRs use a platform-owned ingress queue or mailbox
   which the owner drains. That ingress contract must state its producer model:
   an SPSC queue has one fixed producer context and the FSM owner as consumer;
   an MPSC queue must provide producer serialization or an atomic reservation
   protocol. Producers publish a complete event before notifying the owner,
   using release/acquire ordering or the platform queue's equivalent
   synchronization. The ingress layer defines overflow handling, including a
   reliable sticky cause/mailbox or separate safe action for critical events.
   A caller that instead serializes direct FSM calls must account for callbacks
   posting back into the machine and must not hold a non-recursive lock across
   callback dispatch. A dispatch inspects at most
   `transition_count` rows (`UAPP_SM_MAX_TRANSITIONS`, 32 by default and at
   most 255) and invokes at most that many guards, followed by either up to
   three transition callbacks or one unhandled callback; a callback failure
   adds one `on_fault` call. Enqueue/dequeue each copy one fixed-size event.
   Callback execution time remains application-owned: guards must be
   side-effect-free; all
   callbacks must have bounded execution, avoid blocking waits and unbounded
   I/O/loops, and respect the owner task's deadline and stack budget. Callbacks
   must not call init or recursively dispatch; request reset after the callback
   sequence because reset returns `UETD_BUSY` while processing. `on_fault` is
   synchronous too and must remain bounded. The API is not ISR-safe.
7. `uapp_sm_reset` discards pending events, exits the active current state if
   applicable, returns to the initial state, and runs its entry hook. If a
   previous exit/action/entry failed, the state may be partially exited,
   partially acted upon, or partially entered; reset does not retry the failed
   callback or infer/undo its partial effects. Since the machine marks that
   state inactive, reset skips its exit hook; the application fault path must
   establish a safe hardware/system state before requesting reset. A failed
   reset entry leaves the machine fault-latched again. Events posted by a
   successful reset entry hook remain queued for a later processing call.
8. After configuration is accepted, `uapp_sm_get_state` reports the last
   committed state. `uapp_sm_is_active` is true only after that state's entry
   hook has completed; it is false during the exit/action/entry sequence and
   after a callback failure. Pair it with `uapp_sm_is_faulted` to distinguish
   a latched failure from a transition in progress. State/status reads follow
   the same owner-or-synchronization rule as all other API calls.

## Component lifecycle

Lifecycle operations use explicit transitional states:

```text
CREATED -> INITIALISING -> INITIALISED -> STARTING -> RUNNING
                                                RUNNING -> STOPPING -> INITIALISED
any state -----------------------------------------------> FAULTED
```

Init/start/stop callbacks return a status. A failure latches `FAULTED`; a fault
reported from inside a callback is not overwritten when that callback returns.
The first fault code is retained and the fault callback runs once. Components
are single-owner and non-reentrant, matching the state-machine execution model.
Recovery is explicit application policy; this small lifecycle type does not
provide dependency ordering or rollback across multiple components.

## Deferred capability

The current contract intentionally omits hierarchy, orthogonal regions,
history states, deferred-event ownership, and state-local timers. Applications
can model timeouts as ordinary queued events and compose separate machines.
Promote richer semantics only with a concrete consumer and a bounded memory,
timing, and reset contract.

## Relationship to UEF-PL

The `.uef` state-machine declaration is not currently executable firmware.
`uef-gen` parses, validates, and lowers it into backend-neutral input, but the
firmware emitter does not consume that input. UEF-PL now declares one scheduled
owner, explicit state consumers, an after-run activation boundary, one
transition maximum per activation, and status-returning callbacks. Its guards
are snapshot-based and include `any`; this C API instead consumes queued event
IDs and matches exact source states. The adapter remains unimplemented.

The future adapter must map one owner activation to one synthetic tick event,
expand `any` into exact-source rows in source order, pass one immutable snapshot
to guards, and publish an independent latest-value state mailbox to each
subscriber. These are replaceable snapshots, not a FIFO of state changes.
RTOS input routes reach the owner through synchronized UOS ingress; they update
the activation snapshot before the tick is processed. Only the owner posts or
processes events on the UAPP instance. A generated UEF-PL hook must not post
arbitrary events to that instance, because its transition rows are tick-driven
guard evaluations. BareMetal uses its sole cooperative loop and ISR-safe
latches/signals, not unsupported RTOS queues or direct ISR calls. Callback
status must propagate to the UETD fault latch, and critical-input overflow
still requires a reliable mailbox/sticky cause or an independent safe action.
The owner coordinates tasks through explicit routes; it does not control the
RTOS scheduler or synchronously stop another task. Naming the ownership
contract does not make the UEF-PL model executable by itself.

## Test design when the host harness is planned

Use table-driven tests to cover configuration rejection, initial entry, event
copying and FIFO order, queue-full behavior, guard fallthrough and declaration
priority, exit/action/entry order, nested-process rejection, unhandled events,
each callback-failure latch point and corresponding state/active/fault queries,
pending-event clearing, reset after safe-state cleanup, and lifecycle fault
preservation. Keep target timing and ISR integration checks separate from host
semantic tests.
