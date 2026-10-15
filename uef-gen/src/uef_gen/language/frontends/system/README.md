# Multi-target system language

A `.uefs` project composes independently built `.uef` target projects. It owns system topology, shared cross-target contracts, link semantics, and system-level hybrid relationships. Each node keeps its own runtime, components, tasksets, and safe-state authority. A remote endpoint is a message port, never an in-process function call or a NEXUS graph node.

The `.uefs` parser, semantic validator, node-project compiler integration, and deterministic system-plan lowering are implemented. `uef-gen check <file>.uefs` verifies the system contract and every referenced `.uef` project. `uef-gen generate <file>.uefs` currently reports that the firmware emitter for a system plan is unavailable; a valid plan is not a flashable artifact.

The language syntax is owned by `registry.json`; `schema.json` validates that language registry. `.uefs` shares `.uef`'s scalar, unit, shape, enum, record, and no-implicit-conversion rules rather than defining another UETD type catalogue. UETD target/module/type/unit/UMATH facts come through the API-linked UETD registries, not this directory.

The QCW DRSSTC example uses one STM32H745XIH6 controller project, one STM32G474VET6 executor project, two external FPGA endpoints, and `../../../examples/showcases/tesla_coil/tesla_coil_system.uefs`. The H745 M4 owns capture timestamp extension/phase reconstruction; its M7 owns sensor publication, estimation, the ENMPC-table/online-SQP planning hybrid, and the second hybrid with finite-set MPC. The G474 retains timer and local protection authority. The system manifest describes cross-MCU contracts and the logical fibre protocol. The custom-board pin map, transport implementation, plant parameters, policy table, and solver/timing qualification remain project design inputs.

## System compiler behavior

1. Resolve node project paths relative to the `.uefs` source and compile each `.uef` against its own exact UETD target context.
2. Resolve target, board, module, and runtime facts only from UETD registries. Keep project-owned records and model types attached to their node.
3. Verify link endpoint existence/direction, type contracts, rate/freshness/clock constraints, transport readiness, and bounded failure/recovery behavior.
4. Preserve system-hybrid latency and previous-cycle semantics, invalid/stale behavior, and each node's local safety authority.
5. Lower only a wholly valid system into an aggregate plan with per-node inputs, typed links, endpoint views, and source provenance.

The compiler never infers a wire ABI from C/C++ struct layout. Each node remains independently buildable; the system plan is not a remote-call graph and it is not an emitted firmware artifact.
