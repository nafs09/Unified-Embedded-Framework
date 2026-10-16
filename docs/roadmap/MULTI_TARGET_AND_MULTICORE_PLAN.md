# UETD multi-target and multicore support plan

**Status:** architecture plan; implementation is not started.

This plan adds first-class support for UETD systems containing multiple
independently programmed chips, while also making on-chip multicore behavior
explicit. The existing Tesla-coil design is an input requirement and reference
system supplied by the project author. Its H745, G4, and FPGA roles are not a
new example proposed by this plan.

## 1. Scope and terminology

UETD needs two related but distinct models:

| Term | Meaning |
|---|---|
| **System** | Composition of connected firmware targets and declared interfaces. |
| **Node** | One independently built and deployed unit, normally an MCU image. |
| **Core** | An execution domain inside one node, such as H745 M7 or M4. |
| **Link** | A physical or shared-memory path connecting two endpoints. |
| **Application role** | A system responsibility such as coordinator, local safety controller, or data-acquisition node. |
| **Bus role** | A protocol-specific role such as SPI controller/peripheral. It exists only when the selected transport defines one. |

“Master/slave” may be used as an application description when it matches the
system, but UETD must not assume every communication link has one master and
one slave. A CAN network, for example, does not acquire a universal master just
because one application node coordinates system behavior. Keep application
authority separate from bus arbitration and endpoint direction.

An on-chip M7/M4 pair is also not the same as two MCUs communicating over a
wire. Cores may share physical memory and reset/startup resources. Separate
chips communicate through a declared transport and cannot exchange pointers or
assume a common address space. Both can appear in one system description, but
their execution and link contracts remain distinct.

## 2. Design rules

1. **Keep target projects local.** A `.uef` project continues to describe one
   concrete target build. Its components, peripherals, interrupt bindings,
   local state machines, and tasksets belong to that target.
2. **Add composition above target projects.** A system description names
   projects as nodes and declares links between their exported/imported
   endpoints. It must not merge remote components into one local application
   graph.
3. **Declare interfaces once.** A project-specific wire contract belongs in
   the system source (or a project-owned imported interface file). Reusable
   standard protocols and runtime capabilities remain UETD registry facts.
   Both endpoint projects resolve the same contract; they do not maintain
   handwritten copies that can drift.
4. **Make communication bounded and typed.** Messages have fixed maximum sizes,
   explicit encoding and units, direction, version, sequence/freshness rules,
   rates, and timeout behavior. A target must reject a contract or transport
   it cannot implement.
5. **Keep safety local.** Each target owns the safe state for the hardware it
   directly controls. A remote coordinator can request an action; a local
   safety controller validates permissives and independently removes outputs
   on faults or stale/lost commands.
6. **Fail closed.** Missing target, board, startup, link, runtime, or protocol
   capability records block generation for that node or system. A partial
   system build must be reported as partial, not as a complete firmware set.

## 3. Authoring model

Keep the existing `project { target = ... }` form for one target. Add a
system-composition source, provisionally `.uefs`, that references target
projects and binds endpoints to link contracts. The extension and exact grammar
are subject to review during implementation; the important boundary is that a
system file composes projects rather than replacing them.

The maintained example is `uef-gen/examples/showcases/tesla_coil/tesla_coil_system.uefs`. It names
one dual-core H745 controller project, the separate G4 protection/executor
project, and the paired external FPGA endpoints. The H745's ENMPC/SQP selection
and ENMPC-to-FS-MPC fusion are local algorithm hybrids; `.uefs` only composes
the target projects and their wire contracts. The abbreviated shape is:

```text
system QcwDrsstc {
  node controller { project = "tesla_coil_h745_controller.uef"; role = h745_estimation_and_mpc_controller; }
  node executor { project = "tesla_coil_g474_executor.uef"; role = g474_local_executor_and_protection; }
  external fpga_controller { implementation = external_rtl; attached_to = controller.controls_endpoint; }
  external fpga_coil { implementation = external_rtl; attached_to = executor.coil_endpoint; }

  link qcw_fibre_command { from = fpga_controller; to = fpga_coil; protocol = qcw_fibre; }
  link qcw_fibre_telemetry { from = fpga_coil; to = fpga_controller; protocol = qcw_fibre; }
}
```

The link contract is the authority for message IDs and schemas, field widths,
units, byte order, checksum/authentication policy where required, maximum
frame size, update rate, sequence handling, freshness, retry limits, disconnect
detection, and recovery state. Version checks belong to the declared wire
protocol when that protocol defines a version field; the language must not add
a separate contract-ID/hash handshake by default. Concrete
transport selection (for example UART, SPI, or CAN-FD), pin/DMA/IRQ bindings,
and bus roles are supplied by the selected target records and each node's local
project. `configured_target_link` above is a placeholder, not a registered
transport.

The first implementation should model point-to-point links and one coordinator
with one or more service/safety nodes. It should permit later peer and
redundant-link topologies without adding a universal master/slave assumption.
Remote component calls are not implicit: messages and their effects must be
declared at the link boundary.

## 4. On-chip multicore model

Each exact chip target record needs a verified core topology before it can be
generation-ready:

- stable core name/ID, architecture, toolchain profile, and clock limits;
- core-local and shared memory regions, ownership, accessibility, and cache
  attributes;
- reset/boot order, which core starts first, how secondary cores are released,
  and how startup failure is reported;
- peripheral and interrupt ownership/visibility;
- supported runtime mode: independent per-core schedulers (AMP), or SMP only
  where the silicon and selected RTOS port explicitly qualify it;
- image layout and linker/startup inputs for each independently linked image.

`taskset core = ...` is an affinity/ownership declaration within this target.
It is not a mechanism for addressing another MCU. A cross-core channel must
declare its producer, consumer, fixed capacity, overflow/freshness behavior,
memory region, cache maintenance/barrier policy, and notification mechanism.
Do not expose ordinary PRIMASK critical sections or single-core atomics as
cross-core synchronization.

For heterogeneous M7/M4 parts, plan around AMP and explicit per-core images or
another vendor-qualified startup model; do not infer generic FreeRTOS SMP from
the presence of two cores. The H745/H755 target profile must remain unavailable
until its exact board memory map, startup, core release, shared-memory and
cache rules, toolchain, and runtime configuration have been verified.

## 5. Compiler and generator responsibilities

The system compiler should:

1. Resolve every referenced project and exact target/board record.
2. Compile each target project in its own target context.
3. Resolve link endpoints and the single shared contract; verify both endpoint
   implementations agree on the same version and supported message set.
4. Check that the chosen runtime and target implement each declared core,
   transport, memory, and interrupt requirement.
5. Produce an aggregate system manifest plus one build plan and artifact
   provenance record per node (and per core image where required).
6. Report the status of every node, link, and artifact. Do not silently skip a
   node whose compiler, linker, or deployment backend is unavailable.


The generator may reuse UETD's per-target resolver and public project-generation
contracts, but it must not duplicate chip, peripheral, protocol, or runtime
catalogues inside `uef-gen`.

## 6. Implementation sequence

### Phase A — Freeze the first supported topology

- Treat the author's Tesla-coil H745/G4/FPGA system as the acceptance source.
- Assess it as a complete-system capability requirement: protocol layouts,
  board data, plant parameters, timing limits and external FPGA artifacts are
  explicit project/target inputs, while reusable capability contracts remain
  owned by UETD.
- Confirm exact chips/boards, which links connect which endpoints, and the
  electrical/protocol contract from the hardware design before claiming a
  generated target is supported.
- Record unresolved board, transport, timing, and safety facts as explicit
  blockers; do not invent them from the illustrative projects.
- Use the updated two-MCU arrangement: one H745 M4/M7 controller and one G4
  local executor/protection controller. Treat both FPGA endpoints as external
  RTL connected through declared MCU-side protocol contracts; UETD generates
  the MCU codec, not FPGA RTL.

### Phase B — Specify typed target, core, system, and link records

- Extend UETD's canonical target/board records for per-core topology, memory,
  startup, resource ownership, and readiness.
- Define schemas for system nodes, exported/imported endpoints, links, and
  versioned message contracts.
- Decide the reviewed system file extension/grammar and stable diagnostics.
- Keep exact hardware facts in UETD; keep project-specific wire/message
  contracts beside the system that owns them.

### Phase C — Implement the language frontends

- First complete the minimum `.uef` path needed for the selected targets:
  operating-system root, tasksets, core assignment, task/resource references,
  and local safety state machines.
- Add a system parser/validator that resolves multiple `.uef` projects and
  checks links/contracts without lowering remote work into local task graphs.
- Add negative diagnostics for unknown cores, mismatched contracts, missing
  endpoints, unsupported transports, duplicate ownership, invalid bounds, and
  unqualified targets.

### Phase D — Bring up H745 multicore and controller-to-executor contracts

- Qualify M7/M4 startup, per-core image/linker setup, shared-memory regions,
  cache/barrier behavior, inter-core notification, and fault/reset behavior.
- Implement the H745-to-G4 controller/executor transport through the paired
  FPGA endpoints with bounded static buffers, the framing/sequence/CRC rules
  declared by the protocol, freshness tracking, timeout detection, and defined
  recovery.
- Preserve local G4 interlock authority; a stale/lost H745 command must reach
  the G4-defined safe state without waiting on the H745.
- Keep FPGA RTL implementation and electrical interface qualification outside
  UETD generation; require the MCU-side signal, timing, and wire contracts to be
  complete before claiming the FPGA-connected project is qualified.

### Phase E — Build, provenance, deployment, and qualification

- Build each target/core artifact with its own exact toolchain and startup
  configuration; produce one aggregate system report with hashes and link
  compatibility results.
- Bind each runtime to its exact MCU/board, compiler/ABI, RTOS configuration,
  clock tree, startup/vector code, linker map, and interrupt-priority policy.
  UOS ingress and storage guarantees must be qualified for that profile; see
  the [UOS API and target contract](../../UETD/docs/design/UOS_CONTRACTS.md).
- Add deployment sequencing and partial-failure reporting only after every
  target has a safe, verified programming path.
- Verify host-side schema/compiler behavior, then exercise each image and link
  on hardware. Include power-up ordering, missing peer, stale/duplicate frames,
  reset during traffic, secondary-core failure, link recovery, and local safe
  state behavior.
- Mark support available per exact board/runtime/link profile only after this
  evidence exists.

## 7. Acceptance criteria

The first supported release is complete only when:

- the system source resolves to all expected independently built artifacts;
- a wire contract has one source of truth; endpoints implement the same message
  layout and apply version checks only when the declared wire protocol includes
  them;
- core affinity and cross-chip routing are distinct, validated concepts;
- all shared memory/queue/link bounds and overflow or timeout policies are
  explicit and statically checkable;
- a remote link loss or stale command puts the locally controlled hardware into
  its declared safe state;
- one node or core build failure is visible and cannot be reported as a fully
  successful system build;
- exact board-level tests support every generation-ready claim.

## 8. Current implementation status

- `.uef` describes one target build. The `.uefs` frontend parses and validates
  system composition and lowers its links to a backend-neutral plan. The Tesla projects declare shared records,
  protocol field maps, link-specific endpoint facts (including the G4 capture
  clock profile), and explicit endpoint/task-channel bindings. Firmware
  emission from the lowered system plan is not implemented.
- Project parsing, taskset validation/lowering, and compiler entry points are
  implemented. A system manifest cannot yet be emitted as usable firmware.
- H745/H755 are catalogued but not generation-ready. Current Cortex-M UHAL,
  atomics, and runtime configuration do not establish multicore safety.
- UETD therefore has a planning/example-level linked-target design, not
  supported multi-chip master/coordinator firmware or qualified dual-core
  support.

See [PROJECT_LANGUAGE_DESIGN.md](../design/PROJECT_LANGUAGE_DESIGN.md) for the existing
single-target language boundary and [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md)
for the cross-project implementation checklist.
