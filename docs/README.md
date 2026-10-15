# Repository documentation

The Unified Embedded Framework (UEF) contains two separately versioned but
integrated components:

- [UETD](../UETD/README.md) is the C11 embedded framework. Its architecture,
  catalogues, audits, and implementation work are indexed in
  [UETD documentation](../UETD/docs/README.md).
- [uef-gen](../uef-gen/README.md) is the Python project assembler. Its source
  architecture, language audit, and implementation work are indexed in
  [uef-gen documentation](../uef-gen/docs/README.md).

Both components are licensed together under the root GPL-3.0-only license.

Cross-project design and planning documents:

- [Implementation plan](roadmap/IMPLEMENTATION_PLAN.md)
- [Multi-target and multicore plan](roadmap/MULTI_TARGET_AND_MULTICORE_PLAN.md)
- [Standalone project language design](design/PROJECT_LANGUAGE_DESIGN.md)
- [UEF-PL user handbook](guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md) is the
  official getting-started and usage guide for standalone `.uef` projects.
- [UEF-PL technical guide](guides/UEF_PROJECT_LANGUAGE_GUIDE.md) gives the
  source structure, type and operator rules, composition model, and runtime
  contract.
- [UEF-PL registries and discovery](guides/UEF_PL_REGISTRIES.md) explains the
  JSON catalogues, how names are resolved, and where to make registry changes.
- [Generated language catalogue](reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md)
  and [unit reference](reference/UEF_PL_UNITS_REFERENCE.md) provide the
  registry-derived module/type/UMATH inventories and dimension-grouped units.

Repository preparation and release guidance:

- [Pre-publication checklist](PUBLICATION_CHECKLIST.md)
- [Release procedure](RELEASING.md)

The documents describe plans and current repository state; they do not mean a
scaffolded feature or target is complete or hardware-qualified. Check the
relevant subproject README and catalogue status before relying on a capability.
