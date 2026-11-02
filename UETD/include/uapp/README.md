# UAPP application services

The portable first-pass services below have executable implementations and
regression tests. [uapp.h](uapp.h) includes their public interfaces.

| Header | Implemented behavior |
|---|---|
| [runtime/lifecycle.h](runtime/lifecycle.h) | Guarded component lifecycle; callback failures and explicitly reported faults remain latched. |
| [runtime/statemachine.h](runtime/statemachine.h) | Bounded copied-event queue, ordered guarded transitions, exit/action/entry callbacks, fault latching and explicit reset. |
| [runtime/supervisor.h](runtime/supervisor.h) | Periodic health checks, persistence thresholds, stale-report detection and recursive-tick protection. |
| [runtime/watchdog_gate.h](runtime/watchdog_gate.h) | Health-lease refresh authorization, startup grace, first-fault latch and recursive-tick rejection. |
| [diagnostics/fault.h](diagnostics/fault.h) | Bounded recoverable-fault queue and platform fatal/reset boundary. |
| [diagnostics/fault_history.h](diagnostics/fault_history.h) | Canonical byte encoding, CRC, bounded architecture context, atomic retained-provider integration and sequence-checked lifecycle. |
| [diagnostics/diagnostics.h](diagnostics/diagnostics.h) | Copied structured records, bounded synchronous maintenance, authorization rechecks and terminal result callbacks. |
| [product_session.h](product_session.h) | Bounded pending commands, copied payloads, expiry and authorization, evidence-based completion and no implicit reconnect replay. |
| [secure_update.h](secure_update.h) | Descriptor/policy admission, ordered staging, durable checkpoints, authenticated resume, image verification, boot-health confirmation and anti-rollback ordering. |

Use zero initializers for state machines, deferred queues, diagnostics, product
sessions and update sessions. These objects have one task/main-loop owner.
Callbacks are synchronous and must bound their own execution time; work budgets
bound invocation counts, not elapsed time.

Provider-backed services require real implementations of their declared
callbacks. A portable update state machine does not supply cryptographic keys,
signature algorithms, flash drivers or a bootloader. Crash history requires
atomic retained storage; product sessions require authenticated protocol
evidence. Missing providers are rejected.

The [manual assembly guide](../../docs/UOS_UAPP_FIRST_PASS.md) includes
template commands, migration notes, validation scope and remaining U* dependencies.
