# UOS implemented contracts

The first-pass result-bearing API is implemented in [uos.h](../../include/uos/uos.h),
[common.c](../../src/uos/common.c), [baremetal.c](../../src/uos/baremetal.c) and
[freertos.c](../../src/uos/freertos.c). The previous bool/void signatures are
replaced; migrate all callers and rebuild all translation units.

## Results and outputs

`UOS_OK` is zero. Errors distinguish timeout, empty, full, busy, invalid input,
not ready, unsupported, overflow, not owner and backend error. Required output
parameters remain unchanged on failure. Optional ISR wake outputs are set false
before validation. Poll failures are EMPTY/FULL/BUSY; failed blocking waits are
TIMEOUT. Never interpret a result as a boolean success flag.

Static create requires zero-initialized, aligned object storage. Create once.
Objects, stacks, item buffers and contexts stay alive at fixed addresses; copying,
moving or recreating live objects is invalid. There is no destruction API.
FreeRTOS control-block sizes and alignments are checked against the selected
configuration at compile time. UOS storage macros are overridable ABI settings
and must match across the whole program.
FreeRTOS objects contain the kernel's native static types, not byte arrays
cast to control blocks. CMake publishes `UOS_FREERTOS_NATIVE_STORAGE=1` and
kernel/config includes to all consumers; a manual compiler build must do the
same. Allocate stacks as aligned `uos_stack_word_t` arrays.

Queue creation checks item size, depth, native integer range, multiplication
and the supplied item-buffer extent. Sends copy one complete item. Pointers
inside an item remain borrowed references: UOS does not deep-copy pointed data.
Receive failures preserve the destination. Count is a snapshot, not a reservation.
Full queues reject the newest item.

## Clock and durations

Call `uos_clock_bind()` once during single-threaded startup. Its read callback
must be thread-safe, provide a monotonic 64-bit microsecond epoch, extend counter
wrap and define sleep behavior. It returns readiness/errors separately from a
valid timestamp of zero. Its context outlives UOS. Bare-metal delay additionally
needs the provider's delay callback. No CPU-cycle counter or integer MHz
frequency is assumed.

Alternatively, `uos_clock_use_platform()` binds UCORE time and delay after the
application has initialized that platform clock. This is an explicit assertion
of platform readiness: the value-only UCORE API cannot probe it. The selected
UCORE implementation still has to satisfy monotonicity and clock accuracy.
Before binding, UOS time returns NOT_READY. Rebinding returns BUSY.

Both OS backends use this same clock. The FreeRTOS backend never exposes raw
wrapping scheduler ticks as absolute time. A provider must saturate or report an
error at its representable limit rather than wrap.

`UOS_US/MS/S` are constant-expression helpers, not checked arithmetic.
Computed values use `uos_duration_from_ms/s`; overflow never becomes FOREVER.
FOREVER is reserved for supported blocking synchronization operations and is
invalid for delays and periods.

For a finite FreeRTOS wait, UOS rounds microseconds up to ticks and adds a phase
guard tick, so a call immediately before a tick cannot expire early. Conversion
uses division/remainder arithmetic to avoid intermediate multiplication overflow.
An out-of-range finite wait returns OVERFLOW, never `portMAX_DELAY`.
FOREVER is supported only with `INCLUDE_vTaskSuspend=1`.
A zero delay yields; a zero synchronization timeout polls.

`uos_task_delay_until(&next, period)` treats next as the upcoming absolute
release. Initialize it to `now + period` for a periodic worker. It waits if the
release is future, skips missed releases, and advances to the first cadence
point strictly after the post-wait time. Every addition/multiplication is
checked; errors preserve next. A wait already performed cannot be undone.
Bare-metal delays reject a stalled/regressing clock instead of looping forever.

## Backend and execution context

| Facility | Bare metal | FreeRTOS |
|---|---|---|
| Clock | Bound common provider | Same bound common provider |
| Delay / periodic release | Bounded-provider busy wait | Kernel delay, finite conversion |
| Yield | No-op success | Task yield |
| Task creation / scheduler | NOT_SUPPORTED | Static task creation; scheduler return is BACKEND_ERROR |
| Mutex / semaphore / queue / event flags | NOT_SUPPORTED | Implemented kernel wrappers |
| Explicit ISR operations | NOT_SUPPORTED | Queue send, semaphore give, event set |

Creation is startup/task-context only. Other FreeRTOS task operations require
a running, unsuspended scheduler. Only named `*_from_isr` operations may be
called from an ISR; the exact interrupt priority and nesting must be legal for
the selected kernel port. Arbitrary ISR misuse cannot be detected portably.
Task bodies must not return. Stack size is expressed in bytes and checked
against native word size, depth type and port alignment.

Mutexes are nonrecursive and priority-inheriting kernel mutexes. Recursive
acquisition returns BUSY; unlocking from a nonowner returns NOT_OWNER.
Semaphores have a fixed maximum; an extra give returns FULL.

Event waits are wait-any, clear-on-success. Results contain only the requested
matching bits; unrelated bits remain set. All masks must be nonzero subsets of
`uos_event_valid_mask()`. FreeRTOS reserves its control bits, which are excluded
according to the configured tick width. ISR event set enqueues a timer-daemon
command; OK means admission, and a full command queue returns FULL.

The adapter requires static allocation, mutexes, counting semaphores, timers,
pended timer functions, task delay, scheduler-state and mutex-holder APIs.
The runtime FreeRTOSConfig template supplies these portable options.
The application-owned `uos_port_config.h` supplies clock/interrupt settings and
a nonreturning assertion path.

`UOS_FREERTOS_YIELD_FROM_ISR(woken)` is the port hook and must return normally
to the wrapper. Define it as `portYIELD_FROM_ISR(woken)` only for ports where
that macro is a normal statement. There is no unsafe default. Ports whose native macro returns from the interrupt
handler must override the hook. The Windows test port accumulates the request
and returns it from its native handler. Do not perform a second independent yield
for the same request.

## FSM ingress and ownership

One task or main loop owns all calls to a particular UAPP machine. Its internal
event queue is not synchronized ingress. External tasks and qualified ISRs
publish complete copied records through a separate UOS queue; only the owner
drains it and posts to the FSM. A bounded drain/process budget prevents producer
bursts from monopolizing the owner.

Bare-metal interrupt ingress requires a qualified atomic signal or bounded
buffer outside the FSM. A coalescing signal does not preserve every event.
Neither backend makes an ordinary FIFO an emergency safety mechanism: critical
causes need a sticky latch/reliable delivery or an independent output-inhibit path.

Latest-state fan-out requires one coherent mailbox per consumer. Multiple
consumers of one queue divide work; they do not receive independent snapshots.
The standalone event-table templates do not claim to implement the future
UEF-PL snapshot/fan-out emitter.

## Deferred work

`deferred.h` works in either backend and uses caller-owned slots. It is
single-owner, with no hidden locks, heap or ISR entry points. Finite release
deadlines have an explicit clock domain and generation. Poll rejects a
regressing or mismatched clock. Ready items run in admission order; future
items do not block ready ones.

Poll snapshots the admission cutoff and executes at most its callback budget.
A callback may submit/cancel; newly admitted work waits for a later poll.
Canceling a running callback returns BUSY. Reset discards pending items and
increments the token generation. Stale tokens fail; token counters never wrap.
Recursive poll/reset is rejected. Callback cost remains the application's
responsibility.

## Evidence and target boundary

Host tests exercise checked time, state-machine integration and actual vendored
FreeRTOS Kernel 11.3.1 behavior through its Windows simulator, using 16-, 32- and
64-bit ticks and simulated interrupts. They cover FIFO copying, full/empty,
timeouts, event masks, deferred-command overflow, static storage and mutex
ownership. A host pass does not establish MCU timing or interrupt behavior.

Each deployment still selects an exact compiler/ABI, kernel port/configuration,
monotonic timer, interrupt priorities, stack sizes, startup/linker configuration,
clock tree and memory policy. SafeRTOS remains an unselected licensed-port
outline. The first pass does not claim that every vendored RTOS port or MCU is
qualified.
