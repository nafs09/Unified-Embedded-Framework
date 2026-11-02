# uef-gen signed artifact and update orchestration

**State:** future deployment work; this is not ordinary flashing and is not
implemented by the current artifact store.

## Signing boundary

Create a canonical signed manifest after the build has produced final segment
hashes and load ranges. The signature binds target compatibility, board
revision range, bootloader compatibility, firmware version/security index,
segment roles/addresses/lengths/hashes, dependency IDs and selected security
profile. Canonicalization and signature algorithm are versioned. Signing uses
an explicit key-provider interface (HSM, OS key store or offline signer); the
private key never enters the generated project, source snapshot or ordinary
artifact directory.

Artifact state distinguishes `unsigned`, `signed`, and
`signature_verified`. A build-side SHA-256 digest is integrity/provenance only;
it is not a signature. The artifact store verifies detached signature and
manifest contents on every load before an authenticated deployment can use
them.

## Target update lifecycle

Update backends declare capabilities: signature verification, protected key
storage, staging region, dual-slot/trial boot, rollback, monotonic version
state, recovery image and minimum bootloader version. The orchestrator rejects
a requested guarantee unsupported by that profile. Single-bank ROM bootloader
flows can be authenticated programming without claiming power-loss rollback.

The backend state machine is receive, stage, verify hash, verify signature,
check exact device and compatibility, program inactive slot, verify programmed
bytes, mark trial, reset, await health confirmation, then confirm or rollback.
Every phase has a deadline, cancellation rule, durable progress state and
power-loss behavior. Reverification occurs immediately before activation.
Probe discovery remains read-only; update requires explicit confirmation
before any state-changing operation.

## Failure and reporting

Report the precise failed phase, verified chip/probe identity, artifact digest,
signature/key ID, programmed range, recoverability and resulting boot slot.
Never describe a failed/partial stage as success. Progress and cancellation
must not skip verification or leave an inactive image selected. Anti-rollback
state advances only after the new image has been confirmed by the target's
boot policy.

## Release evidence

Host tests cover manifest canonicalization, invalid signatures, tampering,
wrong targets and rollback policy. Hardware evidence injects power loss/reset
at each durable state transition and verifies trial confirmation and recovery.
The report links all results to exact artifact, bootloader, board revision and
toolchain hashes. These checks do not imply a security certification.
