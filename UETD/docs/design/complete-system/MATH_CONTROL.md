# Numerical and algorithm implementation design

Priority and readiness are separate. Existing FIRST_PASS algorithms remain
first-pass obligations. Undefined plant/profile details block implementation
and generation; they are not silently moved to later scope. More specialized
alternatives retain their actual later priority.

The existing C headers, equation checklist, 35 UMATH module records and UCON
algorithm records are the inventory. This document specifies shared contracts
and family implementation rules. The generated design matrix maps every UCON
identity to a family and its promotion obligations.

The [UCON design matrix](../../audits/UCON_DESIGN_MATRIX_2026-10-29.csv)
contains every current UCON identity; the
[whole implementation matrix](../../audits/UETD_IMPLEMENTATION_DESIGN_MATRIX_2026-10-29.csv)
also covers every other registered module. Scope recommendations in these
inventories do not change implementation or generation readiness.

## Numerical ABI and failure model

One generated image selects scalar/accumulator/fixed-point policy, FPU ABI,
contraction policy and target-math provider consistently. Those choices enter
the ABI/build fingerprint. Public arrays specify element type, shape, stride,
layout and alignment; a scalar-count workspace query cannot be confused with
a byte-count query. Size multiplication is checked before pointer arithmetic.

Each operation declares aliasing, finite/domain requirements, output commit
behavior, conditioning/convergence policy and work/scratch bounds. A checked
status operation cannot return success with an unrepresentable/NaN/infinite
result unless a particular API explicitly permits it. Cast-to-scalar checks
occur after accumulator arithmetic. Multi-output operations either preflight
and commit atomically or explicitly declare partially modified scratch that
must be discarded on failure.

Fix the reproduced Cholesky solve overflow before numerical qualification:
division and narrowing need finite checks in both triangular passes and the
output-on-failure contract must be explicit. Correct the Q31 CORDIC fallback's
positive-one conversion with a specified saturation/rounding rule; zero angle
must not yield `INT32_MIN` as cosine. Hardware and software accelerators use
the same angle/scaling convention and are compared at boundary values.

Fixed point declares Q format, rounding, saturation/overflow, intermediate
width and division-by-zero behavior for each operation. No signed overflow or
out-of-range float-to-integer conversion may be relied on. Saturating helpers
are distinct from checked operations that return an overflow status.

Linear algebra uses solves rather than explicit inverse when solving a system.
Tolerance is relative to matrix scale and numeric precision; a single absolute
epsilon is not a universal rank policy. Covariance/state updates preserve
symmetry/positive-semidefinite assumptions within a documented tolerance and
report failure instead of silently adding arbitrary diagonal noise.

## UMATH module design

| Family and current identities | Required implementation and policy |
|---|---|
| `core`, generic `vector/vector` | Checked scalar operations and contiguous vector kernels; stable norm/distance scaling, zero-vector behavior, exact in-place allowances; resolve duplicate vector source ownership through one implementation owner |
| `fixed` | Q15/Q31/Q16 arithmetic, conversion/sqrt and range/rounding contracts; boundary and exhaustive small-width/reference tests |
| `complex` | Scaled division/magnitude, principal branches for sqrt/log/power, singular/domain handling and finite-result checks |
| `vector2`, `vector3`, `vector4` | Typed geometry, projection/rejection/reflection/angle, robust zero/collinear cases, units and coordinate-frame semantics at bindings |
| `geometry/quaternion`, `geometry/transform3` | Scalar-first Hamilton convention, active local-to-parent rotation, shortest-path interpolation, proper-rotation checks, branch behavior and explicit frame composition |
| `derivatives/dual`, `derivatives/jacobian` | Dual seed/direction, nondifferentiable-point policy, bounded function callbacks; finite differences with scale-aware step, forward/central scheme, exact evaluation count and unchanged-output/failure policy |
| `matrix/core`, `matrix/cholesky` | Products, norms, determinant/inverse/exponential and SPD solves; dimensions/workspace counts, pivot/scale policy, narrow-result checks; exponential declares method, convergence threshold and iteration/scaling ceiling |
| `matrix/lu`, `matrix/ldlt` | Pivoted general and symmetric-indefinite solves; permutation/block-pivot format, singular/rank policy and no unbounded search; do not treat LDLT as SPD-only Cholesky |
| `matrix/qr`, `matrix/svd`, `matrix/symmetric_eigen` | Householder/pivot or declared QR variant; bounded SVD/eigensolver iterations, rank threshold, ordering/sign convention, no-convergence result and reproducible workspace layout |
| `lie/so2`, `lie/so3`, `lie/se2`, `lie/se3` | Exp/log, adjoint and Jacobians; tangent ordering, left/right perturbation convention, near-zero series, near-pi branch and frame/unit contracts |
| `numerical/integration` | Sampled trapezoid/Simpson constraints; fixed-step RK4 and bounded DP5(4) step contract, state/error outputs, maximum substeps/rejections and fail behavior at discontinuities |
| `signal/fft`, `signal/windows` | Explicit transform length/radix, real/complex layout, forward/inverse normalization, table/workspace ownership, window amplitude/energy normalization and valid minimum length |
| `statistics/streaming`, `probability/distributions`, `probability/multivariate_normal` | Streaming mean/variance/covariance, population/sample divisor and reset; quantile method/bound; stable log-sum-exp/softmax, tail accuracy and factor-based Gaussian density; invalid/empty inputs distinguished |
| `optimization/least_squares` | QR/SVD-backed linear/weighted solves, weight positivity, rank/conditioning result, residual/cost and workspace; no normal-equation inversion as a hidden universal implementation |
| `interpolation/splines`, `polynomial/core`, `polynomial/roots` | Coefficient order, knot/multiplicity/extrapolation, derivative continuity and bounded root method; complex roots/order/multiplicity, residual threshold and no-convergence behavior |
| `special/functions`, `random/prng` | Accuracy/domain profiles for erf/log-gamma/I0; named deterministic PRNG algorithm/seed/state, reproducible streams and normal generation; explicitly excluded from security entropy |
| Later `future/matrix/sparse_csr` | CSR canonicality/duplicate indices/ordering, checked row offsets, bounded output capacity and zero policy; add only with an application that needs sparse storage/solvers |

The table covers all current mathematical families. Per-function implementation
uses its actual header, not a duplicate inferred prototype. A module can contain
working direct-C functions while language binding remains draft; record those
two facts separately. A module-derived scaffold label must not be used as a
count of unimplemented function bodies.

Acceptance combines algebraic identities, independent high-precision/reference
values, conditioning/scaling cases, invalid/NaN/infinite inputs, aliasing and
workspace boundaries, and both f32/f64 plus selected wider-accumulator profiles.
Approximation routines publish error envelopes over their supported domain.
Property tests alone cannot prove a derivative, branch or control convention.

## Typed algorithm instance contract

Replace the type-erased scaffold call envelope with per-family typed
configuration, state, input, result and workspace. The envelope remains a
development stub, never the generated firmware ABI.

A proposed instance has immutable validated configuration, caller-owned state,
an explicit workspace size/layout and diagnostic state. Operations are
`validate/configure`, `initialize/reset`, bounded `predict/step/update` as
appropriate, and read diagnostics. Runtime retuning is a validated transaction
with reset/bumpless policy, not arbitrary mutation of a live configuration.

Each activation receives a coherent input snapshot with units/frames, validity,
capture time and sample period. `dt` may be configured constant or a bounded
measured value; an algorithm cannot accept arbitrary jitter by accident. Output
includes value, validity, saturation/constraint status, solver/estimator
diagnostics and the time/sequence to which it applies.

On failure, preserve the previous committed state/output or publish the
explicitly defined invalid/fallback result. A solver's best feasible iterate
is usable only if feasibility and the product's fallback policy are verified;
“ran out of iterations” does not imply a valid command. No algorithm calls a
target peripheral directly. The application/actuator owner applies its output.

Bounds cover state/input/output dimensions, horizon, particles/modes, taps,
solver iterations, callback evaluations, lookup regions and diagnostic history.
Configuration-derived bounds are checked at generation and again at C API
admission where untrusted values can arrive.

## Control and reusable blocks

| Family | State/configuration and algorithm-specific obligations |
|---|---|
| Gain/sum/saturation/deadband/hysteresis/mode/unit-delay | Exact inclusive/exclusive boundaries, initialization, NaN/invalid input, mode selection, units and memory/reset behavior; delay creates an explicit graph timing break |
| Integrator/rate limiter/anti-windup/bumpless transfer | Discretization, state limits, output versus state clamp, signed gain, tracking, external saturation feedback, reset value and mode-change matching |
| PID/lead-lag/IMC/Smith/deadbeat/repetitive/Dahlin | Validated discrete model and sample time, delay representation, integrator/derivative state, actuator limits and tuning; PID deadband/rate limiting are explicit composed blocks unless its typed API is intentionally extended |
| State feedback/LQR/LQI/LQG/decoupler | Fixed matrix dimensions, state/observer conventions, gain source and synthesis evidence; distinguish offline Riccati/pole placement from a bounded runtime matrix multiply; integral augmentation and observability/controllability requirements |
| Gain scheduling/adaptive/MRAC/L1/ILC/self-tuning | Scheduling variable region/interpolation, parameter/rate/projection bounds, initialization, persistence and stability/admissibility assumptions; excitation and estimator-confidence gates; learning/retuning supervision |
| NDI/INDI/INDI2/computed torque/feedback linearization/flatness | Explicit plant/control-effectiveness model, relative degree, invertibility/rank threshold, derivative/actuator dynamics and model-mismatch fallback |
| ADRC/DOB/EDOB/ESO and related disturbance rejection | Distinct augmented state and measured channels, observer bandwidth/discretization, disturbance estimate bound and application to control; avoid treating related observers as aliases |
| Sliding-mode/backstepping/adaptive BS/RISE/IDA-PBC/fuzzy/dynamic surface | Plant assumptions, relative degree, gains, saturation/boundary layer, derivative/filter states and discrete implementation; chatter/noise/uncertainty and actuator bandwidth limitations |
| CLF/CBF/reference governor/control allocation | Definition of constraints and authority, relative degree/model validity, slack/priority, solver feasibility and local safety fallback; optimization infeasibility never bypasses a hardware inhibit |

All current algorithm names map to these families in the design matrix.
Specific gains, plant equations, Lyapunov claims or stability domains cannot be
invented from a catalogue key. FIRST_PASS methods needing such a profile remain
first-pass **profile-blocked** obligations. A published mathematical family is
not a reviewed UETD implementation.

## Filters, signal processing and trajectories

Runtime EMA/moving average/FIR/median/notch/biquad use fixed state/buffer sizes,
coefficient convention, startup/fill behavior, reset, sample period and output
validity. Median implementation declares its worst-case ordering work.
Coefficient designers (Butterworth/Chebyshev/Bessel/elliptic/FIR windowed) are
host/offline by default, with filter order, pass/stop constraints, transform and
coefficient quantization reviewed against an independent response reference.
They are not silently executed in a real-time loop.

LMS/NLMS, Savitzky-Golay, CIC/polyphase, synchronous demodulation, FFT/Goertzel,
spectral peak, RMS/peak and PLL/FLL/NCO/SOGI/DFT work declares gain normalization,
sample/window timing, state, frequency capture/range, phase convention,
aliasing/decimation and transient validity. FFT ownership remains UMATH;
UCON spectral wrappers own estimator/window/time semantics rather than another
FFT implementation.

Trajectory profiles specify position/velocity/acceleration/jerk units, endpoint
conditions, duration/limits, sampling and phase state. Trapezoid/S-curve/jerk and
polynomial/Bézier/B-spline profiles validate feasibility and continuity before
activation. Online retargeting has a continuity/abort contract. Minimum-snap
and time-optimal optimization additionally require a bounded solve and a
feasible fallback; they remain later unless a first-pass project selects them.

## Estimation and identification

| Family | Required state and numerical policy |
|---|---|
| Scalar/linear KF, EKF/iterated EKF | State/covariance, process/measurement dimensions and noise units, analytic or bounded Jacobians, predict/update order, innovation solve/gating, Joseph or other justified covariance update; iterated EKF has an iteration/linearization ceiling |
| UKF/UKF Merwe, CKF | Explicit sigma/cubature point and weight policy, square-root method, covariance validity and negative-weight handling; CKF uses third-degree spherical-radial cubature, not Gauss-Hermite |
| Square-root/UD KF | Factor storage/sign/permutation convention, rank updates/downdates, near-singular failure and equivalence tests against covariance implementations |
| Error-state/invariant/MEKF | Nominal/error-state split, manifold/retraction convention, injection/reset Jacobian, attitude/frame state, covariance transport and small-angle limits |
| IMM/adaptive/fading/robust/Gaussian-sum/dual | Mode count/transition probabilities, mixing/pruning/normalization, adaptation bounds, estimator interdependence and diagnostics; no hidden unbounded mixture growth |
| Delayed/out-of-sequence | Timestamp/domain, fixed replay/history horizon, maximum repropagations and input retention; too-old or unobservable measurement is rejected visibly |
| Particle | Fixed particle count/state, deterministic seeded algorithm, likelihood/log-weight normalization, effective sample size and bounded resampling; invalid all-zero likelihood has a declared recovery |
| Alpha-beta/gamma, Luenberger/high-gain/reduced/UIO/sliding/H-infinity observers | State/measurement model, gain and stability/observability conditions, dt/noise bounds, model mismatch and missing-data policy |
| Attitude complementary/Mahony/Madgwick/TRIAD/QUEST | Sensor axis/frame/calibration, gravity/magnetic assumptions, quaternion convention, gyro bias state, acceleration/magnetic rejection and yaw observability |
| RLS/recursive AR/ARMA/ARX/ARMAX/OE/BJ/IV/RPEM | Model orders/delays, regression/history buffer, parameter/covariance initialization, forgetting/step/projected bounds, excitation/conditioning gate and reset/retuning |
| MHE | Explicit nonlinear/linear model, fixed horizon/window, arrivals/missing observations, constraints, warm start, bounded solver and feasible fallback |

Innovation monitoring/CUSUM and persistent-excitation monitoring are separate
composable diagnostic blocks. They report confidence/quality and do not
silently change actuator authority. Numeric jitter, covariance singularity,
missing/reordered samples, outliers, unobservable modes and frame mistakes
are mandatory adversarial cases.

## Optimization and MPC

At least one bounded dense convex QP implementation is a first-pass dependency
of the existing predictive-control scope. Use a typed active-set reference
profile first; ADMM/projected-gradient and sparse alternatives remain later
choices unless a selected first-pass controller needs them. A solver contract
contains decision/constraint limits, Hessian symmetry/convexity expectations,
scaling, tolerances, maximum iterations, warm-start format and diagnostic result
with primal/dual residuals, cost, feasibility and termination reason.

For linear/tracking/zone/economic/robust/stochastic/adaptive/scheduled/sparse
MPC, define the actual model, horizon, control blocking, constraints, state/
disturbance estimates, terminal policy, cost and robustness/chance assumptions.
Runtime admission validates dimensions and resources. Plant-specific robust or
probabilistic guarantees require their own derivation; they cannot be inferred
from a solver status or a module name.

Finite-set MPC enumerates a fixed admissible action set with an explicit plant
prediction, cost, switching penalties and timing/measurement boundary. A maximum
candidate count is required. Deadbeat MPC needs a defined model/control law and
input/actuator constraints before implementation; retain the documented
priority conflict as a specification decision instead of inventing equations.

Explicit MPC/ENMPC tables identify the offline model, domain, units, regions,
interpolation/lookup, validity/fallback and content hash. Table lookup has a
bounded worst-case region search. Online NMPC RTI/iLQR/DDP/MHE defines model and
derivative callbacks, integration, linearization/regularization, bounded
iterations/line search, warm start and constraint feasibility. ENMPC and RTI
remain separate algorithms; a project hybrid explicitly chooses or blends
them with a validity/time budget and state-transfer contract.

Acceptance: independent small optimization references, infeasible/unbounded or
nonconvex inputs, bad warm starts, deadline exhaustion, feasible-but-suboptimal
results, fixed memory, and direct/generated output equivalence. Solver runtime
measurements are tied to exact dimensions, compiler and target.

## Electrical, aerospace and application profiles

Electrical transforms declare phase/axis/sign/scaling. FOC/SVPWM/DTC/MTPA/flux
weakening/sensorless observers require motor parameters, angle/frame source,
voltage/current limits, sample/PWM synchronization, deadtime effects and local
hardware protection. Grid resonant/PLL/droop families require frequency range,
phase convention, balanced/unbalanced/harmonic assumptions and island/fault
behavior. Converter methods require topology, sample/switch boundary and
admissible actions; the deferred half-cycle method remains undefined until
those facts exist.

Aerospace guidance/attitude/navigation profiles declare vehicle/frame, control
authority, actuator allocation, valid speed/dynamic region, reference path and
sample/sensor assumptions. INS/GNSS loose/tight coupling and GNSS DLL/PLL/FLL
have distinct measurement/code/carrier/time contracts. A generic trajectory or
Kalman library does not by itself implement a complete navigation/autopilot.

Medical catalogue entries stay profile-blocked until the physiological model,
state/measurement definition, parameter units, admissible domain, clinical
requirements and validation are supplied. Reusable numerical/estimation pieces
can be implemented independently. No medical safety or suitability is created
by naming a glucose, ventilator, or PK/PD controller.

The deferred LLC resonant EKF requires its actual plant and observability model.
Fractional-order sliding control requires finite-memory approximation and
error/domain bounds. These genuinely underspecified/later methods remain in
later scope. A deep design must identify missing information rather than
manufacture a plausible but unreviewed control law.
