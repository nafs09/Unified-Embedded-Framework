# Pre-publication checklist

This checklist tracks the legal, security, documentation, and release work
needed before publishing the repository.

## Legal and provenance

- [x] Add the exact GPL-3.0 license text at the repository root and declare
  the same license for uef-gen and its VS Code extension metadata.
- [x] Document that UEF (UETD and uef-gen) is licensed under GPL-3.0-only and
  that third-party components retain their own terms.
- [ ] Decide whether inbound contributions are covered by the project license
  alone, a Developer Certificate of Origin, or a Contributor License Agreement.
  Update `CONTRIBUTING.md` before accepting outside code.
- [ ] Confirm the copyright holder has authority to license every original
  contribution and all assets in the repository.
- [ ] Audit `UETD/third_party/`, generated examples, copied code, schemas, and
  documentation for license notices, attribution, redistribution terms, and
  exclusions. Retain upstream notices where required.
- [ ] Remove secrets, private logs, personal data, unpublished credentials,
  and accidental binary/build output from the Git history and current tree.

## Community setup

- [ ] Choose the public repository description, website, topics, default
  branch, and whether GitHub Discussions will be enabled.
- [ ] Configure repository labels used by issue triage and templates. Keep
  labels aligned with the issue forms in `.github/ISSUE_TEMPLATE/`.
- [ ] Set a monitored conduct contact in the repository's **About** section.
  Arrange an independent route for reports involving the primary maintainer.
- [ ] Enable GitHub private vulnerability reporting and set a monitored
  fallback security contact before publishing `SECURITY.md` as the reporting
  policy.
- [ ] Identify the maintainer(s) responsible for triage, code review, releases,
  and security reports. Add backup maintainers before project activity requires
  uninterrupted coverage.

## Repository controls and automation

- [ ] Protect the default branch: require pull requests, prevent force pushes
  and deletion, and require appropriate status checks once they exist.
- [ ] Add CI for the supported host build and uef-gen checks before inviting
  routine contributions. Report cross-compilation and hardware validation
  separately; do not label a host build as target qualification.
- [ ] Confirm workflow permissions are read-only by default and add narrowly
  scoped secrets only when a release or deployment workflow needs them.
- [ ] Decide how release artifacts, checksums, SBOMs, and release notes will be
  published. Keep UETD and uef-gen version compatibility explicit.
- [ ] Review the GitHub community profile and confirm README, contribution,
  conduct, security, and license information is discoverable.

## User documentation to author before inviting users

The architecture, audits, roadmaps, contribution policy, and release procedure
are indexed in this repository. They are maintainer/design documents; they do
not replace the following practical user and integration guides. Keep every
guide explicit about whether a command, language feature, target, or workflow
is implemented, scaffolded, or hardware verified.

- [x] **Initial UEF-PL public documentation.** The repository now has an
  [official user handbook](guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md), a
  [technical source guide](guides/UEF_PROJECT_LANGUAGE_GUIDE.md), a
  [registry/discovery guide](guides/UEF_PL_REGISTRIES.md), and generated
  [module/type/UMATH](reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md) and
  [unit](reference/UEF_PL_UNITS_REFERENCE.md) references. These documents
  distinguish the implemented frontend from the unfinished firmware emitter,
  build, and target pipeline; they do not claim release-ready firmware
  generation.
- [ ] **Release-ready UEF Project Language specification and reference.** Use
  “UEF Project Language” as the readable name; refer to it informally as the
  `.uef` language. Explain installation and CLI workflow; lexical rules and
  grammar; declarations, records, fixed-width enums, scalar/vector/matrix types
  and units; expressions and casts; component and driver bindings; models,
  algorithms, protocols, subsystems and hybrid composition; operating system,
  tasks, timing, interrupts and failure policies; `.uefs` multi-target
  composition; diagnostics; editor support; generation limits; and contract
  versioning. Provide small complete projects and annotated diagnostics.
- [ ] **Getting started / first generated firmware tutorial.** Once a supported
  standalone generation path exists, cover supported
  host tools and versions, installing UETD and `uef-gen` from this monorepo,
  configuring UETD discovery, inspecting a target, creating and validating a
  project, selecting modules, generating, building, flashing/monitoring where
  supported, and inspecting provenance/manual actions. Include one honest
  example from a clean checkout and state exactly which steps require a real
  board or external vendor tools.
- [ ] **Supported target and toolchain matrix.** For each selectable
  board/target, name the exact MCU/package and board revision, compiler and
  version, runtime, build route, available peripherals, memory/startup/linker
  status, tested examples, and evidence date. Separate roadmap/family/chip
  records from profiles verified for a physical board. State exclusions and
  qualification level.
- [ ] **Hardware porting and driver-author guide.** Explain how to bring up a
  board and implement UHAL/UPAL ports; add a target resource map; connect clocks,
  pins, DMA, interrupts, cache/coherency, memory, and safe states; implement a
  sensor/actuator contract; expose calibration storage; and report timing,
  blocking, error, and concurrency behavior. Include a minimal driver example
  and a checklist for validating it on hardware.
- [ ] **Safety and real-time integration guide.** Document the runtime lifecycle,
  watchdog servicing and independence, fault propagation, safe-state contract,
  scheduling/deadline assumptions, priority inversion and ISR restrictions,
  startup/shutdown behavior, and how users verify worst-case timing and
  failure paths. Clearly state that library use alone is not a safety
  certification or a substitute for independent hardware protection.
- [ ] **Public API and compatibility reference.** Document installed/public C
  headers, module selection and dependencies, ownership/lifetime, units and
  numeric policies, return/error behavior, concurrency and ISR constraints,
  memory bounds, and generated artifact contracts. Define which headers and
  serialized formats are stable; describe UETD/uef-gen version matching,
  deprecation, breaking changes, and migration notes. Generate repetitive
  symbol reference material from source where practical, but review the
  behavioral contracts by hand.
- [ ] **Catalogue extension guide.** Show the complete path for proposing and
  adding a module, algorithm, protocol, exact chip, development board, or
  target profile: authoritative record location, schemas, manifest fields,
  IDs/naming, dependencies, readiness gates, tests/evidence, generated outputs,
  and how uef-gen consumes the UETD-owned contract without shadow catalogues.
  Clarify when an entry is only a future scaffold and what is required before
  it becomes selectable.
- [ ] **Troubleshooting and diagnostics guide.** Explain common configuration,
  registry, generation, toolchain, build, flashing, runtime-fault, and
  language-diagnostic failures; show how to collect a useful bug report without
  exposing secrets or private data.
- [ ] **Versioned release notes and migration notes.** For each release, list
  user-visible changes, supported UETD/uef-gen pairs, compatibility changes,
  target/evidence changes, known limitations, and upgrade steps. The root
  `CHANGELOG.md` is drafted and should become the maintained source for these.

Published capability claims match the released implementation. Guides label
unimplemented features as unavailable and distinguish design examples from
runnable, target-qualified projects.

## Launch gate

- [x] Replace the former blanket pre-publication notice with an accurate
  licensing statement in the root README. Contribution terms remain a launch
  decision.
- [ ] Ensure all documentation links work from the repository's default branch.
- [ ] Review the current-readiness statements in both project READMEs, target
  catalogues, examples, and TODOs.
- [ ] Make the repository public only after the legal, security-contact, and
  contributor-process blockers above are resolved.

GitHub's [`SECURITY.md` policy guidance](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting/add-security-policy)
and [private reporting setup](https://docs.github.com/en/code-security/how-tos/report-and-fix-vulnerabilities/configure-vulnerability-reporting)
describe the separate repository settings needed for vulnerability intake.
