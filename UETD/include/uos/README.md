# UOS operating-system services

The first-pass portable runtime is implemented. Include [uos.h](uos.h) for
status-bearing OS services and [deferred.h](deferred.h) for owner-context timers
and deferred callbacks.

| Facility | Contract |
|---|---|
| Clock | Bind a status-bearing monotonic provider once at startup, or call `uos_clock_use_platform()` after initializing UCORE time. Both backends use this epoch; FreeRTOS tick wrap cannot reset it. |
| Bare metal | Checked time, finite busy delays and periodic deadlines. Tasking, mutexes, semaphores, queues and event groups explicitly return `UOS_NOT_SUPPORTED`. Use the application loop and deferred work. |
| FreeRTOS | Static tasks, mutexes with ownership checking, counting semaphores, copied queues, wait-any event flags, and explicit ISR operations. |
| Deferred work | Caller-owned slots, finite clock-domain deadlines, bounded callback count, admission-order dispatch among ready items, cancellation tokens, reset generations and reentrancy guards. |
| SafeRTOS | Nonselectable licensed-port outline. No implementation or qualification is claimed. |

This first pass deliberately changes the old bool/void UOS API. Check
`uos_status_t` against `UOS_OK` (zero); do not use the old boolean checks.
Time, queue-count and event-wait results now use output parameters. Queue
creation takes the backing buffer size. Zero-initialize opaque objects with
`UOS_OBJECT_INITIALIZER`, create once and never copy/move them.
FreeRTOS builds use native static control members and `uos_stack_word_t` stacks.
CMake propagates `UOS_FREERTOS_NATIVE_STORAGE=1` and the kernel/config include
paths; apply them to every consumer when compiling manually.

See the [implemented contracts](../../docs/design/UOS_CONTRACTS.md) and
[manual assembly guide](../../docs/UOS_UAPP_FIRST_PASS.md).
