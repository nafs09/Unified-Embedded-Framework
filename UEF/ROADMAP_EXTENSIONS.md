# UCON roadmap extensions

This curated extension list fills visible coverage gaps in UEF Part XVI for embedded estimation, identification, signal processing, optimization, control, guidance, trajectories, and power conversion. These are **PLANNED catalogue proposals** only. Their generated functions are fail-closed outlines, not equations, safety claims, or deployable algorithms. UCON remains the single owner of implementations and templates; uef-gen only reads registered UEF entries.

Each proposal has an individual detail file under `registry/control_ir/manifest/algorithms/`, indexed by key in `registry/control_ir/registry.json`. Its implementation notes identify the first decisions that must be made. The list is intentionally finite and project-oriented; it does not promise every method in numerical optimization, signal processing, or control theory.

## Proposals

| Key | Family | Scope |
|---|---|---|
| `ADAPTIVE_KF` | `estimation/kalman` | Adaptive Kalman filter with an explicitly bounded covariance or noise adaptation law. |
| `ADAPTIVE_MPC` | `control/predictive` | MPC with a bounded online update to selected plant-model parameters. |
| `AVERAGE_CURRENT_MODE_CONTROL` | `electrical/power_conversion` | Average-current-mode controller for a declared converter topology and sampled current loop. |
| `CIC_DECIMATOR` | `signal_processing/resampling` | Cascaded-integrator-comb decimator with configured rate change, stage count, and differential delay. |
| `CUSUM_CHANGE_DETECTOR` | `estimation/diagnostics` | Sequential cumulative-sum detector for a configured change in a scalar or vector statistic. |
| `DUAL_KF` | `estimation/kalman` | Dual estimation scheme that jointly tracks plant state and selected unknown parameters. |
| `DYNAMIC_SURFACE_CONTROL` | `control/nonlinear` | Nonlinear dynamic-surface control using filtered virtual-control derivatives. |
| `FADING_MEMORY_KF` | `estimation/kalman` | Kalman filter variant that discounts old information using a configured fading-memory factor. |
| `FFT_RADIX2` | `signal_processing/transforms` | Radix-2 discrete Fourier transform for fixed-size power-of-two blocks. |
| `FLUX_WEAKENING_CONTROL` | `electrical/motor_control` | Flux-weakening reference generation under a declared motor voltage, current, and speed envelope. |
| `GAIN_SCHEDULED_MPC` | `control/predictive` | MPC profile that selects among declared model/controller schedules using a bounded scheduling variable. |
| `GAUSSIAN_SUM_FILTER` | `estimation/kalman` | Gaussian-sum filter that maintains and reduces a finite mixture of Gaussian state estimates. |
| `GOERTZEL` | `signal_processing/transforms` | Single-bin or small-bin DFT evaluation using a bounded Goertzel recurrence. |
| `H2_STATE_FEEDBACK` | `control/state_space` | H2-optimal state-feedback law using gains synthesized offline for a declared plant and performance model. |
| `HIGH_GAIN_OBSERVER` | `estimation/observers` | High-gain nonlinear observer for a declared observable normal-form model. |
| `HYSTERETIC_CURRENT_CONTROL` | `electrical/power_conversion` | Hysteresis-band current controller that selects switching actions from current error and configured bounds. |
| `INNOVATION_MONITOR` | `estimation/diagnostics` | Monitor estimator innovations and covariance-normalized residuals for configured consistency limits. |
| `INTEGRAL_SMC` | `control/nonlinear` | Integral sliding-mode control with a declared surface, reaching law, and bounded integral state. |
| `LMS` | `filtering/adaptive` | Least-mean-squares adaptive FIR update driven by a declared reference and error signal. |
| `LOS_GUIDANCE` | `aerospace/guidance` | Line-of-sight guidance that converts a declared path-relative geometry into a bounded guidance command. |
| `MINIMUM_SNAP_TRAJ` | `trajectory/optimization` | Polynomial trajectory generation minimizing integrated snap under configured boundary and corridor constraints. |
| `MTPA_CONTROL` | `electrical/motor_control` | Maximum-torque-per-ampere current-reference selection for a declared motor model and operating region. |
| `NLMS` | `filtering/adaptive` | Normalized LMS adaptive FIR update with input-energy normalization and a positive regularizer. |
| `OUT_OF_SEQUENCE_KF` | `estimation/kalman` | Kalman update for delayed measurements whose timestamps precede the current estimate. |
| `PEAK_CURRENT_MODE_CONTROL` | `electrical/power_conversion` | Peak-current-mode controller with a topology-specific peak threshold and cycle-by-cycle switch decision. |
| `PEAK_TRACKER` | `signal_processing/statistics` | Bounded peak and optional trough tracker with explicit decay or reset behavior. |
| `PERSISTENT_EXCITATION_MONITOR` | `estimation/identification` | Monitor whether a configured regressor supplies sufficient excitation over a bounded window. |
| `PHASE_SHIFT_FULL_BRIDGE_CONTROL` | `electrical/power_conversion` | Phase-shift modulation/control outline for a full-bridge converter with explicit phase and dead-time conventions. |
| `POLYPHASE_RESAMPLER` | `signal_processing/resampling` | Rational-rate polyphase FIR resampler with a declared phase table and coefficient set. |
| `PURE_PURSUIT_GUIDANCE` | `aerospace/guidance` | Geometric path-following guidance that aims toward a configured look-ahead point. |
| `QP_ACTIVE_SET` | `optimization/qp` | Bounded active-set quadratic-program solver for a fixed-shape convex QP. |
| `QP_ADMM` | `optimization/qp` | Alternating-direction method of multipliers solver for a fixed-shape convex QP. |
| `QP_PROJECTED_GRADIENT` | `optimization/qp` | Projected-gradient solver for a convex quadratic objective with an explicitly projectable feasible set. |
| `RECURSIVE_AR` | `estimation/identification` | Online parameter estimation for an autoregressive time-series model. |
| `RECURSIVE_ARMA` | `estimation/identification` | Online parameter estimation for an autoregressive moving-average time-series model. |
| `RECURSIVE_ARMAX` | `estimation/identification` | Online estimation of an ARX model augmented with a moving-average noise model. |
| `RECURSIVE_ARX` | `estimation/identification` | Online estimation of an autoregressive model with exogenous inputs. |
| `RECURSIVE_BJ` | `estimation/identification` | Online Box-Jenkins model estimation with separate plant and noise polynomials. |
| `RECURSIVE_IV` | `estimation/identification` | Recursive instrumental-variable estimator for regressions with correlated disturbances. |
| `RECURSIVE_OE` | `estimation/identification` | Online output-error model estimation with a declared plant and noise separation. |
| `RMS_WINDOW` | `signal_processing/statistics` | Windowed root-mean-square estimator with bounded sample storage or running-sum state. |
| `ROBUST_KF` | `estimation/kalman` | Robust Kalman estimation variant with a named bounded-influence or outlier-resistance rule. |
| `RPEM` | `estimation/identification` | Recursive prediction-error minimization for a declared parameterized input-output model. |
| `SAVGOL_FILTER` | `filtering/runtime` | Savitzky-Golay local polynomial smoother or derivative estimator with fixed window and order. |
| `SENSORLESS_FOC_OBSERVER` | `electrical/motor_control` | Sensorless motor-state observer supplying the electrical angle/speed needed by a declared FOC profile. |
| `SLIDING_MODE_OBSERVER` | `estimation/observers` | Sliding-mode state observer with an explicit output injection and convergence assumptions. |
| `SPECTRAL_PEAK_INTERPOLATION` | `signal_processing/analysis` | Sub-bin frequency estimate from a declared interpolator around a detected spectral peak. |
| `SYNCHRONOUS_DEMOD` | `signal_processing/demodulation` | Synchronous demodulator using a phase-aligned reference and bounded low-pass accumulation. |
| `TIME_OPTIMAL_PROFILE` | `trajectory/profiles` | Time-optimal one-dimensional motion profile under declared velocity, acceleration, and jerk limits. |
| `UD_KF` | `estimation/kalman` | UD-factorized Kalman filter that propagates covariance as unit-upper and diagonal factors. |

## Promotion gate

Before a proposal becomes selectable, write the algorithm contract into the UEF specification and manifest: typed dimensions and units, inputs/outputs, fixed-size state and workspace, init/reset/step behavior, numeric and convergence policy, bounded runtime and memory, failure-state preservation, and any manual hardware/model actions. Implement generic C in UEF, provide UEF-owned templates only where generation needs structural specialization, update API/module metadata, and review the result against the specification. Do not move a stub into `registered_algorithms` or set `registered: true` to indicate interest or priority.

## Selection notes

- Online AR/ARMA/ARX/ARMAX/OE/BJ estimation, recursive prediction-error methods, and recursive instrumental-variable methods are kept as distinct model/estimator families because they have different regressors and update contracts.
- LMS/NLMS are adaptive FIR coefficient-update algorithms. They are not substitutes for plant-parameter RLS.
- FFT/Goertzel/spectral interpolation, synchronous demodulation, rolling RMS/peak tracking, and CIC/polyphase rate conversion cover common deterministic embedded signal paths.
- QP solver algorithms are separated from MPC formulations. MPC entries must state which solver interface they consume; adding a solver does not make a controller formulation complete.
- Motor/power-converter candidates stay under electrical families because topology, switching timing, sensing, and safe gate behavior are essential parts of their contracts.
- Medical control profiles remain software boundaries. Clinical models, therapy values, independent safety supervision, and device approval are outside a generic UCON algorithm.
