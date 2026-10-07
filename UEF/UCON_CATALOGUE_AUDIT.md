# UCON catalogue audit

Audit scope: the UEF-owned catalogue and generated UCON algorithm skeletons, checked against the current `UEF_Specification.md`. This file records code-side resolutions and specification questions; the authored specification was not changed.

## Current catalogue status

The manifest now contains **211 keys**: 100 FIRST_PASS, 107 PLANNED, and 4 DEFERRED. Registered implementations: LEAD_LAG, PID. A priority means work scope, not implementation maturity. Every other key is `registered: false`, has no output template, and its generated functions return `UCON_NOT_IMPLEMENTED` without writing state or outputs.

The catalogue includes 50 clearly tagged roadmap proposals beyond Part XVI. They live under `ROADMAP_EXTENSIONS.md` and remain future/unselectable entries.

The per-algorithm source files now outline the module lifecycle and its algorithm-family operations. Each declaration has its own source definition and TODO that names the operation, required data, constraints, and unresolved policy. The shared type-erased call envelope is a development boundary only; replace it with typed fixed-size configuration/state before implementation. Empty `features`, `numerics`, `modules`, and `outputs` remain explicit gaps where the specification has not selected concrete contracts. They are not silently filled with invented parameters.

## Corrected catalogue issues

| Key or group | Audit action |
|---|---|
| `FREQ_EST` | Removed as a concrete algorithm row; retained as a compatibility alias to PLL, FLL, ZERO_CROSS_FREQ, and SLIDING_DFT. |
| `MHE` | Moved to estimation/moving_horizon. |
| `PARTICLE` | Moved to estimation/bayesian. |
| `RLS` | Moved to estimation/identification. |
| `SCALAR_KF` | Moved to estimation/kalman and described as a one-state Kalman filter. |
| `ESO` | Moved to estimation/observers; ADRC is separately classified as disturbance-rejection control. |
| `RISE and FRACTIONAL_ORDER_SMC` | Moved to control/nonlinear. |
| `HALF_CYCLE_PREDICTIVE_CURRENT_CONTROL` | Moved to electrical/power_conversion; remains deferred until topology/timing/control law are defined. |
| `RESONANT_EKF_LLC_SRC` | Moved to estimation/kalman/power_conversion; remains deferred and model-undefined. |
| `ALPHA_BETA`, `ALPHA_BETA_GAMMA` | Moved to estimation/tracking; these are kinematic state estimators, not generic signal filters. |
| `FIR_WINDOWED` | Moved to filtering/coefficient_design; it designs FIR taps, while `FIR` performs runtime convolution. |
| `ADRC` | Moved to control/disturbance_rejection; `ESO` is its estimator component. |
| `GAIN_SCHEDULE` | Moved to control/scheduled; gain scheduling does not itself imply online adaptation. |
| `PID_AUTOTUNE` | Moved to control/tuning; it is a supervised tuning utility, not an implicit runtime adaptive controller. |
| `REFERENCE_GOVERNOR` | Moved to control/safety as a constraint-aware reference shaper. |
| `CONTROL_ALLOCATOR` | Moved to control/allocation for bounded mapping from generalized commands to actuator commands. |
| `SENSOR_FDI` | Moved to infrastructure/fault_detection because its residual interface is domain-generic. |
| `POLE_PLACE` | Moved to control/synthesis and outlined as offline gain synthesis, not a runtime state-feedback step. |
| `CKF` | Catalogue wording corrected to third-degree spherical-radial cubature. |

## Specification conflicts and unresolved contracts

1. **CKF quadrature.** Part 16.6 states the CKF uses a third-degree spherical-radial cubature rule and explicitly says it is not Gauss-Hermite. The older catalogue row around 16.8 and the later CKF implementation sketch still say third-order Gauss-Hermite. The code-side catalogue now follows the explicit 16.6 correction. Reconcile the stale rows/equations in the specification before CKF implementation.
2. **DEADBEAT_MPC priority.** Part 15.4 calls `DEADBEAT_MPC` deferred. Part 16.6 retains it as planned, and the UEF API currently selects it for the QCWDRSSTC project set. The code keeps the more specific Part 16.6/project selection (`PLANNED`, unregistered) and records this as an authored-spec consistency issue.
3. **RESONANT_EKF_LLC_SRC meaning.** The name plausibly denotes an EKF for an LLC series-resonant converter. That is an interpretation of the key, not a defined model. The state vector, unknown parameters, switched/averaged plant, measurements, observability, and operating region remain open. No estimator equations have been invented.
4. **PID feature parity.** The registered C implementation supports P/I/D, two-degree-of-freedom weights, Tustin integral, filtered derivative, output saturation, integral-state bounds, conditional integration, and back-calculation. Part 16.14 also lists deadband and output rate limiting, which are not PID configuration fields in the current C API. Decide whether these belong inside PID or should be explicit graph blocks; do not assume the current implementation covers them.
5. **Observer naming.** The specification describes `EDOB` as an extended disturbance observer with state estimation and `ESO` as a standalone extended state observer used in ADRC. The catalogue now records these separately; the exact augmented state and relationship to `DOB` still need a typed contract to avoid three aliases for the same behavior.
6. **Deferred converter/control methods.** `HALF_CYCLE_PREDICTIVE_CURRENT_CONTROL` needs its topology, timing boundary, prediction equation, candidate actions, constraints, and fault behavior. `FRACTIONAL_ORDER_SMC` needs a finite-memory/discretization contract. `DAHLIN` needs a defined plant/discretization design range. They remain deferred.

## Existing implementations reviewed

- **Shared mathematics:** scalar/fixed-point, vector, matrix, complex, quaternion, transform, and polynomial primitives are owned by UMATH; UCON keeps a compatibility spelling `ucon_scalar_t` and algorithm-specific behavior.
- **Lead-lag:** the source recurrence matches the Part 16.8 bilinear transform of `K(1 + Tlead*s)/(1 + Tlag*s)`, including the static gain factor. It computes into a local candidate and commits state/output only after finite-value checks.
- **PID:** the implementation follows the stated two-degree-of-freedom error channels, trapezoidal integral, filtered derivative, conditional integration, and back-calculation update. Its clamp mode bounds the stored integral state; it is not an output-aware clamp. See the feature-parity question above.
- The source-level audit found no other implemented UCON algorithm: PID and lead-lag are the only `registered: true` algorithm entries. No build or tests were run during this catalogue pass.

## Family coverage added

The roadmap now includes these distinct method groups:

- **State estimation:** adaptive/fading-memory/robust Kalman variants, Gaussian-sum, UD-factorized, delayed-measurement and dual estimators; high-gain and sliding-mode observers.
- **Online identification:** recursive AR, ARMA, ARX, ARMAX, output-error and Box-Jenkins estimators, recursive instrumental variables and prediction-error minimization, plus persistent-excitation monitoring.
- **Signal processing:** LMS/NLMS adaptive filters; FFT, Goertzel and spectral-peak interpolation; synchronous demodulation; RMS/peak statistics; CIC decimation, polyphase resampling, and Savitzky-Golay filtering.
- **Optimization and control:** active-set, ADMM and projected-gradient QP solvers; H2 state feedback; dynamic-surface and integral sliding-mode control; adaptive and gain-scheduled MPC.
- **Guidance and trajectories:** pure pursuit, line-of-sight guidance, minimum-snap trajectory optimization, and time-optimal motion profiles.
- **Electrical drives and conversion:** average/peak current-mode, hysteretic-current and phase-shift full-bridge control; MTPA, flux weakening and a sensorless FOC observer outline.

These additions are literature-backed catalogue directions, not a claim that they replace MATLAB/Scade design, simulation, verification, certified code generation, or safety tooling. Each method still needs a UEF-specific contract and validation record.
