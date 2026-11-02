# First-pass design catalogue

`registry.json` owns first-pass requirements whose implementations or target
integration are incomplete. The public locator is `api.json.first_pass_catalogue`.
Every item declares `priority: FIRST_PASS`, `phase: 0`, `status: design_only`
and `generation: blocked`. Priority does not grant runtime or generation readiness.

The approved placement migration moved 20 planning records here from the later
catalogue. Runtime source ownership remains in `registry/modules/`; language,
deployment and evidence contracts remain with their owning uef-gen components.
Shared schemas live in `registry/contracts/`. Do not duplicate these items in
`registry/future/`, which now contains genuinely later scope only.

Use `load_first_pass_catalogue` for inspection and contract/schema reference
checks. Ordinary module selection still checks implementation and generation
readiness independently. See the [first-pass scope and migration record](../../docs/design/complete-system/FIRST_PASS_SCOPE.md).
