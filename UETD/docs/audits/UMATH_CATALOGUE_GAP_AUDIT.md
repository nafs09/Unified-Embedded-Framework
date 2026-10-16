# UMATH header and source coverage audit

**Audit date:** 2026-10-13  
**Scope:** all 39 headers under `include/umath`, all 36 C files under `src/umath`, all 34 registered UMATH modules, and their UMATH equation checklist and registries.

This is a static inventory of declared APIs, source definitions, fail-closed
stubs, and common mathematical operations missing from otherwise implemented
modules. It is not a claim that the selected equations are numerically
qualified. The calculation checklist remains the per-equation source review.

## Findings addressed in this change

- Added checked scalar negation, add, subtract, multiply, divide, reciprocal, integer
  power, real power, inverse trigonometric functions, and hyperbolic
  functions. Their checked outputs are committed only after range validation.
- Added generic contiguous-vector add, subtract, multiply, divide, and scale, with preflight
  checks so a failure leaves output unchanged.
- Added saturating Q15, Q31, and Q16 division. A zero denominator reports
  `UMATH_SINGULAR` and preserves the caller's output.
- Expanded forward dual numbers with constructors, negation, reciprocal,
  integer and dual powers, inverse trigonometric functions, `atan2`, inverse
  square root, hyperbolic functions, `hypot`, and fused multiply-add.
- Completed the pre-existing complex, 2D/3D/4D vector, quaternion, dense-matrix core, and polynomial implementations, adding principal complex functions, vector geometry, quaternion conversion/interpolation/exp/log, matrix norms/determinant/inverse/exponential, and polynomial coefficient/algebra operations.
- Reformatted all UMATH headers to the compact declaration style used by the original API headers and replaced stale scaffold comments where operations are now implemented.
- Updated the UMATH registry and equation checklist for those APIs. Corrected
  the checklist's incorrect claim that generic vector arithmetic was already
  present, and corrected the catalogue's claim that all scaffold symbols
  return `UMATH_NOT_IMPLEMENTED`.

## Additional coverage APIs added

The coverage pass added 124 public function declarations with matching C definitions. Of those, 119 now have implementations: 60 in the original six eligible sources and 59 in the pre-existing complex, typed-vector, quaternion, matrix-core, and polynomial sources. Five remain fail-closed in less-complete modules: LU determinant, streaming standard deviation/RMS, and probability softmax/log-softmax. Module generation remains blocked until each module's typed contract is complete and reviewed.

The cutoff was calculated from each source's registered public C functions
before implementation: `scalar.c` 28/48, `fixed.c` 18/30, generic `vector.c`
8/11, `derivatives/dual.c` 24/44, `geometry/transform3.c` 4/7, and
`matrix/cholesky.c` 2/4. Later work completed the pre-existing, substantially implemented complex, vector, quaternion, matrix-core, and polynomial files as well.

| Header/module | Newly declared families | Current state |
|---|---|
| `scalar.h` / `umath/core` | `log1p`, `expm1`, `log2`, `log10`, `exp2`, `cbrt`, `sinc`/`cosc`/`sincc`, rounding, `fmod`, `remainder`, `modf`, `copysign`, and inverse hyperbolic functions. | Implemented |
| `vector/vector.h` / `umath/core` | Arbitrary-length normalization, distance, and interpolation. | Implemented |
| `fixed.h` / `umath/fixed` | Q15/Q31/Q16 absolute value, square root, and all six pairwise format conversions. | Implemented |
| `vector/vector2.h`, `vector3.h`, `vector4.h` | Componentwise multiply/divide, distance/interpolation, projection/rejection/reflection, and signed 2D or unsigned 3D/4D angle helpers. | Implemented |
| `complex.h` / `umath/complex` | Principal square root/power, argument/polar conversion, circular functions, and hyperbolic functions. | Implemented |
| `matrix/core.h` / `umath/matrix/core` | Identity/diagonal construction, elementwise and outer products, rank-one update, trace, Frobenius/one/infinity norms, determinant, inverse, and scaling-and-squaring matrix exponential. Determinant, inverse, and exponential report caller-workspace sizes of $n^2$, $2n^2$, and $3n^2$ accumulator elements. | Implemented |
| `matrix/cholesky.h` / `umath/matrix/cholesky` | Factor-based log-determinant and inverse. | Implemented |
| `matrix/lu.h` / `umath/matrix/lu` | Determinant from a pivoted LU factor and permutation sign. | Fail-closed scaffold |
| `geometry/quaternion.h` / `umath/geometry/quaternion` | Dot/norm, rotation-matrix conversion, slerp, quaternion exponential and logarithm. | Implemented |
| `geometry/transform3.h` / `umath/geometry/transform3` | Homogeneous-matrix conversion and rigid-transform interpolation. | Implemented |
| `polynomial/core.h` / `umath/polynomial/core` | Coefficient derivative/antiderivative, addition, multiplication, and composition. | Implemented |
| `statistics/streaming.h` / `umath/statistics/streaming` | Standard-deviation and RMS reductions. | Fail-closed scaffolds |
| `probability/distributions.h` / `umath/probability/distributions` | Stable softmax and log-softmax. | Fail-closed scaffolds |
| `derivatives/dual.h` / `umath/derivatives/dual` | Additional elementary functions, rounding/remainder, absolute value, min/max, clamp, and angle wrapping. | Implemented in C; module generation remains blocked |

## Module-by-module inventory

| Module | Header and source | State and coverage gap |
|---|---|---|
| `umath/core` | `scalar.h`, `vector/vector.h`, `status.h`; `status.c`, `scalar.c`, `vector/vector.c` | Implemented checked scalar extensions, stable sinc-family functions, and generic-vector normalize/distance/interpolation. |
| `umath/fixed` | `fixed.h`; `fixed.c` | Implemented arithmetic, saturating absolute value/square root, and all pairwise Q-format conversions. |
| `umath/vector/vector2` | `vector/vector2.h`; `vector/vector2.c` | Implemented vector arithmetic and checked componentwise operations, distance/interpolation, projection/rejection/reflection, and signed angle. |
| `umath/vector/vector3` | `vector/vector3.h`; `vector/vector3.c` | Implemented vector arithmetic and checked componentwise operations, distance/interpolation, projection/rejection/reflection, and unsigned angle. |
| `umath/vector/vector4` | `vector/vector4.h`; `vector/vector4.c` | Implemented vector arithmetic and checked componentwise operations, distance/interpolation, projection/rejection/reflection, and unsigned angle. |
| `umath/complex` | `complex.h`; `complex.c` | Implemented arithmetic, magnitude, exponential/logarithm, principal square root/power, polar conversion, and complex circular/hyperbolic functions. |
| `umath/matrix/core` | `matrix/core.h`; `matrix/core.c` | Implemented dense kernels, constructors, elementwise/outer product, rank-one update, trace/norms, determinant, inverse, and scaling-and-squaring matrix exponential with queried caller-owned workspace. |
| `umath/matrix/cholesky` | `matrix/cholesky.h`; `matrix/cholesky.c` | Implemented factorization, solve, factor-based log-determinant, and inverse. |
| `umath/geometry/quaternion` | `geometry/quaternion.h`; `geometry/quaternion.c` | Implemented rotation operations, dot/norm, proper-rotation matrix conversion, shortest-path slerp, and principal quaternion exp/log. |
| `umath/geometry/transform3` | `geometry/transform3.h`; `geometry/transform3.c` | Implemented rigid-transform operations, homogeneous-matrix conversion, and interpolation. |
| `umath/polynomial/core` | `polynomial/core.h`; `polynomial/core.c` | Implemented evaluation, coefficient derivative/antiderivative, definite integration, addition, multiplication, and composition. |
| `umath/derivatives/dual` | `derivatives/dual.h`; `derivatives/dual.c` | Direct-C implementations for its registered operators and elementary/nonsmooth extensions. It remains Phase 0 and blocked from generation pending complete bindings and contract review. |
| `umath/derivatives/jacobian` | `derivatives/jacobian.h`; `derivatives/jacobian.c` | Fail-closed scaffold. Both workspace sizing and forward/central finite-difference Jacobian evaluation return `UMATH_NOT_IMPLEMENTED`. |
| `umath/numerical/integration` | `numerical/integration.h`; `numerical/integration.c` | Fail-closed scaffold. Trapezoid, Simpson, RK4, and Dormand–Prince 5(4) are all declared but return `UMATH_NOT_IMPLEMENTED`; the current checklist correctly records candidate equations rather than completed work. |
| `umath/signal/fft` | `signal/fft.h`; `signal/fft.c` | Fail-closed scaffold. Plan initialization, complex forward/inverse execution, and real forward transform are declared but unimplemented. |
| `umath/signal/windows` | `signal/windows.h`; `signal/windows.c` | Fail-closed scaffold. Hann, Hamming, Blackman, and Kaiser coefficients are declared but unimplemented. |
| `umath/lie/so2` | `lie/so2.h`; `lie/so2.c` | Fail-closed scaffold. SO(2) exponential and principal logarithm are declared but unimplemented. |
| `umath/lie/so3` | `lie/so3.h`; `lie/so3.c` | Fail-closed scaffold. Hat/vee, exponential/logarithm, and left/right Jacobians and inverses are declared but unimplemented. |
| `umath/lie/se2` | `lie/se2.h`; `lie/se2.c` | Fail-closed scaffold. Exponential, logarithm, and adjoint are declared but unimplemented. |
| `umath/lie/se3` | `lie/se3.h`; `lie/se3.c` | Fail-closed scaffold. Exponential, logarithm, adjoint, and left Jacobian/inverse are declared but unimplemented. |
| `umath/matrix/lu` | `matrix/lu.h`; `matrix/lu.c` | Fail-closed factorization/solve plus a fail-closed determinant-from-factor scaffold. |
| `umath/matrix/ldlt` | `matrix/ldlt.h`; `matrix/ldlt.c` | Fail-closed scaffold. Pivoted factorization and solve are declared but unimplemented. |
| `umath/matrix/qr` | `matrix/qr.h`; `matrix/qr.c` | Fail-closed scaffold. Householder factorization and least-squares solve are declared but unimplemented. |
| `umath/matrix/svd` | `matrix/svd.h`; `matrix/svd.c` | Fail-closed scaffold. SVD and pseudoinverse solve are declared but unimplemented. |
| `umath/matrix/symmetric_eigen` | `matrix/symmetric_eigen.h`; `matrix/symmetric_eigen.c` | Fail-closed scaffold. Symmetric eigensystem is declared but unimplemented. |
| `umath/statistics/streaming` | `statistics/streaming.h`; `statistics/streaming.c` | Fail-closed scaffold. Mean, variance, covariance, quantile, standard deviation, and RMS are declared but unimplemented. |
| `umath/probability/distributions` | `probability/distributions.h`; `probability/distributions.c` | Fail-closed scaffold. Log-sum-exp, softmax/log-softmax, normal log-PDF/CDF, and chi-square CDF are declared but unimplemented. |
| `umath/probability/multivariate_normal` | `probability/multivariate_normal.h`; `probability/multivariate_normal.c` | Fail-closed scaffold. Cholesky-based multivariate normal log-PDF is declared but unimplemented. |
| `umath/optimization/least_squares` | `optimization/least_squares.h`; `optimization/least_squares.c` | Fail-closed scaffold. Least squares, weighted least squares, and residual norm are declared but unimplemented. |
| `umath/interpolation/splines` | `interpolation/splines.h`; `interpolation/splines.c` | Fail-closed scaffold. Cubic Hermite, Bézier, B-spline basis, and B-spline derivatives are declared but unimplemented. |
| `umath/polynomial/roots` | `polynomial/roots.h`; `polynomial/roots.c` | Fail-closed scaffold. Real and complex root solvers are declared but unimplemented. |
| `umath/special/functions` | `special/functions.h`; `special/functions.c` | Fail-closed scaffold. Error function, log-gamma, and modified Bessel I0 are declared but unimplemented. |
| `umath/random/prng` | `random/prng.h`; `random/prng.c` | Fail-closed scaffold. Seeding, U32, uniform, and normal stream operations are declared but unimplemented. |
| `umath/future/matrix/sparse_csr` | `future/matrix/sparse_csr.h`; `future/matrix/sparse_csr.c` | Later fail-closed scaffold. CSR validation, matvec, and addition are declared but unimplemented. |

## Shared headers and umbrella

- `config.h` defines the configured scalar and accumulator types; it has no
  operation declarations.
- `internal/scalar_math.h` dispatches C11 libm calls to the configured
  accumulator type; it is an implementation helper, not a public numerical
  API.
- `umath.h` intentionally exports only implemented/selectable modules. The
  partial dual module and the other scaffold headers must be included by their
  module path.
- `status.h` and `status.c` define the common status values and printable
  status names; both are accounted for under `umath/core`.

## UETD algorithm construct coverage

The cross-check used the algorithm families and candidate equations in
`UETD_EQUATION_CHECKLIST.md` as the project-specific baseline. This is a
coverage map, not a claim that every listed algorithm is implemented.

| UETD family | Mathematical constructs mapped to UMATH/UCON | Current boundary |
|---|---|---|
| Feedback, state-space control, and nonlinear control | Scalar/vector/matrix kernels, norms, factorization/least-squares scaffolds; UCON keeps controller laws, constraints, and plant models. | CLF/CBF and MPC quadratic programs are already represented by future UCON QP solver scaffolds; UMATH has no shared QP solver API. |
| Kalman, sigma-point, particle, GNSS/INS, and adaptive estimation | Matrix products/solves, rank-one covariance updates, Cholesky/QR/SVD/eigen, Jacobians/dual numbers, probability, statistics, random streams, vector and rotation geometry. | The primitive families are catalogued; several are still fail-closed. Earth geodesy and sensor models remain UCON/domain contracts. |
| Attitude, guidance, and rigid-body algorithms | Fixed-size vector geometry (including projection/reflection), quaternion operations, SO(3)/SE(3), and rigid transforms. | Vector-angle/projection/reflection, quaternion matrix conversion/slerp/exp/log, and transform matrix conversion/interpolation are implemented; Lie modules remain fail-closed. |
| Signal processing and electrical control | Scalar trigonometry, stable small-angle kernels, complex arithmetic, FFT/windows, statistics. | FFT/window modules remain fail-closed; Clarke/Park, PLL, modulation, and motor equations remain UCON algorithm modules. |
| Trajectories and numerical models | Polynomial calculus/roots, splines, integration, interpolation, matrix exponential for linear continuous-time discretization. | Polynomial coefficient operations and a matrix exponential with fixed iteration limits and caller-sized workspace are implemented; numerical integration, roots, and splines remain fail-closed. |
| Optimization and identification | QR/SVD/least squares, matrix solves, rank/conditioning decisions, and QP formulations. | Least-squares and factorization operations are scaffolded; shared QP solving is not in UMATH and remains an explicit UCON roadmap family. |

The project checklist also contains fractional-order control, geodetic
conversion, and application-specific models. Those require state/history,
coordinate, or plant contracts, so the coverage map leaves them in UCON rather
than pretending they are scalar/matrix library functions.

## Static consistency and remaining coverage gaps

Static matching found 328 unique public function declarations and 328 UMATH
registry records; each declaration has a matching source definition or a
header-inline definition. The 22 fully scaffolded C sources remain fail-closed.
Of the 124 added coverage functions, 119 are implemented and five remain
fail-closed: LU determinant, standard deviation/RMS, and softmax/log-softmax.
Across the full registry, 70 APIs remain `scaffold` and three remain `future`.
The unchecked equations and unresolved policies are recorded in
`UETD_EQUATION_CHECKLIST.md`.

The broader construct pass also catalogues covariance rank-one primitives,
vector-angle operations, RMS/standard-deviation reductions, softmax functions,
and a general matrix exponential after comparing the repository equation
checklist with the UMATH API. The rank-one update, vector-angle operations, and
matrix exponential now have checked implementations; the five reductions and
probability APIs remain fail-closed.

The largest remaining implementation gaps are the 22 fully fail-closed module
sources and the five added coverage functions that remain scaffolded. Their
candidate equations and unresolved numerical policies are recorded in the
checklist; they remain non-selectable until implemented and reviewed.
