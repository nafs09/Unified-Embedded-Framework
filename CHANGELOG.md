# Changelog

User-visible changes to the Unified Embedded Framework (UEF), which consists of
UETD and `uef-gen`, are recorded here. Entries are grouped by component because
the two components have separate package versions and release schedules.

## Unreleased

### Repository

- Drafted contributor, conduct, security, support, publication, and release
  guidance, along with GitHub issue forms and a pull request template.
- Organized cross-project design and planning documents under `docs/`, with
  documentation indexes for UETD and uef-gen.

### UETD

- Hardened the UAPP FSM lifecycle: machine storage must start with
  `UAPP_SM_INITIALIZER`, reinitializing a live instance returns `UETD_BUSY`,
  and CMake propagates one FSM capacity configuration to the library and its
  consumers. Event posting now copies only declared payload bytes and clears
  unused bytes. The public storage initialization contract is a source-level
  compatibility change for FSM callers.
- Clarified FSM status, callback failure, execution ownership, and platform
  ingress contracts. Tightened signed capacity bounds in the FSM, health
  registry, fault queue, and logger.

### uef-gen

- UEF-PL language version 13 now requires one FSM owner and an explicit
  plural `consumers` list; channels use the same plural subscriber form.
  Callback results, owner scheduling, per-consumer delivery, synchronized
  ingress, and the required OS failure policy are validated and preserved in
  the backend-neutral specification. State consumers receive race-free
  latest-value snapshots on their own schedules; channel subscribers receive
  independent bounded FIFO routes. The UETD adapter and firmware emitter remain
  unimplemented, and this compatibility change does not produce executable
  firmware.
- Documentation paths and indexes were updated. The top-level overview now
  describes the standalone `.uef` language consistently with its current
  design document.

Before publishing a release, replace placeholder entries with the changes,
compatibility notes, and validation evidence for that release. Follow
[`docs/RELEASING.md`](docs/RELEASING.md).
