# UAPP secure boot and field-update design

**State:** future optional system service. A build hash proves artifact
identity; it does not authenticate the publisher or make a target update-safe.

## Security profiles

Define selectable profiles rather than imposing one update mechanism on every
target:

| Requirement | Guarantee | Target requirements |
|---|---|---|
| Development unsigned | Identity and corruption checks only; unsigned images are permitted only by explicit local development policy. | Development boot path; never implies release authentication. |
| Authentication | Verify a digital signature over the canonical manifest binding every image segment, target identity, version and load range. | Protected trust anchor and verifier in boot path. |
| Recovery | Staging, power-loss-safe activation, trial boot, health confirmation and rollback. This composes with authentication. | Dual slot or equivalent recovery image and enough flash. |
| Anti-rollback | Reject versions below protected monotonic security index. This can be required with or without recovery. | Non-resettable/secure monotonic counter or equivalent protected state. |

Recovery and anti-rollback are independent requirements; a release can require
both. Recovery and anti-rollback always imply authentication. Encryption is
orthogonal and is added only when confidentiality is required. Unsigned
development cannot be combined with authentication or accepted by a release
policy.
Checksums detect accidental corruption and are never treated as an
authentication mechanism.

## Signed manifest

The signed object is a canonical serialization with a specified encoding and
signature algorithm. The signed bytes are the canonical top-level manifest
with the detached `signature` envelope omitted; including the signature in its
own signed payload is forbidden. The verifier applies a fixed domain-separation
prefix, an allow-listed algorithm, and a key policy to those exact bytes.
Canonicalization version and duplicate-key/unknown-critical-field behavior are
defined so different parsers cannot verify different meanings. It binds
project ID, artifact schema, exact chip/board
compatibility set, hardware revision range, bootloader minimum/maximum,
firmware version, monotonic security index, image segment role/address/length,
image hashes, dependencies, and update policy. Signature metadata outside the
payload includes algorithm, key ID, signature bytes or a detached signature
file hash. Key IDs resolve only through the protected trust policy; the
signature envelope cannot add a trusted key. Mutable deployment progress is
stored outside the signed payload.

Key lifecycle covers manufacturing provisioning, key rotation, revocation,
recovery key policy, debug unlock state and what happens when no valid key
remains. `registry/future/trust_policy.schema.json` sketches key IDs, allowed
project/target scope, lifecycle state, protected key references and revocation
epoch; it contains no private-key material. A project cannot claim
authenticated release unless the selected bootloader profile verifies the
signed data using an immutable or protected trust root.

## Update state machine

Required states: idle, receiving, staged, integrity-checked, signature-
verified, compatible, pending-trial, trial-boot, confirmed, rollback, and
failed/recovery. Every transition is idempotent across reset. The boot selector
uses an atomic marker with integrity protection. Before trial activation the
old confirmed image remains bootable. Confirmation happens only after an
application-defined readiness/health window; watchdog reset or failed
confirmation returns to the last confirmed image. Single-bank devices may
support authenticated programming only if their ROM/vendor boot path supplies
a documented recovery guarantee; otherwise they must not advertise recoverable
field update.

The recoverable profile persists a versioned update-progress record atomically
at each durable transition; `registry/future/update_progress.schema.json` is
the proposed record. On resume, the device revalidates the manifest signature,
exact target, staged image and segment hashes from the beginning. A progress
record or CRC alone never authorizes image activation. Cancellation and retry
must preserve the previous confirmed slot and use exact expected sequence
numbers so an older session cannot overwrite newer progress.

The updater verifies transport chunks as they arrive, enforces declared length
and timeout, stages without executing unverified bytes, verifies the complete
artifact digest and every segment digest, verifies the signature over the
canonical manifest bytes, checks exact target/board/hardware/bootloader
compatibility, then commits. Segment order, overlap, load-range overflow and
the relationship between transfer offsets and load addresses are validated
before activation.
Anti-rollback state advances only after successful confirmation, with a
power-loss-safe monotonic update rule.

## uef-gen boundary

uef-gen records signed-manifest identity, signing status, signer/key ID,
security profile, image hashes, compatibility constraints and evidence links.
It must never report an artifact as authenticated merely because it contains
SHA-256 values. Signing is a separate explicit operation using a configured
key provider; private keys are not copied into generated projects or artifacts.
Deployment must reverify signature and exact target compatibility immediately
before an update operation.

## Release gate

Profile selection is target-capability checked. Promotion requires injected
power-loss/reset at every state transition, tampered image/signature rejection,
wrong-target refusal, rollback verification, anti-rollback tests where enabled,
and review of key provisioning/recovery. These tests produce evidence records;
they do not by themselves constitute a safety/security certification.
