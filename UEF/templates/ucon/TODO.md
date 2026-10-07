# UCON implementation queue

## Catalog and scaffold organization

- [x] Keep UEF as the only owner of UCON algorithms, typed APIs, catalog metadata, and generation templates.
- [x] Keep one header/source pair per unregistered catalog algorithm, grouped by domain/family rather than aggregating unrelated algorithms into family files.
- [x] Place first-pass and project-relevant planned placeholders in normal family directories; put the remaining planned and every deferred placeholder under `ucon/future/`.
- [x] Give each placeholder a detailed `TODO(UCON-KEY)` covering the catalog's algorithm summary, declared features/numerics/modules/actions, and the contract decisions still required.
- [x] Register each placeholder's unique file pair and module in UEF metadata, and mark the module as non-selectable scaffolding.
- [x] Have uef-gen verify the manifest/file/module registration and reject unregistered algorithms before staging output.

## Before promoting an algorithm

- [ ] Define typed, allocation-free configuration/state and init/reset/step semantics. State dimensions, units, valid ranges, aliasing, sample-time assumptions, and per-instance memory.
- [ ] Implement the equations and fixed-size numeric kernels. Bound loops/iterations and define saturation, convergence, finite-value checks, error returns, and state/output preservation on failure.
- [ ] Add a UEF-owned wrapper template only when ControlIR node mapping is stable. Bind named ports/configuration and emit all required files without putting algorithm behavior in uef-gen.
- [ ] Record accepted ControlIR values, structural dimensions, required modules/capabilities/numeric helpers, output paths, and manual hardware/application actions in the shared manifest.
- [ ] Update `registry/modules/registry.json`, `uef_api.json`, public umbrella headers when appropriate, and the UCON README/TODO with the implementation status.
- [ ] Move the completed implementation into its normal family file path, remove its generated placeholder registration, and keep the direct and generated interfaces aligned.
- [ ] Compare direct C and generated instances for initialization, reset, saturation, bounds, numerical failure, and state preservation before setting `registered: true`.

## Project-focused initial set

The initial directory includes all `FIRST_PASS` entries and the following planned work, with its reason recorded per manifest entry:

- Common filters and commissioning: Bessel, Butterworth, Chebyshev I/II, elliptic, windowed FIR, and PID autotuning.
- Drone: adaptive/backstepping options, cubature/iterated/square-root and other nonlinear estimators, Bezier/B-spline/flatness trajectory work, feedback linearization, INDI2, reference governance, and CLF/CBF safety controllers.
- QCWDRSSTC: DDP/iLQR and deadbeat, economic, robust, sparse, stochastic, and zone MPC options; extremum seeking; square-root UKF.
- Medical: PK/PD target control.

The exact allowlist lives in `UEF/tools/generate_ucon_scaffolds.py` and is mirrored in `uef_api.json`; the manifest records the resulting stage and rationale on each entry. Other planned and all deferred entries remain in `include/uef/ucon/future/` and `src/ucon/future/`.

## Regeneration

After editing the catalog family, key, priority, or planned-project allowlist, run:

```powershell
python tools/generate_ucon_scaffolds.py
```

The generator refreshes one C header/source pair per unregistered algorithm plus its paths in the shared ControlIR manifest, per-algorithm module entries, and API directory map. It prunes stale generated files by its marker and preserves hand-maintained files.
