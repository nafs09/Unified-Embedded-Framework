# Implementation plan

Ordered cross-project checklist. The TODO files hold detailed requirements,
constraints, and implementation guidance.

## 1. Audit and prioritize cross-layer work

- [ ] Complete the source-equivalence and catalogue-readiness review before scaffolding full acceptance projects. Classify portable, target-specific, dependency-blocked, scaffold-only, and verification work across UCORE, UHAL, UOS, UMATH, UPAL, UMID, UPROTO, UAPP, UCON, and uef-gen. See [catalogue and equivalence audit](../../UETD/docs/audits/UETD_EQUIVALENCE_AND_CATALOGUE_GAP_AUDIT.md) and [UETD TODO: cross-layer readiness](../../UETD/docs/TODO.md#cross-layer-readiness-before-target-selection).

## 2. Stabilize shared contracts and portability

- [ ] Resolve public API, numeric, time, overflow, timeout, synchronization, storage, and C/C++ portability contracts before selecting boards. See [UETD TODO: UOS contracts](../../UETD/docs/TODO.md#uos-timing-deadline-blocking-and-storage-contracts) and [UMATH configuration](../../UETD/docs/TODO.md#umath-type-configuration-and-geometry-boundaries).

## 3. Implement and host-check first-pass-relevant portable functionality

- [ ] Define portable UPAL and UPROTO boundaries, implement the prioritized UMATH/UCON/generic C and middleware slices that shape first-pass requirements, and host-check portable behavior and boundary cases. Keep target pin/DMA/IRQ/cache work for the selected targets and sequence the broader catalogue by project need. See [UETD TODO: Phase 0](../../UETD/docs/TODO.md#phase-0--portable-foundation-and-initial-integration), [UCON contracts](../../UETD/docs/TODO.md#phase-2--ucon-algorithm-contracts-and-generated-specialization), and [host verification](../../UETD/docs/TODO.md#phase-3--packaging-examples-and-hardware-verification).

## 4. Select a multi-target first pass

- [ ] Choose at least two concrete MCU/board configurations based on the capability coverage identified in the portable backlog. See [UETD TODO: target-matrix selection](../../UETD/docs/TODO.md#2-select-and-verify-a-multi-target-first-pass).

## 5. Implement target foundations for that matrix

- [ ] Implement and verify UHAL, UOS backends, clocks, memory/cache behavior, and RTOS profiles for each selected target. See [UETD TODO: target foundations](../../UETD/docs/TODO.md#3-implement-target-foundations-for-the-selected-set) and [target records](../../UETD/docs/TODO.md#phase-0--portable-foundation-and-initial-integration).

## 6. Bring up target-dependent drivers and integrations

- [ ] Add DMA, UPAL drivers, storage, and board-bound protocol/sensor integrations only for targets whose resources and validation scope are defined. See [UETD TODO: peripheral paths](../../UETD/docs/TODO.md#4-bring-up-peripheral-paths-and-board-backed-drivers) and [Phase 1](../../UETD/docs/TODO.md#phase-1--initial-release-peripherals-storage-and-middleware).

## 7. Complete generation and application integration

- [ ] Promote reviewed UCON templates and finish middleware, protocol, UAPP, and generated-project behavior against the implemented APIs. See [UETD TODO: integration](../../UETD/docs/TODO.md#5-complete-middleware-protocols-ucon-generation-and-applications) and [uef-gen TODO: project assembly](../../uef-gen/docs/TODO.md#3-project-assembly-and-provenance).

## 8. Qualify and document supported configurations

- [ ] Package and validate each claimed target/toolchain/backend combination, keeping host checks separate from hardware evidence. See [UETD TODO: qualification](../../UETD/docs/TODO.md#6-package-and-qualify-supported-configurations), [UETD Phase 3](../../UETD/docs/TODO.md#phase-3--packaging-examples-and-hardware-verification), and [uef-gen release](../../uef-gen/docs/TODO.md#milestone-6--integration-and-release).

## 9. Add linked-target and multicore support

- [ ] Specify the system-composition layer above one-target `.uef` projects. Use the existing author-designed Tesla-coil H745/G4/FPGA topology as the acceptance requirement; do not treat its projects as build-ready or as a newly proposed example.
- [ ] Define canonical per-core target facts, system nodes, exported/imported endpoints, versioned link contracts, application roles, and protocol-specific bus roles.
- [ ] Complete the minimum `.uef` compiler path for tasksets/core assignment, then add system-level project resolution and endpoint/contract validation.
- [ ] Qualify H745 M7/M4 startup, images, shared memory/cache behavior and inter-core notifications, then implement one selected H745-to-G4 link with local G4 safety authority.
- [ ] Generate and track one artifact per target/core image; add deployment sequencing and hardware qualification only after exact board/link profiles exist.

See [MULTI_TARGET_AND_MULTICORE_PLAN.md](MULTI_TARGET_AND_MULTICORE_PLAN.md) for the architecture boundary, phased work, unresolved hardware inputs, and acceptance criteria.
