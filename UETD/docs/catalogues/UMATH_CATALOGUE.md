# UMATH mathematical systems catalogue

UMATH is UETD’s shared, allocation-conscious numerical layer. UCORE supplies primitive storage types; `umath/config.h` owns the numerical scalar and accumulator aliases. CMake selects the public scalar with `UETD_UMATH_SCALAR_TYPE` (`uetd_f32_t` by default or `uetd_f64_t`) and the intermediate type with `UETD_UMATH_ACCUMULATOR_TYPE` (`SCALAR` by default, or either scalar alias). `UETD::Core` propagates both choices to consumers: the scalar setting affects ABI, while the accumulator setting controls intermediate precision and runtime cost. Arithmetic and libm calls use the configured accumulator precision; widening intermediates can improve robustness at the cost of double-precision execution on targets without suitable hardware. Scalar-typed helpers live in `umath/scalar.h`; the `uetd_*f` helpers in UCORE remain explicitly 32-bit. Interfaces are designed around caller-owned storage and bounded work.

The registry contains 34 UMATH modules: 11 implemented/selectable modules, 19 Phase 0 priority scaffolds, 3 Phase 1 priority scaffolds, and 1 later scaffold. A phase records implementation priority, not implementation status. Modules remain `scaffold_only` and `available_for_generation: false` until their typed contracts are implemented and reviewed. The function registry contains 328 public declarations. Of 124 coverage functions added by the audit, 119 now have implementations and five remain fail-closed: LU determinant, streaming standard deviation/RMS, and probability softmax/log-softmax. Direct-C coverage does not change module generation status. See the [header and source coverage audit](../audits/UMATH_CATALOGUE_GAP_AUDIT.md) for the per-file inventory.

## Floating-point target policy

`UETD_UMATH_ALLOW_FP_CONTRACTION` defaults to `OFF`, so ordinary expressions such as `a * b + c` keep separate-operation rounding in supported compiler configurations. `umath_fma` explicitly performs a fused multiply-add at accumulator precision and then converts to the configured scalar type. The internal `scalar_math.h` dispatch selects `fmaf` or `fma` according to the accumulator type; a target may provide `UETD_UMATH_TARGET_SCALAR_MATH_HEADER` to replace a dispatch macro with a verified intrinsic or library routine that preserves the same semantics.

The board toolchain remains responsible for exact CPU/FPU selection and ABI flags (for example, the compiler's CPU, FPU, and float-ABI options). `UETD_TARGET=CORTEX_M` alone does not identify a particular core or imply that its FPU is enabled. Firmware startup and RTOS configuration must enable the FPU and preserve its state where required. Avoid fast-math modes when relying on UMath's finite-value checks; target libm accuracy and subnormal behavior should be considered for each supported profile.

## Implemented and selectable modules

| System | Module and public API | Current coverage |
|---|---|---|
| Scalar operations | `umath/core`, `umath/scalar.h` | Checked arithmetic and powers; trigonometric, inverse-trigonometric, hyperbolic, inverse-hyperbolic, logarithmic, exponential, and cube-root functions; floor/ceil/trunc/round, `fmod`, `remainder`, `modf`, and `copysign`; stable sinc-family functions; hypot, fused multiply-add, clamp, and radians wrapping. |
| Fixed-point arithmetic | `umath/fixed`, `umath/fixed.h` | Saturating add/subtract/multiply/divide/absolute value, floor-rounded square root, and checked conversion for Q1.15, Q1.31, and Q15.16. |
| Dynamic-length vectors | `umath/core`, `umath/vector/vector.h` | Elementwise add/subtract/multiply/divide/scale, clamp, dot product, Euclidean norm, normalization, distance, and interpolation over caller-owned arrays. |
| Small vectors | `umath/vector/vector2`, `umath/vector/vector3`, `umath/vector/vector4` | Add/subtract/scale, componentwise multiply/divide, dot products, 2D/3D cross products, norms, normalization, distance, interpolation, angles, projection, rejection, and reflection. |
| Complex scalars | `umath/complex`, `umath/complex.h` | Checked arithmetic, magnitude, principal square root/log/power, polar conversion, and circular/hyperbolic functions. |
| Dense matrices | `umath/matrix/core`, `umath/matrix/core.h` | Row-major products, transpose, add/subtract/scale, symmetrization, constructors, elementwise/outer products, rank-one update, trace/norms, determinant, inverse, and scaling-and-squaring matrix exponential. Determinant, inverse, and exponential expose workspace-count queries returning $n^2$, $2n^2$, and $3n^2$ accumulator elements respectively; callers provide all storage. |
| SPD solve | `umath/matrix/cholesky`, `umath/matrix/cholesky.h` | In-place lower Cholesky factorization, forward/back substitution, log-determinant, and inverse. |
| 3D rotations | `umath/geometry/quaternion`, `umath/geometry/quaternion.h` | Scalar-first Hamilton arithmetic, dot/norm, normalization, vector rotation, axis-angle and matrix conversion, shortest-path slerp, and quaternion exp/log. |
| Rigid frame geometry | `umath/geometry/transform3`, `umath/geometry/transform3.h` | Identity, composition/inversion, point transformation, homogeneous-matrix conversion, and interpolation with named frames. |
| Polynomial calculus | `umath/polynomial/core`, `umath/polynomial/core.h` | Descending-order evaluation, derivative/antiderivative coefficient construction, definite integration, addition, multiplication, and composition. |

These direct-C implementations remain inside modules whose generation contracts are still under review.

| Module | Scaffold APIs now declared |
|---|---|
| `umath/matrix/lu` | Determinant from a pivoted LU factor and permutation sign. |
| `umath/statistics/streaming` | Standard deviation and RMS reductions. |
| `umath/probability/distributions` | Softmax and log-softmax. |

The audit added 124 coverage functions; 119 are implemented and the five shown
above remain fail-closed. The dual source implementations are complete for its
registered functions, though the module itself remains unselectable pending
contract and binding review.

The `umath/umath.h` umbrella contains only implemented/selectable modules. Scaffold headers are included explicitly by their module path and do not silently become part of the stable umbrella.

## Phase 0 priority scaffolds

These systems support the projects and algorithm families that are expected to need mathematical foundations first. Most rows are fail-closed outlines; dual numbers have a direct-C implementation but remain blocked from generation pending module contract review. Every row still requires contract completion and numerical review before promotion.

| System | Module | Why it is in the first implementation queue |
|---|---|---|
| Forward automatic differentiation | `umath/derivatives/dual` | Checked direct-C implementations for the registered dual operators and elementary/nonsmooth functions; generation remains blocked pending its complete typed contract and derivative-policy review. |
| Finite-difference Jacobians | `umath/derivatives/jacobian` | Nonlinear estimators linearize process and measurement models; a caller-owned, bounded Jacobian routine supports those contracts when analytic derivatives are unavailable. |
| Quadrature and ODE steps | `umath/numerical/integration` | Sampled integration supports control and trajectory calculations; bounded RK4/Dormand–Prince steps support nonlinear model propagation. UCON still owns each controller’s state, timing, and anti-windup semantics. |
| FFT | `umath/signal/fft` | Supports spectral PLL designs, frequency acquisition/estimation, harmonic analysis, and diagnostics. Some PLL architectures use time-domain phase detectors, while the selected NEXUS designs need transform support. |
| Signal windows | `umath/signal/windows` | Hann/Hamming/Blackman/Kaiser window outlines support FFT-based acquisition and spectral estimates by controlling leakage. |
| SO(3) operations | `umath/lie/so3` | Exponential/logarithm, tangent maps, and left/right Jacobians support 3D attitude and invariant/error-state estimation, including the planned InEKF. |
| SE(3) operations | `umath/lie/se3` | Pose exponential/logarithm, adjoints, and Jacobians support rigid-body state propagation and correction. |
| Pivoted LU | `umath/matrix/lu` | General square solves for systems that are not symmetric positive definite. |
| Symmetric-indefinite LDLᵀ | `umath/matrix/ldlt` | Handles symmetric systems that cannot use Cholesky, with pivot policy and singularity behavior still to define. |
| QR factorization | `umath/matrix/qr` | Stable least-squares and rank-aware solve foundation for estimators, identification, and MPC. |
| SVD and pseudoinverse solve | `umath/matrix/svd` | Handles rank deficiency and poorly conditioned least-squares systems that appear in model fitting and constrained-control support. |
| Symmetric eigensystem | `umath/matrix/symmetric_eigen` | Supports covariance conditioning/inspection, principal directions, and numerically robust estimator diagnostics. |
| Streaming statistics | `umath/statistics/streaming` | Online mean, variance, covariance, and quantile outlines support residual monitoring and signal/estimator diagnostics without retaining the full sample history. |
| Probability distributions | `umath/probability/distributions` | Stable log-sum-exp, normal log-density/CDF, and chi-square CDF support innovation scoring, gating, and likelihood calculations. |
| Multivariate normal | `umath/probability/multivariate_normal` | Gaussian log-density from a caller-supplied Cholesky factor supports multivariate estimator likelihoods and diagnostics. |
| Least squares | `umath/optimization/least_squares` | Linear and weighted least-squares outlines support identification and optimization; conditioning/rank policy depends on QR/SVD. |
| Splines | `umath/interpolation/splines` | Hermite, Bézier, and B-spline evaluation supports smooth references and trajectory interpolation. |
| Polynomial roots | `umath/polynomial/roots` | Real/complex root outlines support trajectory constraints, pole calculations, and model analysis. |
| Special functions | `umath/special/functions` | Error function, log-gamma, and modified Bessel I₀ support probability tails and spectral/model-fitting work. |

## Phase 1 priority scaffolds

These remain in the catalogue and have their own module records, but are behind the first shared numerical foundation.

| System | Module | Intended scope |
|---|---|---|
| SO(2) | `umath/lie/so2` | Planar rotation exponential/logarithm. |
| SE(2) | `umath/lie/se2` | Planar rigid-pose exponential/logarithm and adjoint operations. |
| Deterministic random numbers | `umath/random/prng` | Seeded integer/uniform/normal streams for simulation and sampling; never cryptography. |

## Later catalogue scaffold

| System | Module | Intended scope |
|---|---|---|
| Sparse linear algebra | `umath/future/matrix/sparse_csr` | CSR validation, sparse matrix-vector product, and sparse addition with caller-owned arrays. |

## How UMATH supports UCON and application work

| Application family | Useful UMATH systems |
|---|---|
| EKF, ES-EKF, UKF, CKF, invariant filters, and information filters | Matrix kernels and factorizations, Cholesky, Jacobians/autodiff, probability/statistics, and SO(3)/SE(3) for manifold state representations. |
| Particle and Gaussian-mixture estimators | Probability, stable log-sum-exp, statistics, Cholesky whitening, deterministic random sampling, and matrix/geometry operations. |
| MPC, LQR/LQG, constrained control, and online system identification | Matrix products/solves, QR/SVD and least squares; UCON owns controller and constraint semantics. |
| PLL/FLL, resonant/grid control, spectral estimation, and adaptive signal processing | Complex arithmetic, trigonometry, FFT/windows for spectral paths, integration, and statistics. |
| Trajectory generation and motion planning | Polynomial evaluation/roots, Hermite/Bézier/B-spline interpolation, numerical integration, and least-squares support. |
| Medical and other application-specific estimators | Reusable matrix, probability, statistics, integration, and differentiation primitives; domain models and safety validation remain application-specific. |

## Module and generation contract

The canonical module index is `registry/modules/registry.json`; each entry points to a detail file under `registry/modules/manifest/`. Each module record declares its source list, transitive public-header closure, dependencies, phase, and generation status. `uef-gen` resolves dependency closure and copies only selected module files. All Phase 0/1 scaffolds are blocked from generation until implementation; only the true later scaffold remains under `src/umath/future/`. The stable math umbrella likewise excludes all scaffold modules.

The normal CMake source collection includes Phase 0/1 outline sources, which provide linkable fail-closed symbols returning `UMATH_NOT_IMPLEMENTED`. Of the 124 new coverage APIs alongside existing module code, five still return `UMATH_NOT_IMPLEMENTED`; the other 119 have implementations but do not change module generation status. The `UETD_BUILD_FUTURE_UMATH_SCAFFOLDS` option is only for compiling the later outline under `future/`. Compiling a scaffold does not establish usable behavior.

Before promotion, define scalar precision/range, dimensions and workspace, aliasing, convergence and singularity behavior, deterministic work bounds, and target accuracy. Implement each method and compare it with trusted reference results on supported host and target toolchains before marking it selectable.
