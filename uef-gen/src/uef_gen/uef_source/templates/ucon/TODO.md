# UCON template work

- [ ] As each algorithm design is completed, create its family directory and add the reviewed UEF-owned implementation/template assets; this tree currently contains catalogue documentation only, so do not fill it with guessed or placeholder equations.
- [ ] Add one metadata record per template: algorithm name/version, supported arithmetic, fixed dimensions, inputs/outputs, tunable parameters, generated file list, required UPAL modules, and safety constraints.
- [ ] Separate structural parameters from runtime tuning values as required by the spec.
- [ ] Implement and review `config.h`, execution wrappers, and HAL bindings before advertising algorithm templates.
- [ ] Define the versioned UEF-to-uef-gen selection/assembly contract, then make the separate generator consume the released UEF catalogue and reject missing families with an actionable diagnostic.
- [ ] Track planned-only families from Part XV §15.4 separately from released template bodies.
