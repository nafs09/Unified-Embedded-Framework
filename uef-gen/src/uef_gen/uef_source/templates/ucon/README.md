# UCON template catalogue

UCON is intended to be held by UEF as part of its control and estimation library. `uef-gen` will select and assemble the UEF-owned algorithms/assets needed by each firmware project. The current supplied UEF specification still describes the older generator-owned boundary; reconcile it with this project decision before finalizing the handoff. The specification gives a family tree and selected substitution points, but not complete algorithm source bodies. Those bodies must come from reviewed algorithm designs; this catalogue does not invent them.

| Planned folder | Families named in V1.1 | Example dimensions/slots named in the spec |
|---|---|---|
| `numerics` | matrix-vector/matrix-matrix, Cholesky, Cholesky solve, QR, active-set QP, vector clamp | `N`, `M`, `K` dimensions |
| `control` | PID, state feedback, LQR, lead-lag, Smith predictor, IMC, deadbeat, two-degree-of-freedom PID | integral/derivative update, anti-windup, output law, K matrix, plant/delay model |
| `estimation` | EKF, UKF, linear Kalman, square-root UKF, IMM-EKF, Mahony, Madgwick, complementary, Luenberger, disturbance observer | prediction/Jacobian/covariance/gating and model matrices |
| `predictive` | linear MPC, explicit MPC, finite-set MPC, nonlinear MPC RTI/SQP, iLQR | prediction matrices, QP solve, candidate set, cost, preparation/feedback steps |
| `filtering` | Butterworth, Bessel, Chebyshev, elliptic, notch, resonant PR, moving average, median, FIR, biquad, alpha-beta, alpha-beta-gamma | coefficients, order, taps, window length, notch frequency/Q |
| `nonlinear` | sliding-mode, super-twisting, NDI, INDI, backstepping, CLF-CBF | sliding surface, control law, plant inverse, virtual controls, constraints |
| `adaptive` | Lyapunov MRAC, L1 adaptive, gain scheduling, ADRC | reference model, adaptation law, schedule table, observer bandwidth |
| `trajectory` | first/second-order reference model, jerk profile, polynomial trajectory | fixed-size trajectory parameters |
| `infrastructure` | saturation, rate limiter, deadband, hysteresis, anti-windup variants, bumpless transfer, mode select | project tuning and state-transition settings |

The spec also names root templates for `config.h`, ISR/task execution, HAL stubs, and a UPAL-backed HAL implementation. The family paths above are the intended organization; those subdirectories and algorithm bodies have not yet been checked in. This folder currently contains only this catalogue and its TODO list. Until a reviewed body, metadata, and generation path exist, a family is a planned catalogue entry and must not be advertised as generator-supported.

## Template implementation rules

- Keep all generated buffers and state statically sized; generated UCON has no RTOS API or heap dependency.
- Specialize dimensions at generation time and emit compile-time shape checks.
- Preserve the selected arithmetic type and overflow/saturation behavior throughout each generated algorithm.
- Keep hardware access in generated HAL bindings backed by selected UPAL modules.
- Include source/design provenance with each template so a generated controller can be reviewed and reproduced.
- Do not claim a family is available to `uef-gen` until a reviewed body, metadata, and generation path exist.
