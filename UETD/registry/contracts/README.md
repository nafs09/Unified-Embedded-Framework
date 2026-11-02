# First-pass design contract schemas

These schemas specify proposed cross-layer contracts from the
[complete-system design](../../docs/design/complete-system/README.md). They
validate design/profile structure, not implementation readiness or hardware
qualification. Their records remain `design_only` and generation-blocked.

- `design_contract.schema.json`: priority, ownership, interfaces, resource/failure
  rules, dependencies and acceptance obligations for a missing capability.
- `provider_contract.schema.json`: pinned implementation identity, operation
  subset, context/ownership, memory/work bounds, target scope and evidence.
- `driver_transaction.schema.json`: bounded asynchronous admission, completion,
  cancellation, buffer loans and resource requirements.
- `resource_budget.schema.json`: per-image memory/work allocations with explicit
  evidence class, unknown values and semantic validation obligations.

These complement the existing module authoring and qualification schemas; they
do not define a competing module/type catalogue. Implementation should reference
or extend the existing owning contracts rather than copy their meanings.

The ten shared data/update/evidence schemas now live here after the approved [first-pass placement migration](../../docs/design/complete-system/FIRST_PASS_SCOPE.md#proposed-migration-of-older-first-pass-entries). Their relocation grants no implementation or generation readiness.

Schema checks cannot establish that a dependency exists, a C ABI matches its
logical types, two memory ranges are disjoint, a timing measurement proves a
bound, a provider is trustworthy, or a physical board has the claimed behavior.
The generator must perform those cross-record and evidence checks separately.
