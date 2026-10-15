# UETD equation and algorithm calculation-step checklist

Use the file index to jump to the relevant equation section, then check each
equation or algorithm item in place as you review it for that file. Items
shared by several files name those files in the index. A checked item means
the current source contains the corresponding calculation or algorithm; it is
a static source audit, not a correctness proof, test result, or claim that the
module is generator-selectable. Unchecked items are candidates, contract
decisions, or work not implemented in the current source.

## How to use this checklist

- **SPEC** means the equation is stated in `Specificaitons/UETD_Specification.md`
  V1.2 and should match it unless that specification is revised.
- **Standard candidate** means a conventional formulation useful for writing
  the typed contract. Freeze dimensions, conventions, discretization, and
  numerical policy before implementation; it is not silently normative UETD
  behavior.
- **Contract blocker** means the repository explicitly lacks enough model or
  policy detail to write a unique equation. Resolve that first; do not fill the
  gap with a guessed plant, sensor, medical, or safety model.
- `src/...` is the implementation path. The matching
  `include/...` header owns the public types and contract. Each module's
  `registry/modules/manifest/...json` owns dependency and generation status.

### Quick navigation

- [UMATH equation checklists](#umath-equation-sets)
- [Dual-number operator checklist](#automatic-differentiation-dual-numbers)
- [Finite-difference Jacobian checklist](#finite-difference-jacobians)
- [UCON equation checklists](#ucon-equation-sets)
- [Other UETD equation checklists](#other-uetd-equations-and-algorithms)
- [File-by-file index](#file-by-file-index)

## Shared implementation and promotion checklist

These are implementation/promotion gates, separate from the equation-review
checkboxes within the equation sections.

For each numerical or algorithm module, check all applicable items:

- [ ] Write the state definition and update equations, including reset and
  initialization.
- [ ] Fix units, coordinate frames, sign/axis conventions, sample timing, and
  discretization method.
- [ ] Define dimensions, workspace, aliasing, and whether outputs/state remain
  unchanged on failure.
- [ ] Specify saturation, constraints, singularity/rank/convergence thresholds,
  finite-value checks, and deterministic work limits.
- [ ] Define scalar/accumulator precision and all narrowing/rounding points.
- [ ] Confirm caller-owned storage, ISR/task context, and state ownership where
  applicable.
- [ ] Compare the direct C implementation with generated output; keep
  scaffolds fail-closed until both use the same typed contract.

## UMATH equation sets

These are reusable numerical primitives. UCON owns controller/estimator state,
model assumptions, sample timing, and safety policy.

### Scalar arithmetic

#### Scalar helpers — current API

File: [src/umath/scalar.c](../../src/umath/scalar.c), [include/umath/scalar.h](../../include/umath/scalar.h). Items checked here match the current checked scalar API and inline helpers.

- [x] **Clamp.** Validate finite $x,a,b$ and $a\le b$; compute $y=\min(b,\max(a,x))$; leave output unchanged on invalid input.
- [x] **Wrap radians to $[-\pi,\pi)$.** Compute $r=\operatorname{fmod}(x,2\pi)$; if $r\ge\pi$, set $r\leftarrow r-2\pi$; else if $r< -\pi$, set $r\leftarrow r+2\pi$; return $r$.
- [x] **Sine/cosine/tangent.** For finite radians $x$, compute the corresponding library result $\sin x$, $\cos x$, or $\tan x$ in accumulator precision; narrow only if the result remains finite in configured scalar precision.
- [x] **Two-argument arctangent.** For finite $x,y$, compute $\theta=\operatorname{atan2}(y,x)$; preserve quadrant and signed-zero behavior from the selected math library.
- [x] **Square root and inverse square root.** Require $x\ge0$ for $\sqrt{x}$ and $x>0$ for reciprocal root; compute $r=\sqrt{x}$ or $r=1/\sqrt{x}$.
- [x] **Exponential and natural logarithm.** Compute $e^x$ for finite $x$; require $x>0$ and compute $\ln x$; report non-finite or out-of-range results.
- [x] **Hypotenuse.** Compute $r=\operatorname{hypot}(x,y)$ using the overflow-safe library primitive rather than $\sqrt{x^2+y^2}$.
- [x] **Fused multiply-add.** Compute $r=\operatorname{fma}(a,b,c)$ as one fused operation in accumulator precision, then range-check the scalar conversion.
- [x] **Inline scalar helpers.** Compute $|x|$, $\min(a,b)$, $\max(a,b)$, and $\operatorname{sign}(x)\in\{-1,0,1\}$ by the header's comparisons; callers validate finite inputs when required.
- [x] **Finite predicates.** Check configured scalar and accumulator values directly with the corresponding finiteness predicate; no narrowing is used for the accumulator check.
- [x] **Checked scalar negation/add/subtract/multiply/divide.** Promote operands to the configured accumulator; compute the named operation; check accumulator and scalar-range finiteness before committing. Division rejects a zero denominator with `UMATH_SINGULAR`.
- [x] **Checked reciprocal.** Require finite $x\ne0$; compute $1/x$ in accumulator precision; commit only when the result fits the configured scalar.
- [x] **Integer power.** Use exponentiation by squaring with an unsigned magnitude that safely represents the minimum signed exponent. Negative exponents invert the base first; reject $0^n$ for $n<0$; define $x^0=1$; check intermediate and final range.
- [x] **Real power.** Allow a negative base only for an exactly integral exponent; define $0^0=1$, return zero for $0^p$ when $p>0$, reject $0^p$ when $p<0$, and report a numeric failure when the result is not representable.
- [x] **Inverse trigonometric functions.** Use principal-branch library functions; require $x\in[-1,1]$ for arcsine/arccosine and finite $x$ for arctangent.
- [x] **Hyperbolic functions.** Evaluate $\sinh x$, $\cosh x$, and $\tanh x$ in accumulator precision; range-check before publishing.

#### Extended scalar functions — implemented

These checked functions are implemented in `scalar.c`; the entries below record
their source behavior and chosen domain, rounding, and small-angle policies.

- [x] **Stable logarithmic helpers.** Compute `log1p(x)` for $x>-1$ and `expm1(x)` with the configured accumulator-precision libm functions; range-check before commit.
- [x] **Log/exp variants and cube root.** Compute $2^x$, $\log_2x$, $\log_{10}x$, and the real cube root $\operatorname{cbrt}(x)$; logarithms require $x>0$. Finite underflow results are accepted; overflow or an unrepresentable scalar result fails without changing output.
- [x] **Inverse hyperbolic functions.** Compute $\operatorname{asinh}(x)$, $\operatorname{acosh}(x)$ for $x\ge1$, and $\operatorname{atanh}(x)$ for $|x|<1$; invalid domains return `UMATH_INVALID_ARGUMENT`.
- [x] **Rounding.** `floor`, `ceil`, and `trunc` follow their C libm definitions; `round` sends exact half-way values away from zero, including negative halves. Each result is range-checked before commit.
- [x] **Remainder.** Compute $\operatorname{fmod}(x,y)=x-\operatorname{trunc}(x/y)y$ and IEEE remainder $x-\operatorname{nearbyint}(x/y)y$ using libm; reject $y=0$ with `UMATH_SINGULAR` and preserve libm signed-zero behavior.
- [x] **`modf` and `copysign`.** Split finite $x$ into integral/fractional parts with `modf`; reject null or identical output pointers and commit both parts together. `copysign` copies the sign bit, including signed zero.

- [x] **Stable small-angle kernels.** Define unnormalized sinc(x)=sin(x)/x, cosc(x)=(1-cos(x))/x^2, and sincc(x)=(x-sin(x))/x^3, with limits 1, 1/2, and 1/6 at zero. For $|x|<0.1$, evaluate the Taylor series through $x^8$; outside that interval use the direct sine formula for sinc/sincc and $2\sin^2(x/2)/x^2$ for cosc.

### Fixed-point arithmetic

Files: [src/umath/fixed.c](../../src/umath/fixed.c), [include/umath/fixed.h](../../include/umath/fixed.h). Here $q$ is the signed integer storage and $F$ is the declared fractional-bit count: Q15 uses $F=15$, Q31 uses $F=31$, and Q16 uses $F=16$.

- [x] **Decode fixed value.** Compute $x=q/2^F$ for each supported format.
- [x] **Encode real value.** Compute $s=2^F x$; reject non-finite or out-of-range $s$; round half away from zero by $q=\lceil s-0.5\rceil$ for $s<0$ and $q=\lfloor s+0.5\rfloor$ for $s\ge0$; commit only after conversion succeeds.
- [x] **Saturating add.** Compute widened $t=q_a+q_b$; clamp $t$ to the signed destination range; return the clamped integer.
- [x] **Saturating subtract.** Compute widened $t=q_a-q_b$; clamp $t$ to the signed destination range; return the clamped integer.
- [x] **Saturating multiply.** Compute widened product $p=q_aq_b$; rescale by integer division $t=p/2^F$ (truncating toward zero); clamp $t$ to the destination range.
- [x] **Saturating division.** Compute widened numerator $p=q_a2^F$; divide by nonzero $q_b$ using truncation toward zero; clamp to the destination range; return `UMATH_SINGULAR` without changing output when $q_b=0$.
- [x] **Saturating absolute value.** Compute $|q|$ in widened signed storage and saturate the unrepresentable minimum-integer magnitude to the positive maximum.
- [x] **Fixed-point square root.** Require $q\ge0$ and compute the exact integer floor $q_{out}=\lfloor\sqrt{q\,2^F}\rfloor$, so the encoded result does not exceed $\sqrt{q/2^F}$; negative input is rejected and output is unchanged.
- [x] **Q-format conversion.** Decode $x=q_s/2^{F_s}$ and encode $q_d=\operatorname{sat}(\operatorname{round}(x2^{F_d}))$ for all supported pairs; right shifts round half away from zero and every narrowing conversion saturates.

### Vector operations

#### Generic vector operations

File: [src/umath/vector/vector.c](../../src/umath/vector/vector.c), [include/umath/vector/vector.h](../../include/umath/vector/vector.h).

- [x] **Generic vector elementwise arithmetic.** For each component $i$, compute $(a+b)_i=a_i+b_i$, $(a-b)_i=a_i-b_i$, $(a\odot b)_i=a_ib_i$, $(a/b)_i=a_i/b_i$ for $b_i\ne0$, or $(sa)_i=sa_i$ in accumulator precision; preflight results so output remains unchanged on failure.
- [x] **Contiguous-vector dot product.** Initialize widened $s=0$; for each $i$, accumulate $s\leftarrow s+a_ib_i$; range-check and return $s$.
- [x] **Vector clamp.** Validate bounds and every input component before writing; for each component compute $y_i=\min(b,\max(a,x_i))$.
- [x] **Stable arbitrary-vector norm.** Initialize $scale=0$, $ssq=1$; for each $a_i$, set $v=|a_i|$ and skip zero; if $v>scale$, update $ssq=1+ssq(scale/v)^2$ and $scale=v$; otherwise update $ssq\leftarrow ssq+(v/scale)^2$; return $0$ if $scale=0$, else $scale\sqrt{ssq}$.
- [x] **Generic vector normalization.** Compute $u_i=a_i/\|a\|_2$ with the scaled accumulator-precision norm (without first narrowing the norm to scalar precision); reject zero norm and preflight all values before writing. Exact in-place use is supported; partial overlap is unsupported.
- [x] **Generic vector distance.** Compute $d=\sqrt{\sum_i(a_i-b_i)^2}$ with scaled accumulation; both vectors have the same caller-supplied count and the scalar output is written only after success.
- [x] **Generic vector interpolation.** Compute $r_i=\operatorname{fma}(t,b_i-a_i,a_i)$ for any finite $t$, including extrapolation; preflight every component before writing and disallow partial buffer overlap.

#### Typed 2D, 3D, and 4D vector operations

Files: [src/umath/vector/vector2.c](../../src/umath/vector/vector2.c), [src/umath/vector/vector3.c](../../src/umath/vector/vector3.c), [src/umath/vector/vector4.c](../../src/umath/vector/vector4.c).

- [x] **Typed dot products.** Compute $a_xb_x+a_yb_y$, add $a_zb_z$ for 3D, and add $a_wb_w$ for 4D; these value-returning helpers use the configured scalar expression directly without a status/range check.
- [x] **Two-dimensional cross product.** Compute $a_xb_y-a_yb_x$.
- [x] **Three-dimensional cross product.** Compute $c_x=a_yb_z-a_zb_y$, $c_y=a_zb_x-a_xb_z$, and $c_z=a_xb_y-a_yb_x$.
- [x] **Fixed 2D/3D/4D norms.** Compute $\operatorname{hypot}(x,y)$; for 3D compute $\operatorname{hypot}(\operatorname{hypot}(x,y),z)$; for 4D compute $\operatorname{hypot}(\operatorname{hypot}(x,y),\operatorname{hypot}(z,w))$.
- [x] **Normalize.** Compute $r=\|a\|_2$ with the stable norm; reject $r\le0$; otherwise compute each $\hat a_i=a_i/r$.
- [x] **Typed componentwise product/division.** Compute $c_i=a_ib_i$ or $c_i=a_i/b_i$; zero divisor components return `UMATH_SINGULAR`, and results outside scalar range return `UMATH_NUMERIC_FAILURE` without changing the output.
- [x] **Typed distance/interpolation.** Compute $d=\|a-b\|_2$ with `hypot` accumulation and $r=(1-t)a+tb$; any finite $t$ is accepted, including extrapolation.
- [x] **Vector projection/rejection.** For nonzero axis $b$, compute $p=((a\cdot b)/(b\cdot b))b$ and $r=a-p$ with a scaled axis and accumulator precision; zero axes return `UMATH_SINGULAR`.
- [x] **Vector reflection.** For nonzero normal $n$, compute $r=a-2((a\cdot n)/(n\cdot n))n$ with a scaled normal; zero normals return `UMATH_SINGULAR`.
- [x] **Vector angles.** For 2D, compute signed `atan2(cross,dot)` in $[-\pi,\pi]$; for 3D, use `atan2(norm(cross),dot)`; for 4D, use `acos(clamp(dot/(norms),-1,1))`. Zero vectors return `UMATH_SINGULAR`.

### Dense matrix operations

File: [src/umath/matrix/core.c](../../src/umath/matrix/core.c), [include/umath/matrix/core.h](../../include/umath/matrix/core.h).

- [x] **Matrix-vector product.** For every row $i$, initialize widened $s_i=0$; accumulate $s_i\leftarrow s_i+A_{ij}x_j$ across columns; store $(Ax)_i=s_i$.
- [x] **Dense matrix product.** For every output $(i,j)$, initialize widened $s_{ij}=0$; accumulate $s_{ij}\leftarrow s_{ij}+A_{ik}B_{kj}$ over $k$; store $(AB)_{ij}=s_{ij}$.
- [x] **Matrix transpose.** For row-major $A$ with shape $m\times n$, write $A^T_{ji}=A_{ij}$ at row-major destination index $j m+i$.
- [x] **Matrix add/subtract/scale.** For each linear storage index $k$, compute $C_k=A_k+B_k$, $C_k=A_k-B_k$, or $C_k=sA_k$ using widened intermediates and range checks.
- [x] **Symmetrize.** For each $i<j$, compute $v=(A_{ij}+A_{ji})/2$ and write $v$ to both positions; leave diagonal entries unchanged.
- [x] **Construction/elementwise operations.** Write $I_{ij}=1$ iff $i=j$, form a diagonal matrix from a vector, and compute the Hadamard product $C_{ij}=A_{ij}B_{ij}$.
- [x] **Trace and norms.** Compute $\operatorname{tr}(A)=\sum_iA_{ii}$, $\|A\|_F$ with iterative `hypot`, $\|A\|_1=\max_j\sum_i|A_{ij}|$, and $\|A\|_\infty=\max_i\sum_j|A_{ij}|$.
- [x] **Determinant/inverse.** Use partial-pivot Gaussian elimination for the determinant (exact zero pivot gives determinant zero) and Gauss-Jordan elimination for the inverse (exact zero pivot returns `UMATH_SINGULAR`). Commit the inverse only after successful range checks. Query `umath_matrix_determinant_workspace_count` for $n^2$ accumulator elements or `umath_matrix_inverse_workspace_count` for $2n^2$ before allocating caller-owned workspace; both reject zero or unrepresentable sizes.
- [x] **Outer product/rank-one update.** Form $C_{ij}=x_i y_j$ and update $A_{ij}\leftarrow A_{ij}+\alpha x_i y_j$; the update is in place and input vectors must not overlap the matrix.
- [x] **General matrix exponential.** Use scaling and squaring with Taylor terms on the scaled matrix, choosing the scale so the matrix 1-norm is at most 0.5; use up to 64 terms and return `UMATH_NO_CONVERGENCE` if the bound is not met. Query `umath_matrix_exponential_workspace_count` for $3n^2$ accumulator elements before allocating caller-owned workspace. There is no fixed dimension ceiling; the query rejects sizes that cannot be represented safely.
- [x] **Array contract.** Matrices are row-major; dimensions are checked, inputs and outputs are disjoint unless an API is in-place, and checked kernels report numeric failures if a later element overflows. Some kernels may have written earlier output elements on such a failure.

### Complex arithmetic

File: [src/umath/complex.c](../../src/umath/complex.c), [include/umath/complex.h](../../include/umath/complex.h).

- [x] **Complex add/subtract.** For $a=a_r+ia_i$, $b=b_r+ib_i$, compute $a+b=(a_r+b_r)+i(a_i+b_i)$ and $a-b=(a_r-b_r)+i(a_i-b_i)$.
- [x] **Complex multiply.** Compute $r=a_rb_r-a_ib_i$ and $i=a_rb_i+a_ib_r$; return $r+ii$.
- [x] **Complex conjugate.** Compute $a^*=a_r-ia_i$.
- [x] **Complex division, stable branches.** Let $c=b_r,d=b_i$. If $|c|\ge|d|$, compute $q=d/c$, $D=c+dq$, $r=(a_r+a_iq)/D$, $i=(a_i-a_rq)/D$; otherwise compute $q=c/d$, $D=d+cq$, $r=(a_rq+a_i)/D$, $i=(a_iq-a_r)/D$. Reject $b=0$ and range-check both outputs.
- [x] **Complex magnitude.** Compute $|a|=\operatorname{hypot}(a_r,a_i)$.
- [x] **Complex exponential.** Compute $s=e^{a_r}$, $r=s\cos(a_i)$, $i=s\sin(a_i)$; return $r+ii$ after finite checks.
- [x] **Complex logarithm.** Compute $m=\operatorname{hypot}(a_r,a_i)$; reject $m=0$; return $\ln(m)+i\operatorname{atan2}(a_i,a_r)$ with the principal branch.
- [x] **Square root/power.** Use the principal square-root branch with a scaled real/imaginary formula; compute $z^w=\exp(w\log z)$ on the principal branch. `log(0)` and undefined zero-base powers are singular; $0^w=0$ for positive real $w$.
- [x] **Argument/polar.** Compute $\arg z=\operatorname{atan2}(y,x)$, defining the zero argument as 0; polar conversion requires nonnegative magnitude.
- [x] **Complex circular/hyperbolic functions.** Use the standard complex sin/cos/sinh/cosh equations and scaled tan/tanh formulas; exact poles return `UMATH_SINGULAR`, and unrepresentable results return `UMATH_NUMERIC_FAILURE`.

### Quaternion operations

File: [src/umath/geometry/quaternion.c](../../src/umath/geometry/quaternion.c), [include/umath/geometry/quaternion.h](../../include/umath/geometry/quaternion.h).

- [x] **Quaternion identity/conjugate.** Identity is $(1,0,0,0)$; conjugate $(w,x,y,z)^*=(w,-x,-y,-z)$.
- [x] **Hamilton product.** For $q=(w,v)$ and $r=(s,u)$, compute scalar $ws-v\cdot u$ and vector $wu+sv+v\times u$.
- [x] **Quaternion normalization.** Compute $n=\operatorname{hypot}(\operatorname{hypot}(w,x),\operatorname{hypot}(y,z))$; reject $n\le0$; divide all four components by $n$.
- [x] **Axis-angle to quaternion.** Normalize axis $\hat n$; compute $h=\theta/2$; return $(\cos h,\hat n\sin h)$.
- [x] **Quaternion to principal axis-angle.** Normalize $q$; if $q_w<0$, negate all components; compute $s=\operatorname{hypot}(\operatorname{hypot}(q_x,q_y),q_z)$ and $\theta=2\operatorname{atan2}(s,q_w)$; if $s\le10^{-12}$ return axis $(1,0,0)$, otherwise return $q_v/s$.
- [x] **Vector rotation.** Normalize $q$; form pure quaternion $p=(0,v)$; compute $r=q\otimes p$ then $p'=r\otimes q^*$; return the vector part.
- [x] **Quaternion dot/norm.** Compute $q\cdot r=\sum_{k=0}^3q_kr_k$ and $\|q\|_2$ with accumulator arithmetic and stable `hypot` norm evaluation.
- [x] **Rotation-matrix conversion.** Convert normalized scalar-first $q=(w,x,y,z)$ to/from an active row-major 3x3 matrix; matrix input must be orthonormal with determinant one within 64 epsilons of the less-precise configured scalar/accumulator type.
- [x] **Shortest-path slerp.** If $q_0\cdot q_1<0$, negate $q_1$; use spherical weights and a normalized linear limit near coincident rotations. Require $t\in[0,1]$.
- [x] **Quaternion exp/log.** Compute $\exp(a,v)=e^a(\cos\|v\|,v\sin\|v\|/\|v\|)$ and $\log(a,v)=(\ln\|q\|,\hat v\operatorname{atan2}(\|v\|,a))$. Zero is singular; negative real inputs choose the positive x-axis at $\pi$.

### Rigid transforms

File: [src/umath/geometry/transform3.c](../../src/umath/geometry/transform3.c), [include/umath/geometry/transform3.h](../../include/umath/geometry/transform3.h).

- [x] **Transform identity/composition.** Identity has unit quaternion and zero translation; compose $R=R_1R_2$ and $t=R_1t_2+t_1$ using the active parent-from-child frame convention.
- [x] **Transform inverse.** For unit rotation $R$, compute $R^{-1}=R^T$ and $t^{-1}=-R^Tt$; the implementation uses quaternion conjugation and rotates $-t$.
- [x] **Point mapping.** Compute $p_{parent}=R p_{child}+t$.
- [x] **Homogeneous conversion.** Encode $T=\begin{bmatrix}R&t\\0&1\end{bmatrix}$ in row-major 4x4 form; decode only finite matrices whose bottom row and row orthogonality pass 32 epsilons of the less-precise configured scalar/accumulator type and determinant passes 128 such epsilons.
- [x] **Transform interpolation.** For $t\in[0,1]$, interpolate translation linearly and rotation with shortest-path quaternion slerp, using normalized linear interpolation near coincident rotations; normalize both input and output rotations.
- [ ] **Geometry contract.** Confirm unit-rotation assumptions, scalar-first convention, frame direction, multiplication order, and behavior of value-returning operations on overflow.

### Cholesky factorization and solve

File: [src/umath/matrix/cholesky.c](../../src/umath/matrix/cholesky.c), [include/umath/matrix/cholesky.h](../../include/umath/matrix/cholesky.h).

- [x] **Cholesky diagonal.** For increasing $j$, compute $s=A_{jj}-\sum_{k<j}L_{jk}^2$; require $s>0$; set $L_{jj}=\sqrt{s}$.
- [x] **Cholesky lower entry.** For $i>j$, compute $s=A_{ij}-\sum_{k<j}L_{ik}L_{jk}$; set $L_{ij}=s/L_{jj}$.
- [x] **Cholesky upper triangle.** Set $L_{ij}=0$ for $i<j$.
- [x] **Forward substitution.** For increasing $i$, compute $y_i=(b_i-\sum_{j<i}L_{ij}y_j)/L_{ii}$.
- [x] **Backward substitution.** For decreasing $i$, compute $x_i=(y_i-\sum_{j>i}L_{ji}x_j)/L_{ii}$.
- [x] **Log-determinant.** For $A=LL^T$, compute $\log\det A=2\sum_i\log L_{ii}$; reject nonpositive diagonal entries and preserve output on failure.
- [x] **Inverse.** Form $L^{-1}$ in the caller output's lower triangle, compute $A^{-1}=L^{-T}L^{-1}$ into its upper triangle, then mirror to restore symmetry. Input and output must not overlap; discard the output if a numeric failure occurs after calculation starts.

### Polynomial evaluation and calculus

File: [src/umath/polynomial/core.c](../../src/umath/polynomial/core.c), [include/umath/polynomial/core.h](../../include/umath/polynomial/core.h).

- [x] **Polynomial evaluation, descending coefficients.** For $P(x)=c_0x^n+c_1x^{n-1}+\cdots+c_n$, initialize $r=c_0$; for $i=1,\ldots,n$, update $r\leftarrow rx+c_i$.
- [x] **Polynomial derivative evaluation.** If $n=0$, return $0$; otherwise initialize $r=nc_0$; for $i=1,\ldots,n-1$, update $r\leftarrow rx+(n-i)c_i$.
- [x] **Definite polynomial integral.** At endpoint $t$, initialize $I_t=c_0t/(n+1)$; for $i=1,\ldots,n$, update $I_t\leftarrow I_t t+c_i t/(n-i+1)$; return $I_b-I_a$.
- [x] **Coefficient derivative/antiderivative.** For descending $c_0,\ldots,c_n$, derivative coefficients are $(nc_0,(n-1)c_1,\ldots,c_{n-1})$; antiderivative coefficients are $(c_0/(n+1),\ldots,c_n,C)$. A constant derivative is the one-coefficient zero polynomial.
- [x] **Polynomial algebra.** Align degrees for addition, multiply by convolution, and compose by Horner substitution. Check size arithmetic/output capacity before writing; output buffers must not overlap inputs, and leading zeros are retained.

### Automatic differentiation — dual numbers

Files: [src/umath/derivatives/dual.c](../../src/umath/derivatives/dual.c),
[include/umath/derivatives/dual.h](../../include/umath/derivatives/dual.h).

The dual-number API implements checked arithmetic and common elementary
functions for one seeded direction. `umath_dual_constant()` and
`umath_dual_seed()` construct zero- and unit-tangent values.

For a dual number with value $v_x$ and tangent $d_x$, check each supported
operator independently:

- [x] **Addition** ($z=x+y$)
  $$
  v_z=v_x+v_y,\qquad d_z=d_x+d_y.
  $$
- [x] **Subtraction** ($z=x-y$)
  $$
  v_z=v_x-v_y,\qquad d_z=d_x-d_y.
  $$
- [x] **Multiplication** ($z=xy$)
  $$
  v_z=v_xv_y,\qquad d_z=d_xv_y+v_xd_y.
  $$
- [x] **Division** ($z=x/y$, $v_y\ne0$)
  $$
  v_z=\frac{v_x}{v_y},\qquad
  d_z=\frac{d_x-v_zd_y}{v_y}
     =\frac{d_xv_y-v_xd_y}{v_y^2}.
  $$
- [x] **Square root** ($z=\sqrt{x}$, $v_x\ge0$)
  $$
  v_z=\sqrt{v_x},\qquad d_z=\frac{0.5\,d_x}{v_z}.
  $$
  Reuse $v_z$; at $v_x=0$ the derivative calculation is non-finite and the current implementation returns singular without publishing output.
- [x] **Exponential** ($z=e^x$)
  $$
  v_z=e^{v_x},\qquad d_z=d_xv_z.
  $$
- [x] **Natural logarithm** ($z=\ln x$, $v_x>0$)
  $$
  v_z=\ln v_x,\qquad d_z=\frac{d_x}{v_x}.
  $$
- [x] **Sine** ($z=\sin x$)
  $$
  v_z=\sin v_x,\qquad d_z=d_x\cos v_x.
  $$
- [x] **Cosine** ($z=\cos x$)
  $$
  v_z=\cos v_x,\qquad d_z=-d_x\sin v_x.
  $$
- [x] **Tangent** ($z=\tan x$)
  $$
  v_z=\tan v_x,\qquad d_z=d_x(1+v_z^2).
  $$
  The derivative uses $\sec^2 x=1+\tan^2x$; the scalar tangent routine reports failure for a non-finite or out-of-range result, without an extra near-pole tolerance band.
- [x] **Negation and reciprocal.** Compute $(-x)=(-v_x,-d_x)$ and, for $v_x\ne0$, $x^{-1}=(1/v_x,-d_x/v_x^2)$.
- [x] **Integer power.** Compute $v_z=v_x^n$; for $v_x\ne0$, use $d_z=n(v_z/v_x)d_x$. At zero, define $n=0$ as constant one, $n=1$ as the input tangent, and $n>1$ as zero tangent; reject negative powers as singular.
- [x] **Dual exponentiation.** For $v_x>0$, compute $v_z=v_x^{v_y}$ and $d_z=v_z(d_y\ln v_x+v_y d_x/v_x)$.
- [x] **Inverse trigonometric functions.** Compute $d(\arcsin x)=d_x/\sqrt{1-v_x^2}$ and $d(\arccos x)=-d_x/\sqrt{1-v_x^2}$ for $|v_x|<1$; compute $d(\arctan x)=d_x/(1+v_x^2)$. Arcsine/arccosine endpoints are singular.
- [x] **Two-input arctangent.** For $v_x^2+v_y^2>0$ off the negative-$x$ branch cut, compute $d_z=(v_xd_y-v_yd_x)/(v_x^2+v_y^2)$; return singular at the origin and on the cut.
- [x] **Inverse square root.** For $v_x>0$, compute $v_z=1/\sqrt{v_x}$ and $d_z=-\tfrac12v_z^3d_x$.
- [x] **Hyperbolic functions.** Compute $(\sinh v_x,d_x\cosh v_x)$, $(\cosh v_x,d_x\sinh v_x)$, and $(\tanh v_x,d_x(1-\tanh^2v_x))$.
- [x] **Hypotenuse.** For $r=\operatorname{hypot}(v_x,v_y)>0$, compute $d_z=(v_xd_x+v_yd_y)/r$; return singular at the origin.
- [x] **Fused multiply-add.** For $z=xy+w$, compute the primal value with one accumulator-precision FMA and tangent $d_z=d_xv_y+v_xd_y+d_w$.
- [x] **Dual elementary extensions.** Apply $d_f=f'(v_x)d_x$ for `log1p`, `expm1`, `exp2`, `log2`, `log10`, `cbrt`, `asinh`, `acosh`, and `atanh`; enforce scalar domains and report singular derivatives at endpoints where the derivative is undefined.
- [x] **Dual rounding/remainder.** Floor/ceil/trunc/round use the zero-derivative convention everywhere. `fmod`/`remainder` use $d_z=d_x-kd_y$ with the selected integer quotient locally constant; return `UMATH_SINGULAR` at quotient discontinuities (nonzero integer ratios for `fmod`, half-integer ratios for `remainder`).
- [x] **Dual absolute value, extrema, clamp, and angle wrap.** `abs(x)` returns $|v_x|$ with tangent $\operatorname{sign}(v_x)d_x$ and is singular at zero; min/max select the winning input and are singular at equal values; clamp passes the input tangent strictly inside the bounds, returns zero tangent strictly outside, and is singular at either boundary (equal bounds define a constant); wrapping preserves the tangent except at the half-open interval boundary, where it is singular.
- [x] **Dual seed / tangent convention**
  $$
  d_x=\frac{dx}{d\epsilon},\qquad
  \text{for a unit seed in coordinate }j,\quad d_{x_i}=\delta_{ij}.
  $$

### Finite-difference Jacobians

Files: [src/umath/derivatives/jacobian.c](../../src/umath/derivatives/jacobian.c),
[include/umath/derivatives/jacobian.h](../../include/umath/derivatives/jacobian.h).

The public API is declared, but the implementation currently returns a
not-implemented status. The following steps are therefore unchecked algorithm
work, not source-confirmed calculations.

- [ ] **Choose the finite-difference step for column $j$.** Candidate rule:
  $h_j=\eta\max(1,|x_j|)$ for configured relative step $\eta>0$; require
  finite $h_j>0$, finite $x_j\pm h_j$, and representable perturbations. The
  public header leaves the exact scaling and representability policy open.
- [ ] **Build the positive perturbation.** Copy $x$ to $x_j^+$; set
  $(x_j^+)_j=x_j+h_j$; leave all other coordinates unchanged.
- [ ] **Evaluate the positive callback.** Compute $f_j^+=f(x_j^+)$; verify
  callback success and all $m$ finite outputs before using any component.
- [ ] **Forward-difference column.** For each $i=0,\ldots,m-1$, compute
  $J_{ij}=(f_{j,i}^+-f_i(x))/h_j$, using the caller-supplied $base\_output=f(x)$.
- [ ] **Build the negative perturbation for central differences.** Copy $x$
  to $x_j^-$; set $(x_j^-)_j=x_j-h_j$; leave every other coordinate unchanged.
- [ ] **Evaluate the negative callback.** Compute $f_j^-=f(x_j^-)$; verify
  callback success and all $m$ finite outputs before using any component.
- [ ] **Central-difference column.** For each row $i$, compute
  $J_{ij}=(f_{j,i}^+-f_{j,i}^-)/(2h_j)$.
- [ ] **Write row-major storage.** Store each result at
  $J[i n+j]$ for output row $i$ and input column $j$.
- [ ] **Select the correct workspace count.** Reserve $n+m$ scalars for
  forward differences and $n+2m$ for central differences; check size addition
  for overflow and validate the caller's workspace count before callbacks.
- [ ] **Finish the Jacobian transactionally.** Repeat for
  $j=0,\ldots,n-1$; commit the complete $m\times n$ result only if all
  callback calls, dimensions, perturbations, and calculations succeed. The
  implementation is currently a stub, so these remain unchecked.

### Dual-number piecewise functions — implemented

These APIs are implemented with explicit branch policies for wrapping and
nonsmooth operations.

- [x] **Angle wrap.** Compute $v_z=\operatorname{wrap}_{[-\pi,\pi)}(v_x)$ and propagate $d_z=d_x$; return `UMATH_SINGULAR` at the wrap boundary.
- [x] **Absolute value, min/max, and clamp.** Propagate the selected branch derivative; return `UMATH_SINGULAR` at absolute-value zero, min/max ties, and distinct clamp boundaries. A clamp with equal bounds is constant and has zero derivative.

### Numerical integration — ODE steps

File: [src/umath/numerical/integration.c](../../src/umath/numerical/integration.c). These are explicit candidate rules; the fixed panel treatment and solver policy must be frozen in the contract.

- [ ] **Composite trapezoid: validate the input.** Require $n\ge2$, finite samples $y_0,\ldots,y_{n-1}$, and finite nonzero spacing $h$; set the sum accumulator $s=0$.
- [ ] **Composite trapezoid: add the first and last half weights.** Set $s\leftarrow s+y_0/2+y_{n-1}/2$.
- [ ] **Composite trapezoid: add interior samples.** For $i=1,\ldots,n-2$, set $s\leftarrow s+y_i$; return $I=hs$.
- [ ] **Composite Simpson: validate panels.** Define whether input count means samples or intervals; require an even number of intervals and do not silently drop an unmatched interval.
- [ ] **Composite Simpson: accumulate weights.** For uniform spacing $h$, compute $s=y_0+y_{n-1}+4\sum_{i\text{ odd}}y_i+2\sum_{i\text{ even},\,0<i<n-1}y_i$; return $I=hs/3$.
- [ ] **RK4: evaluate stage one.** Compute $k_1=f(t,x)$.
- [ ] **RK4: evaluate stage two.** Compute $k_2=f(t+h/2,x+hk_1/2)$.
- [ ] **RK4: evaluate stage three.** Compute $k_3=f(t+h/2,x+hk_2/2)$.
- [ ] **RK4: evaluate stage four.** Compute $k_4=f(t+h,x+hk_3)$.
- [ ] **RK4: combine.** For every state component, compute $x^+=x+h(k_1+2k_2+2k_3+k_4)/6$; check every RHS call and intermediate before publishing any output.
- [ ] **Dormand–Prince stage one.** Compute $k_1=f(t,x)$.
- [ ] **Dormand–Prince stage two.** Compute $k_2=f(t+h/5,\;x+h k_1/5)$.
- [ ] **Dormand–Prince stage three.** Compute $k_3=f(t+3h/10,\;x+h(3k_1/40+9k_2/40))$.
- [ ] **Dormand–Prince stage four.** Compute $k_4=f(t+4h/5,\;x+h(44k_1/45-56k_2/15+32k_3/9))$.
- [ ] **Dormand–Prince stage five.** Compute $k_5=f(t+8h/9,\;x+h(19372k_1/6561-25360k_2/2187+64448k_3/6561-212k_4/729))$.
- [ ] **Dormand–Prince stage six.** Compute $k_6=f(t+h,\;x+h(9017k_1/3168-355k_2/33+46732k_3/5247+49k_4/176-5103k_5/18656))$.
- [ ] **Dormand–Prince stage seven.** Compute $k_7=f(t+h,\;x+h(35k_1/384+500k_3/1113+125k_4/192-2187k_5/6784+11k_6/84))$.
- [ ] **Dormand–Prince fifth-order estimate.** Compute $x^{(5)}=x+h(35k_1/384+500k_3/1113+125k_4/192-2187k_5/6784+11k_6/84)$.
- [ ] **Dormand–Prince fourth-order estimate.** Compute $x^{(4)}=x+h(5179k_1/57600+7571k_3/16695+393k_4/640-92097k_5/339200+187k_6/2100+k_7/40)$.
- [ ] **Dormand–Prince scaled error.** For each component $i$, compute $e_i=(x_i^{(5)}-x_i^{(4)})/(\epsilon_a+\epsilon_r\max(|x_i|,|x_i^{(5)}|))$; compute the specified RMS or max norm $E$.
- [ ] **Dormand–Prince accept/reject and suggested step.** Accept iff $E\le1$; compute $h_{new}=h\,\operatorname{clip}(f_{min},f_{max},f_s E^{-1/5})$ with a defined zero-error rule; publish $x^{(5)}$ only on acceptance and count every RHS evaluation against the limit.
- [ ] **Quadrature/ODE failure contract.** Define callback-failure handling, finite/range checks, workspace count, signed step support, error norm, tolerance floors, and output/state commit behavior.

### Fast Fourier transform

File: [src/umath/signal/fft.c](../../src/umath/signal/fft.c), [include/umath/signal/fft.h](../../include/umath/signal/fft.h).

- [ ] **DFT reference definition.** For $k=0,\ldots,N-1$, compute $X_k=\sum_{n=0}^{N-1}x_n e^{-i2\pi kn/N}$; declare forward sign and whether inverse includes $1/N$.
- [ ] **Radix-2 FFT: bit-reverse input.** Require $N=2^m$; reverse the $m$ index bits and place each input into its bit-reversed position, respecting the declared in-place/out-of-place contract.
- [ ] **Radix-2 FFT: form twiddle factors.** For each stage length $L=2,4,\ldots,N$, compute $W_L=e^{-i2\pi/L}$ and successive powers $w_j=W_L^j$.
- [ ] **Radix-2 FFT: butterfly.** For each block and $j<L/2$, set $u=X_{base+j}$, $v=w_jX_{base+j+L/2}$; write $X'_{base+j}=u+v$ and $X'_{base+j+L/2}=u-v$.
- [ ] **Inverse FFT.** Use the inverse sign in $w_j$; apply the documented normalization exactly once (commonly divide every output by $N$).
- [ ] **Real forward FFT.** Compute or derive only bins $0,\ldots,\lfloor N/2\rfloor$; define DC and even-$N$ Nyquist imaginary parts as zero within rounding policy.
- [ ] **FFT contract.** Freeze supported lengths/radices, workspace, twiddle generation/storage, scaling, overlap, finite checks, and no-partial-output behavior.

### Window functions

File: [src/umath/signal/windows.c](../../src/umath/signal/windows.c), [include/umath/signal/windows.h](../../include/umath/signal/windows.h).

- [ ] **Window application.** For each $n$, compute $y_n=w_nx_n$; specify whether the window is symmetric or periodic.
- [ ] **Hann window candidate.** Compute $w_n=\tfrac12(1-\cos(2\pi n/(N-1)))$ for symmetric form, or replace denominator by $N$ for periodic form.
- [ ] **Hamming window candidate.** Compute $w_n=0.54-0.46\cos(2\pi n/(N-1))$ for symmetric form; freeze the exact coefficients and convention.
- [ ] **Blackman window.** For $n=0,\ldots,N-1$, compute $w_n=0.42-0.50\cos(2\pi n/L)+0.08\cos(4\pi n/L)$, with $L=N-1$ for symmetric or $L=N$ for periodic form; require a defined minimum length and fixed coefficients.
- [ ] **Kaiser window.** Select $L=N-1$ for symmetric or $L=N$ for periodic form; compute $\xi_n=2n/L-1$ and $w_n=I_0(\beta\sqrt{1-\xi_n^2})/I_0(\beta)$; define $N=1$, beta bounds, the bounded $I_0$ approximation, and overflow policy.

### Lie groups

#### SO(2)

File: [src/umath/lie/so2.c](../../src/umath/lie/so2.c), [include/umath/lie/so2.h](../../include/umath/lie/so2.h).

- [ ] **SO(2) exponential.** For angle $\theta$, compute $c=\cos\theta$, $s=\sin\theta$; write $R=[[c,-s],[s,c]]$ in row-major order.
- [ ] **SO(2) logarithm.** Validate orthogonality and determinant within tolerance; compute $\theta=\operatorname{atan2}(R_{21},R_{11})$ and normalize to the declared principal interval.

#### SO(3)

File: [src/umath/lie/so3.c](../../src/umath/lie/so3.c), [include/umath/lie/so3.h](../../include/umath/lie/so3.h).

- [ ] **SO(3) hat/vee.** For $\phi=(\phi_x,\phi_y,\phi_z)$, form $[\phi]_\times=[[0,-\phi_z,\phi_y],[\phi_z,0,-\phi_x],[-\phi_y,\phi_x,0]]$; vee extracts $(M_{32},M_{13},M_{21})$ after skew-symmetry validation.
- [ ] **SO(3) exponential.** Compute $\theta=\|\phi\|$ and $q=(\cos(\theta/2),\phi\,\sin(\theta/2)/\theta)$; use the declared Taylor series at $\theta\approx0$ and normalize only under the documented policy.
- [ ] **SO(3) logarithm.** Normalize $q$ and fix the antipodal sign; compute $s=\|q_v\|$, $\theta=2\operatorname{atan2}(s,q_w)$; return $\phi=q_v\theta/s$, using the small-angle limit when $s\approx0$.
- [ ] **SO(3) left Jacobian.** Let $A=[\phi]_\times$, $\theta=\|\phi\|$; compute $J_l=I+(1-\cos\theta)A/\theta^2+(\theta-\sin\theta)A^2/\theta^3$; substitute the fixed series near zero.
- [ ] **SO(3) inverse left Jacobian.** Compute $J_l^{-1}=I-A/2+[1/\theta^2-(1+\cos\theta)/(2\theta\sin\theta)]A^2$; use its small-angle series and reject/handle singular angles by policy.
- [ ] **SO(3) right Jacobians.** Compute $J_r(\phi)=J_l(-\phi)$ and $J_r^{-1}(\phi)=J_l^{-1}(-\phi)$ under the declared perturbation convention.

#### SE(2)

File: [src/umath/lie/se2.c](../../src/umath/lie/se2.c), [include/umath/lie/se2.h](../../include/umath/lie/se2.h).

- [ ] **SE(2) exponential.** For tangent $\xi=(\rho_x,\rho_y,\theta)$, compute $V(\theta)=[[\sin\theta/\theta,-(1-\cos\theta)/\theta],[(1-\cos\theta)/\theta,\sin\theta/\theta]]$; set $t=V\rho$ and $R=R(\theta)$; use the zero-angle series.
- [ ] **SE(2) logarithm.** Extract principal $\theta$ from $R$; construct/invert matching $V(\theta)$; compute $\rho=V^{-1}t$; return $(\rho_x,\rho_y,\theta)$.
- [ ] **SE(2) adjoint.** For pose $(R,t)$ and tangent order $[\rho_x,\rho_y,\theta]$, form $\operatorname{Ad}_T=[[R,-[t]_\times R],[0,1]]$ with dimensions/signs fixed to the repository's ordering.

#### SE(3)

File: [src/umath/lie/se3.c](../../src/umath/lie/se3.c), [include/umath/lie/se3.h](../../include/umath/lie/se3.h).

- [ ] **SE(3) exponential.** For $\xi=[\rho,\phi]$, compute $R=\operatorname{Exp}_{SO(3)}(\phi)$, $t=J_l(\phi)\rho$, and return $T=(R,t)$.
- [ ] **SE(3) logarithm.** Compute principal $\phi=\operatorname{Log}_{SO(3)}(R)$; compute $\rho=J_l^{-1}(\phi)t$; return $[\rho,\phi]$.
- [ ] **SE(3) adjoint.** For tangent ordering $[\rho,\phi]$, form $\operatorname{Ad}_T=[[R,[t]_\times R],[0,R]]$; validate row-major layout against the public contract.
- [ ] **SE(3) adjoint algebra.** For $\xi=[\rho,\phi]$, form $\operatorname{ad}_\xi=[[\phi]_\times,[\rho]_\times;0,[\phi]_\times]$ under the declared $[\rho,\phi]$ ordering.
- [ ] **SE(3) left Jacobian series.** Compute $J_l(\xi)=\sum_{n=0}^{\infty}\operatorname{ad}_\xi^n/(n+1)!$; implement the named bounded truncation or closed form with a stable zero-angle limit; verify $J_lJ_l^{-1}\approx I$.
- [ ] **SE(3) inverse Jacobian.** Solve $J_lX=I$ using the selected stable matrix method or evaluate its declared closed form; check conditioning and singularity thresholds before publishing.

### Matrix factorization and least-squares algorithms

#### Pivoted LU factorization

File: [src/umath/matrix/lu.c](../../src/umath/matrix/lu.c), [include/umath/matrix/lu.h](../../include/umath/matrix/lu.h).

- [ ] **Pivoted LU factorization.** At column $k$, choose $p=\arg\max_{i\ge k}|A_{ik}|$ under the named scaling rule; reject a pivot below tolerance; swap rows/permutation; set $L_{ik}=A_{ik}/A_{kk}$; update $A_{ij}\leftarrow A_{ij}-L_{ik}A_{kj}$ for $i,j>k$.
- [ ] **LU solve.** Apply the stored row permutation to $b$; solve $Ly=Pb$ by forward substitution; solve $Ux=y$ by backward substitution; check pivots before output commit.
- [ ] **LU determinant.** Compute $\det(A)=s_{perm}\prod_i U_{ii}$; define overflow and singular-matrix policy.

#### Pivoted LDL factorization

File: [src/umath/matrix/ldlt.c](../../src/umath/matrix/ldlt.c), [include/umath/matrix/ldlt.h](../../include/umath/matrix/ldlt.h).

- [ ] **Pivoted LDLáµ€ factorization.** Select the documented 1-by-1 or 2-by-2 symmetric pivot; permute matching rows and columns; record $P$, unit-lower $L$, block diagonal $D$, and each pivot block size.
- [ ] **LDLáµ€ 1-by-1 update.** For pivot $d=A_{kk}$, compute $l_i=A_{ik}/d$ for trailing rows; update each trailing entry $A_{ij}\leftarrow A_{ij}-l_i d l_j$.
- [ ] **LDLáµ€ 2-by-2 update.** For pivot block $D_b$, compute each trailing row $L_{i,:}=A_{i,b}D_b^{-1}$; update trailing block $A_{tail}\leftarrow A_{tail}-L D_bL^T$; reject singular blocks and keep permutation/block descriptors consistent.
- [ ] **LDLáµ€ solve.** Compute $b_p=P^Tb$; solve $Ly=b_p$; solve block-diagonal $Dz=y$ (including 2-by-2 blocks); solve $L^Tw=z$; compute $x=Pw$.

#### Householder QR factorization

File: [src/umath/matrix/qr.c](../../src/umath/matrix/qr.c), [include/umath/matrix/qr.h](../../include/umath/matrix/qr.h).

- [ ] **Householder QR.** For column $k$, form $v=x+\operatorname{sign}(x_1)\|x\|e_1$; set $\tau=2/(v^Tv)$; apply $A_{k:,:}\leftarrow A_{k:,:}-\tau v(v^TA_{k:,:})$; store reflector and $R$.
- [ ] **QR least squares.** Apply stored reflectors to compute $c=Q^Tb$; estimate numerical rank using the declared tolerance; solve $R_{11}x_1=c_1$; define explicitly whether rank-deficient inputs fail or use a minimum-norm solution.

#### Symmetric Jacobi eigensolver

File: [src/umath/matrix/symmetric_eigen.c](../../src/umath/matrix/symmetric_eigen.c), [include/umath/matrix/symmetric_eigen.h](../../include/umath/matrix/symmetric_eigen.h).

- [ ] **Symmetric Jacobi eigensolver.** For selected $p,q$, compute $\tau=(a_{qq}-a_{pp})/(2a_{pq})$, $t=\operatorname{sign}(\tau)/(|\tau|+\sqrt{1+\tau^2})$, $c=1/\sqrt{1+t^2}$, $s=tc$; update $a_{pp}\leftarrow a_{pp}-t a_{pq}$, $a_{qq}\leftarrow a_{qq}+t a_{pq}$, and set $a_{pq}=a_{qp}=0$.
- [ ] **Symmetric Jacobi eigensolver: update remaining rows.** For each $r\ne p,q$, compute $a_{rp}^{new}=ca_{rp}-sa_{rq}$ and $a_{rq}^{new}=sa_{rp}+ca_{rq}$; mirror values into symmetric positions.
- [ ] **Symmetric Jacobi eigensolver: update eigenvectors.** Apply the same two-column rotation to $V$; repeat until off-diagonal tolerance or sweep limit; sort eigenpairs and fix eigenvector sign convention.

#### One-sided Jacobi singular-value decomposition

File: [src/umath/matrix/svd.c](../../src/umath/matrix/svd.c), [include/umath/matrix/svd.h](../../include/umath/matrix/svd.h).

- [ ] **One-sided Jacobi SVD candidate.** For each column pair $p,q$, compute $\alpha=A_p^TA_p$, $\beta=A_q^TA_q$, $\gamma=A_p^TA_q$; skip if $|\gamma|$ is below the orthogonality tolerance.
- [ ] **One-sided Jacobi SVD candidate: rotate.** Compute $\zeta=(\beta-\alpha)/(2\gamma)$, $t=\operatorname{sign}(\zeta)/(|\zeta|+\sqrt{1+\zeta^2})$, $c=1/\sqrt{1+t^2}$, and $s=ct$; update $A_p\leftarrow cA_p-sA_q$, $A_q\leftarrow sA_p+cA_q$, and apply the same rotation to $V$; sweep to tolerance or the cap.
- [ ] **Finish one-sided Jacobi SVD.** Compute $\sigma_j=\|A_j\|_2$; set $U_j=A_j/\sigma_j$ for nonzero singular values; sort $\sigma_j$ descending while permuting $U,V$ columns together; check $A_{input}\approx U\Sigma V^T$ and orthogonality. Pin this algorithm or another exact bounded method in the module contract.
- [ ] **SVD pseudoinverse solve.** Set cutoff $\tau=\epsilon_{rel}\sigma_{max}$; compute $z=U^Tb$; set $w_i=z_i/\sigma_i$ when $\sigma_i>\tau$, else $w_i=0$; compute $x=Vw$ and report rank.

#### Least-squares solvers

File: [src/umath/optimization/least_squares.c](../../src/umath/optimization/least_squares.c), [include/umath/optimization/least_squares.h](../../include/umath/optimization/least_squares.h).

- [ ] **Least squares.** Solve $\hat\theta=\arg\min_\theta\|A\theta-y\|_2^2$ through QR/SVD; form residual $r=y-A\hat\theta$ componentwise; compute a scaled norm $\|r\|_2$; report rank/tolerance.
- [ ] **Weighted least squares.** Validate $w_i>0$; form $A'_{i,:}=\sqrt{w_i}A_{i,:}$ and $y'_i=\sqrt{w_i}y_i$; solve the transformed least-squares system without forming $A^TWA$.

### Streaming statistics and probability

#### Streaming statistics

File: [src/umath/statistics/streaming.c](../../src/umath/statistics/streaming.c), [include/umath/statistics/streaming.h](../../include/umath/statistics/streaming.h).

- [ ] **Streaming mean.** Initialize $n=0,\mu=0$; for each finite sample $x$, increment $n$, compute $\delta=x-\mu$, then $\mu\leftarrow\mu+\delta/n$.
- [ ] **Welford variance.** With old $\delta=x-\mu$, update $\mu\leftarrow\mu+\delta/n$, then $M_2\leftarrow M_2+\delta(x-\mu)$; return population variance $M_2/n$ or sample variance $M_2/(n-1)$ as selected.
- [ ] **Streaming covariance.** For vector sample $x$, compute $\delta=x-\mu$; update $\mu\leftarrow\mu+\delta/n$; compute $\delta_2=x-\mu$; update $C_{ij}\leftarrow C_{ij}+\delta_i\delta_{2,j}$; divide by $n$ or $n-1$ and symmetrize under the declared convention.
- [ ] **Quantile.** Validate $p\in[0,1]$; sort or select the order statistic; define index $h=(n-1)p$ and interpolation between $x_{\lfloor h\rfloor}$ and $x_{\lceil h\rceil}$, or name another exact convention.
- [ ] **Standard deviation scaffold.** Compute the selected sample/population variance and take its nonnegative square root; preserve the caller output on invalid count or numeric failure.
- [ ] **RMS scaffold.** Compute sqrt(sum_i x_i^2/n) with scaled sum-of-squares accumulation; define empty input and overflow behavior.

#### Scalar probability distributions

File: [src/umath/probability/distributions.c](../../src/umath/probability/distributions.c), [include/umath/probability/distributions.h](../../include/umath/probability/distributions.h).

- [ ] **Log-sum-exp.** Let $m=\max_i x_i$; compute $s=\sum_i e^{x_i-m}$; return $m+\ln s$; define empty and all-negative-infinity behavior.
- [ ] **Softmax/log-softmax scaffolds.** Compute logsumexp L; return p_i=exp(x_i-L) or log p_i=x_i-L. Define empty input, infinities, aliasing, sum-to-one error, and transactional output; these support normalized particle and model weights.
- [ ] **Normal log-density.** Validate $\sigma^2>0$; compute $z=x-\mu$; return $-\tfrac12[\ln(2\pi\sigma^2)+z^2/\sigma^2]$.
- [ ] **Normal CDF.** Compute $z=(x-\mu)/\sigma$ for $\sigma>0$; evaluate the fixed bounded approximation to $\Phi(z)=\tfrac12[1+\operatorname{erf}(z/\sqrt2)]$; specify tail clipping/error.
- [ ] **Chi-square CDF.** Validate $x\ge0$ and integer degrees of freedom $\nu>0$; compute $P(\nu/2,x/2)=\gamma(\nu/2,x/2)/\Gamma(\nu/2)$ using the named bounded series/continued fraction.

#### Multivariate normal distribution

File: [src/umath/probability/multivariate_normal.c](../../src/umath/probability/multivariate_normal.c), [include/umath/probability/multivariate_normal.h](../../include/umath/probability/multivariate_normal.h).

- [ ] **Multivariate normal log-density.** Solve $Lz=r$ by forward substitution; accumulate $q=\sum_i z_i^2$ and $\ell=\sum_i\ln L_{ii}$; return $-\tfrac12[d\ln(2\pi)+q]-\ell$ for $P=LL^T$.

### Splines, polynomial roots, random generation, special functions, and sparse matrices

#### Spline interpolation

File: [src/umath/interpolation/splines.c](../../src/umath/interpolation/splines.c), [include/umath/interpolation/splines.h](../../include/umath/interpolation/splines.h).

- [ ] **Cubic Hermite basis.** For $u=(t-t_0)/T$, compute $h_{00}=2u^3-3u^2+1$, $h_{10}=u^3-2u^2+u$, $h_{01}=-2u^3+3u^2$, $h_{11}=u^3-u^2$; compute $p=h_{00}p_0+h_{10}Tv_0+h_{01}p_1+h_{11}Tv_1$.
- [ ] **Hermite derivatives.** Differentiate the basis with respect to $u$; compute velocity as $(dp/du)/T$ and acceleration as $(d^2p/du^2)/T^2$.
- [ ] **Bezier by de Casteljau.** Copy control points to workspace; for level $r=1,\ldots,n$, replace each point $P_i\leftarrow(1-u)P_i+uP_{i+1}$; the final point is the value. Define derivative control points as $n(P_{i+1}-P_i)$ and apply the same recurrence.
- [ ] **B-spline Cox–de Boor basis.** Initialize $N_{i,0}(u)=1$ iff $t_i\le u<t_{i+1}$ (special-case the final knot); recursively compute $N_{i,p}(u)=\frac{u-t_i}{t_{i+p}-t_i}N_{i,p-1}+\frac{t_{i+p+1}-u}{t_{i+p+1}-t_{i+1}}N_{i+1,p-1}$, treating zero denominators as zero.
- [ ] **B-spline derivative.** Compute $N'_{i,p}(u)=\frac{p}{t_{i+p}-t_i}N_{i,p-1}(u)-\frac{p}{t_{i+p+1}-t_{i+1}}N_{i+1,p-1}(u)$; general higher-order derivatives need the exact recursive contract.

#### Polynomial roots

File: [src/umath/polynomial/roots.c](../../src/umath/polynomial/roots.c), [include/umath/polynomial/roots.h](../../include/umath/polynomial/roots.h).

- [ ] **Polynomial roots.** Normalize coefficient scale and remove leading zero coefficients; solve the selected degree-specific or iterative method; bound iterations; verify each candidate by residual $|P(r)|$ and the documented tolerance; specify repeated-root and ordering behavior.

#### Pseudorandom number generation

File: [src/umath/random/prng.c](../../src/umath/random/prng.c), [include/umath/random/prng.h](../../include/umath/random/prng.h).

- [ ] **PRNG state transition.** Name the generator, constants, bit widths, and transition $s_{k+1}=F(s_k,stream)$; advance exactly as specified and derive output bits from the resulting state.
- [ ] **Uniform random mapping.** Map integer bits to $u\in[0,1)$ with the stated number of random bits; define rejection/correction if avoiding modulo bias; convert to $[a,b)$ by $a+(b-a)u$.
- [ ] **Normal random mapping.** If using Box–Muller, obtain independent $u_1,u_2\in(0,1]$; compute $z_0=\sqrt{-2\ln u_1}\cos(2\pi u_2)$ and $z_1=\sqrt{-2\ln u_1}\sin(2\pi u_2)$; return $\mu+\sigma z$ and define cached-sample stream use.

#### Special functions

File: [src/umath/special/functions.c](../../src/umath/special/functions.c), [include/umath/special/functions.h](../../include/umath/special/functions.h).

- [ ] **Special functions.** For erf, log-gamma, and $I_0$, record the chosen approximation/series, domain, iteration bound, overflow scaling, and maximum error; no approximation is complete until those constants are fixed.

#### Compressed sparse row matrices

File: [src/umath/future/matrix/sparse_csr.c](../../src/umath/future/matrix/sparse_csr.c), [include/umath/future/matrix/sparse_csr.h](../../include/umath/future/matrix/sparse_csr.h).

- [ ] **CSR validation.** Require $row\_offsets[0]=0$, nondecreasing offsets, $row\_offsets[m]=nnz$, all column indices $<n$, and finite stored values; define sortedness/duplicate policy.
- [ ] **CSR matrix-vector product.** For each row $i$, initialize $s_i=0$; for $k=row_i,\ldots,row_{i+1}-1$, accumulate $s_i\leftarrow s_i+value_k x_{column_k}$; commit output after validation.
- [ ] **CSR addition.** For each row, merge the two sorted column-index runs; copy the smaller column, add values on equal columns, apply zero-elision policy, and check exact output capacity before writing.

## UCON equation sets

### Two-degree-of-freedom PID — SPEC Part 1681

Files: [src/ucon/control/classical/pid.c](../../src/ucon/control/classical/pid.c), [include/ucon/control/classical/pid.h](../../include/ucon/control/classical/pid.h). The sequence below follows the registered Tustin PID implementation.

- [x] **Validate configuration.** Check every gain, weight, bound, tracking time, and sample time is finite; require $T_s>0$, $I_{min}\le I_{max}$, valid anti-windup enum, output bounds ordered when saturation is enabled, $N>0$ when $K_d\ne0$, and $T_t>0$ for back-calculation when $K_i\ne0$.
- [x] **Form weighted errors.** Compute $e_P=br-y$, $e_I=r-y$, and $e_D=cr-y$ from reference $r$, measurement $y$, proportional setpoint weight $b$, and derivative setpoint weight $c$.
- [x] **Propose the integral update.** Compute $I^*=I_k+\frac{T_s}{2}(e_I+e_{I,k-1})$; reject non-finite results; clamp to $[I_{min},I_{max}]$.
- [x] **Handle disabled derivative.** If $K_d=0$, set $D_{k+1}=0$.
- [x] **Filter derivative when enabled.** Compute $q=2+NT_s$, $a_0=2K_dN/q$, $a_1=(2-NT_s)/q$; then compute $D_{k+1}=a_1D_k+a_0(e_D-e_{D,k-1})$; reject invalid denominator or non-finite state.
- [x] **Form the unsaturated control.** Compute $u_{raw}=K_pe_P+K_iI^*+D_{k+1}+u_{ff}$ and check finiteness.
- [x] **Conditional anti-windup.** Compute $\Delta u_I=K_i(I^*-I_k)$. If $u_{raw}>u_{max}$ and $\Delta u_I>0$, or $u_{raw}<u_{min}$ and $\Delta u_I<0$, restore $I^*=I_k$ and recompute $u_{raw}$.
- [x] **Clamp the output.** If saturation is enabled, compute $u=\min(u_{max},\max(u_{min},u_{raw}))$; otherwise set $u=u_{raw}$.
- [x] **Back-calculation anti-windup.** When selected and $K_i\ne0$, choose $u_t$ from the supplied tracking output or the controller's own saturated output; update $I^*\leftarrow I^*+\frac{T_s}{T_tK_i}(u_t-u_{raw})$; check finiteness and clamp the integral again.
- [x] **Commit state last.** Store $I_{k+1}=I^*$, $D_{k+1}$, $e_{I,k}=e_I$, and $e_{D,k}=e_D$; publish $u$ only after all calculations succeed. Check reset initializes all stored values to zero.

### Lead-lag — SPEC Part 1682

Files: [src/ucon/control/classical/lead_lag.c](../../src/ucon/control/classical/lead_lag.c), [include/ucon/control/classical/lead_lag.h](../../include/ucon/control/classical/lead_lag.h).

- [x] **Validate inputs and tuning.** Require finite $K,T_L,T_G,T_s$, with $T_L,T_G,T_s>0$ and finite input $x_k$.
- [x] **Compute bilinear denominators.** Set $d_L=T_s+2T_L$ and $d_G=T_s+2T_G$; reject non-finite or non-positive values.
- [x] **Compute recurrence coefficients.** Set $a=(T_s-2T_L)/d_L$, $b=(T_s-2T_G)/d_G$, and $K_b=K d_L/d_G$; validate all three.
- [x] **Compute output.** Using prior input $x_{k-1}$ and prior output $y_{k-1}$, compute $y_k=-b y_{k-1}+K_b(x_k+a x_{k-1})$.
- [x] **Commit after validation.** Check $y_k$ is finite; then store $x_{k-1}\leftarrow x_k$, $y_{k-1}\leftarrow y_k$, and publish output. Verify reset zeroes both histories.

### State feedback, LQR/LQI/LQG, decoupling, pole placement

Source family: `src/ucon/control/state_space/`, `src/ucon/estimators/observers/`, and related state-feedback files. These are standard candidates; matrices, state order, sampling method, and tracking/feedforward design are model-specific contract inputs.

- [ ] **Discrete plant propagation.** For the declared discrete model, compute $x_{k+1}=Ax_k+Bu_k+w_k$ and $y_k=Cx_k+Du_k+v_k$; check dimensions and coordinate units.
- [ ] **Tracking error.** Compute $e_k=r_k-y_k$ or $e_k=x_k-x_{ref,k}$ under the selected feedback convention; do not mix output and state error.
- [ ] **LQR finite-horizon recursion.** Initialize terminal $P_N=Q_f$; for $k=N-1,\ldots,0$, compute $S_k=R+B^TP_{k+1}B$, $K_k=S_k^{-1}B^TP_{k+1}A$, and $P_k=Q+A^TP_{k+1}A-A^TP_{k+1}BK_k$.
- [ ] **LQR infinite-horizon candidate.** Iterate the same Riccati update to the fixed point $P$; compute $K=(R+B^TPB)^{-1}B^TPA$; verify $R+B^TPB$ is positive definite and closed-loop eigenvalues satisfy the stability requirement.
- [ ] **Apply state feedback.** Compute $u_k=u_{ff,k}-K(x_k-x_{ref,k})$; then apply the declared input constraints and anti-windup/safety logic.
- [ ] **Integral augmentation for LQI.** Update $z_{k+1}=z_k+T_s(r_k-y_k)$; augment $x^a=[x;z]$, $A_a=[[A,0],[-T_sC,I]]$, $B_a=[[B],[-T_sD]]$ for this sign convention; run LQR on $(A_a,B_a)$ and compute $u=-K_xx-K_iz+u_{ff}$.
- [ ] **Kalman prediction for LQG.** Compute $\hat x^-_{k+1}=A\hat x_k+Bu_k$ and $P^-_{k+1}=AP_kA^T+Q$.
- [ ] **Kalman correction for LQG.** Compute innovation $\nu=y-C\hat x^--Du$, $S=CP^-C^T+R$, $K_f=P^-C^TS^{-1}$; update $\hat x=\hat x^-+K_f\nu$ and $P=(I-K_fC)P^-(I-K_fC)^T+K_fRK_f^T$.
- [ ] **Decoupling matrix.** For a model with output derivatives $y^{(r)}=b(x)+A_d(x)u$, form $A_d$ from the plant; verify its rank/conditioning; compute $u=A_d^{-1}(v-b(x))$ or the named constrained solve; do not assume invertibility.
- [ ] **Pole placement.** Compute characteristic polynomial $\det(zI-(A-BK))$; equate its coefficients to the target polynomial; solve for $K$ using the named controllability/algorithm method; verify achieved poles and conditioning.
- [ ] **State estimator variants.** For Luenberger form, compute $\hat x_{k+1}=A\hat x_k+Bu_k+L(y_k-C\hat x_k-Du_k)$; specify how $L$ is designed and how stale/invalid measurements are handled.
- [ ] **Gain scheduling.** Select adjacent schedule points $\rho_i\le\rho\le\rho_{i+1}$; compute $\lambda=(\rho-\rho_i)/(\rho_{i+1}-\rho_i)$ and each scheduled gain $K(\rho)=(1-\lambda)K_i+\lambda K_{i+1}$; define endpoint clamping, multi-parameter interpolation, and controller-state transfer.
- [ ] **Reference governor.** Predict candidate output $y_{k+j}(v)$ for a proposed reference step $v$; choose the largest $\gamma\in[0,1]$ such that every predicted state/output/input constraint holds for $r_k^{gov}=r_{k-1}^{gov}+\gamma(r_k-r_{k-1}^{gov})$; define horizon, search method, and fallback.
- [ ] **Model contract blockers.** Record plant matrices, discretization, costs $Q,R,Q_f$, noise covariances, reference equilibrium, actuator limits, and safety policy before treating any formula as normative.

### MPC family — SPEC Parts 1683–1686

Source family: `src/ucon/control/predictive/`. The constraints and solver are part of the algorithm contract, not interchangeable implementation details.

- [ ] **Set prediction model.** Record $x_{j+1}=Ax_j+Bu_j$ (or the explicitly selected nonlinear model) and $y_j=Cx_j+Du_j$; freeze horizons $N_p,N_c$ and held-input rule after $N_c$.
- [ ] **Roll out the horizon.** Set $x_{0|k}=x_k$; for each $j$, compute $x_{j+1|k}=Ax_{j|k}+Bu_{j|k}$ and then $y_{j|k}=Cx_{j|k}+Du_{j|k}$.
- [ ] **Form tracking and move penalties.** Compute $J=\sum_{j=1}^{N_p}(y_{j|k}-r_{j|k})^TQ(y_{j|k}-r_{j|k})+\sum_{j=0}^{N_c-1}\Delta u_{j|k}^TR\Delta u_{j|k}+J_f(x_{N_p|k})$, where $\Delta u_j=u_j-u_{j-1}$; record dimensions and positive-semidefinite requirements.
- [ ] **Condense linear dynamics.** Repeatedly substitute the model to form $X=\mathcal A x_k+\mathcal B U$; form the quadratic objective $J(U)=\tfrac12U^THU+f^TU+c$ from the weights and reference.
- [ ] **Build constraints.** Convert each state, output, input, and move bound into $G U\le g$; include slack variables only where declared, with their penalty and bounds.
- [ ] **Solve and validate.** Run the named bounded QP/nonlinear solver; check convergence, feasibility, finite values, and iteration budget; report failure without exposing a partial control.
- [ ] **Apply receding horizon.** Apply only $u_{0|k}^*$; shift warm-start sequence only after successful solution; on the next sample, measure/update $x_k$ and solve again.
- [ ] **Deadbeat MPC variant.** State the terminal equality $x_{N|k}=r$ or the precise tracking state; verify the equality is reachable within the horizon and constraints before accepting a solution.
- [ ] **Economic MPC variant.** Replace or augment the tracking stage cost with the declared economic cost $\ell(x,u)$; retain explicit feasibility/stability constraints and define what selects among multiple optima.
- [ ] **DDP / nonlinear predictive variant.** Roll out nonlinear dynamics; linearize/quadratize each stage; perform the backward recursion for value derivatives; compute a regularized feedforward/feedback step; line-search candidate controls; accept only if objective decreases and constraints hold.
- [ ] **MPC contract blockers.** Freeze plant, horizon, terminal cost/set, constraints, soft-constraint penalties, QP method, tolerances, warm-start behavior, deadline, and fallback action.

### Nonlinear feedback, CLF/CBF QP, and allocation

Source family: `src/ucon/control/nonlinear/`, `src/ucon/control/allocation/`, and related nonlinear-control modules. Each plant-dependent law below requires the plant and sign conventions to be specified.

- [ ] **Control-affine model.** Write $\dot x=f(x)+g(x)u$; state dimensions, input bounds, disturbances, and relative degree must be explicit.
- [ ] **Tracking error.** Compute $e=x-x_d$ (or the named output error); compute $\dot e$ from the model and reference derivatives.
- [ ] **Lyapunov candidate.** Select a positive-definite $V(e)$; compute $\dot V=\nabla V(e)^T(f(x)+g(x)u-\dot x_d)$ and separate drift and control terms.
- [ ] **CLF inequality.** Form $L_fV+L_gV\,u\le-cV+\delta$ with declared $c>0$ and slack policy; validate that dimensions match.
- [ ] **Control barrier function.** Define safe set $\mathcal C=\{x:h(x)\ge0\}$; compute $\dot h=\nabla h^T(f+gu)$; enforce $L_fh+L_gh\,u\ge-\alpha(h)$ for the selected class-$\mathcal K$ function.
- [ ] **CLF/CBF quadratic program.** Minimize $\tfrac12(u-u_{ref})^TR(u-u_{ref})+\tfrac12p\delta^2$ subject to CLF, CBF, and actuator constraints; verify feasibility and solver termination before output.
- [ ] **Control allocation.** Given desired generalized action $\tau_d$ and effectiveness matrix $B$, minimize $\tfrac12(Bu-\tau_d)^TW(Bu-\tau_d)+\tfrac12(u-u_0)^TR(u-u_0)$ subject to actuator limits; compute Hessian $H=B^TWB+R$ and linear term $q=-B^TW\tau_d-Ru_0$; solve the declared bounded QP.
- [ ] **Computed torque.** For $M(q)\ddot q+C(q,\dot q)\dot q+g(q)=\tau$, compute $e=q-q_d$, $v=\ddot q_d-K_d\dot e-K_pe$; compute $\tau=M(q)v+C(q,\dot q)\dot q+g(q)$; apply torque limits and validate model assumptions.
- [ ] **Feedback linearization / NDI.** Differentiate the selected output until $y^{(r)}=b(x)+A(x)u$; form virtual input $v=y_d^{(r)}-K e$; solve $A(x)u=v-b(x)$ using a validated inverse or bounded solver; reject singular/ill-conditioned $A$.
- [ ] **Backstepping first stage.** For strict-feedback $\dot x_1=f_1(x_1)+g_1(x_1)x_2$, set $z_1=x_1-x_{1d}$; choose $V_1=\tfrac12z_1^2$; derive virtual control $\alpha_1$ so $\dot V_1\le-c_1z_1^2+z_1z_2$.
- [ ] **Backstepping recursion.** Set $z_i=x_i-\alpha_{i-1}$; augment $V_i=V_{i-1}+\tfrac12z_i^2$; differentiate including $\dot\alpha_{i-1}$; choose the next virtual control to cancel cross terms and add $-c_i z_i^2$.
- [ ] **Backstepping final input.** Derive the actual input from the final $\dot V_n$ inequality; verify all virtual-control derivatives exist and the model is in the assumed strict-feedback form.
- [ ] **Sliding-mode candidate.** Define surface $s=\dot e+\Lambda e$ (or the declared higher-order surface); compute equivalent control from $\dot s=0$; add the selected bounded reaching term such as $-k\operatorname{sat}(s/\phi)$; document chattering and actuator constraints.
- [ ] **INDI candidate.** From measured angular acceleration $\dot\omega$ and control effectiveness $G$, compute $\Delta u=G^+(\nu_d-\dot\omega)$; apply to previous actuator command; freeze filter delay, pseudo-inverse rank threshold, actuator dynamics, and saturation logic.
- [ ] **Flatness candidate.** Choose flat outputs; derive state and input algebraically from flat outputs and their derivatives; provide the explicit model-specific maps and derivative order before implementation.
- [ ] **Nonlinear-control blockers.** Specify dynamics, disturbance bounds, differentiability, safe set, actuator model, optimization method/deadline, fallback, and robustness assumptions. A family name alone does not uniquely define a controller.

### Adaptive, classical, disturbance-rejection, and tuning

Source family includes `src/ucon/control/adaptive/`, `src/ucon/control/classical/`, and `src/ucon/control/disturbance_rejection/`.

- [ ] **Recursive least squares prediction.** Compute $\hat y_k=\phi_k^T\theta_k$ and innovation $e_k=y_k-\hat y_k$.
- [ ] **RLS gain.** Compute $v_k=P_k\phi_k$, $d_k=\lambda+\phi_k^Tv_k$, $K_k=v_k/d_k$; require forgetting factor $0<\lambda\le1$ and valid positive denominator.
- [ ] **RLS parameter update.** Compute $\theta_{k+1}=\theta_k+K_ke_k$.
- [ ] **RLS covariance update.** Compute $P_{k+1}=\lambda^{-1}(P_k-K_k\phi_k^TP_k)$; symmetrize only under a stated numerical policy and verify positive semidefinite bounds.
- [ ] **Gradient adaptive law.** With model $y=\phi^T\theta$ and error $e=y-\phi^T\hat\theta$, compute $\dot{\hat\theta}=\Gamma\phi e$ (or the signed law selected by the error definition); discretize by the named method.
- [ ] **Projection/leakage.** Project proposed $\hat\theta$ into the specified parameter set; for leakage, compute $\dot{\hat\theta}=\Gamma\phi e-\sigma\Gamma\hat\theta$; record excitation and stability assumptions.
- [ ] **Extremum seeking.** Generate dither $d_k=a\sin(\omega t)$; perturb parameter $\theta+d_k$; multiply measured objective by the phase-aligned dither; low-pass the product to estimate the gradient; update $\theta_{k+1}=\theta_k-k_g\hat g_k$; freeze filter and time-scale choices.
- [ ] **IMC model split.** Specify plant $G$, model $G_m$, and stable proper inverse/filter $Q$; form $u=Q(r-G_m u)+$ any declared feedback correction; derive the exact implemented block recurrence from those transfer functions.
- [ ] **Smith predictor.** Compute model output $\hat y=G_m u$ and delayed model output $\hat y_d=G_m e^{-\tau s}u$; form predicted feedback signal $y_p=y+\hat y-\hat y_d$; feed $r-y_p$ to the nominal controller; define delay mismatch and model reset behavior.
- [ ] **ADRC extended-state observer candidate.** For $\dot x_1=x_2,\;\dot x_2=b_0u+x_3,\;\dot x_3=w$, compute output error $e_o=z_1-y$; update $z_1^+=z_1+T_s(z_2-\beta_1e_o)$, $z_2^+=z_2+T_s(z_3+b_0u-\beta_2e_o)$, $z_3^+=z_3-T_s\beta_3e_o$ under the selected discretization.
- [ ] **ADRC control law candidate.** Compute tracking error $e=z_1-r$ and velocity error $e_v=z_2-\dot r$; form virtual acceleration $v=\ddot r-k_pe-k_de_v$; compute $u=(v-z_3)/b_0$; validate $b_0\ne0$ and apply declared limits.
- [ ] **Autotuning relay test.** Define relay amplitude $d$ and hysteresis; detect steady oscillation amplitude $a$ and period $P_u$; compute ultimate gain $K_u=4d/(\pi a)$ for the ideal symmetric relay case; use only the specified tuning table and acceptance limits.
- [ ] **Autotuning safeguards.** Check excitation, steady-state window, noise, actuator saturation, abort limits, and state restoration; derive gains from a named rule rather than selecting an unspecified rule.
- [ ] **Adaptive/classical blockers.** Freeze plant model/order, tuning rule, filter discretization, delay, adaptation bounds, excitation, convergence criteria, and safe fallback.

### Linear KF, scalar KF, EKF variants, square-root filters

Source family: `src/ucon/estimators/kalman/` and related filter modules.

- [ ] **Linear prediction.** Compute $\hat x^-_k=A\hat x_{k-1}+Bu_{k-1}$ and $P^-_k=AP_{k-1}A^T+Q$.
- [ ] **Innovation.** Compute $\nu_k=y_k-C\hat x^-_k-Du_k$ and $S_k=CP^-_kC^T+R$; check finite values and positive definiteness.
- [ ] **Kalman gain.** Solve $S_k Z=C P^-_k$; set $K_k=Z^T$ without explicitly inverting $S_k$.
- [ ] **State correction.** Compute $\hat x_k=\hat x^-_k+K_k\nu_k$.
- [ ] **Joseph covariance correction.** Compute $A_k=I-K_kC$; then $P_k=A_kP^-_kA_k^T+K_kRK_k^T$; symmetrize only by the selected tolerance policy.
- [ ] **Scalar KF specialization.** Compute $P^-=P+Q$, $S=P^-+R$, $K=P^-/S$, $\hat x^-=\hat x$, $\hat x^+=\hat x^-+K(y-\hat x^-)$, and $P^+=(1-K)P^-$; enforce $Q\ge0,R>0$.
- [ ] **EKF process prediction.** Compute $\hat x^-=f(\hat x,u)$; evaluate $F=\partial f/\partial x$ at the specified point; propagate $P^-=FPF^T+Q$ (or use continuous-time discretization with the declared method).
- [ ] **EKF measurement update.** Compute predicted measurement $\hat y=h(\hat x^-)$; evaluate $H=\partial h/\partial x$; then apply the innovation, gain, state, and Joseph covariance steps above with $C$ replaced by $H$.
- [ ] **Innovation gating.** Compute Mahalanobis statistic $d^2=\nu^TS^{-1}\nu$ by a solve; compare to the declared threshold; state whether a rejected observation leaves state and covariance unchanged.
- [ ] **Square-root prediction.** Represent $P=SS^T$; form the concatenated matrix $[AS, Q^{1/2}]$; compute a QR factorization of its transpose; transpose the triangular factor to obtain the predicted square root.
- [ ] **Square-root correction.** Use triangular solves and QR/Cholesky updates to compute innovation and posterior square roots; state the exact factor-update algorithm, sign normalization, downdate failure behavior, and jitter policy.
- [ ] **Estimator timing and reset.** Fix whether predict precedes measurement update, observation timestamp policy, missing-measurement behavior, covariance initialization, and reset state.

### UKF Merwe UKF, CKF, IMM, particle, MHE

Source family: `src/ucon/estimators/kalman/`, `src/ucon/estimators/bayesian/`, and `src/ucon/estimators/moving_horizon/`.

- [ ] **Merwe sigma-point parameters.** For state dimension $n$, compute $\lambda=\alpha^2(n+\kappa)-n$; compute mean weights $W_0^{(m)}=\lambda/(n+\lambda)$, covariance weight $W_0^{(c)}=W_0^{(m)}+1-\alpha^2+\beta$, and $W_i^{(m)}=W_i^{(c)}=1/[2(n+\lambda)]$ for $i=1,\ldots,2n$.
- [ ] **UKF sigma points.** Factor $(n+\lambda)P=SS^T$; set $\chi_0=\hat x$; for each column $i$ of $S$, set $\chi_i=\hat x+S_i$ and $\chi_{i+n}=\hat x-S_i$.
- [ ] **UKF process transform.** Propagate each $\chi_i$ through $f(\chi_i,u)$; compute $\hat x^- =\sum_iW_i^{(m)}\chi_i^-$; compute $P^-=Q+\sum_iW_i^{(c)}(\chi_i^--\hat x^-)(\chi_i^--\hat x^-)^T$.
- [ ] **UKF measurement transform.** Propagate sigma points through $h$; compute $\hat y$, $S=R+\sum_iW_i^{(c)}(y_i-\hat y)(y_i-\hat y)^T$, and $P_{xy}=\sum_iW_i^{(c)}(\chi_i^--\hat x^-)(y_i-\hat y)^T$.
- [ ] **UKF correction.** Solve $SK^T=P_{xy}^T$; compute $\hat x^+=\hat x^-+K(y-\hat y)$ and $P^+=P^--KSK^T$; check covariance and factorization policy.
- [ ] **Cubature points.** For CKF dimension $n$, form $2n$ points $\chi_i=\hat x\pm\sqrt n\,S_j$ with weight $1/(2n)$ for each signed column; propagate and compute weighted mean/covariance, then use the same cross-covariance correction sequence.
- [ ] **IMM mixing probabilities.** Given transition matrix $\pi_{ij}$ and prior model probabilities $\mu_i$, compute $c_j=\sum_i\pi_{ij}\mu_i$ and mixing weights $\mu_{i|j}=\pi_{ij}\mu_i/c_j$; handle zero $c_j$ explicitly.
- [ ] **IMM mixed initial conditions.** Compute $\hat x_{0|j}=\sum_i\mu_{i|j}\hat x_i$ and $P_{0|j}=\sum_i\mu_{i|j}[P_i+(\hat x_i-\hat x_{0|j})(\hat x_i-\hat x_{0|j})^T]$.
- [ ] **IMM model likelihood/update.** Run the selected filter per model; compute likelihood $\Lambda_j$ from its innovation density; update $\mu_j=\Lambda_jc_j/\sum_l\Lambda_lc_l$ and normalize; combine model states/covariances including between-model spread.
- [ ] **Particle prediction and weighting.** Propagate each particle $x_i$ through the process model; multiply prior weight by likelihood $p(y|x_i)$; normalize $w_i\leftarrow w_i/\sum_jw_j$; define underflow-safe log-weight handling.
- [ ] **Particle resampling.** Compute $N_{eff}=1/\sum_iw_i^2$; resample only under the declared threshold; specify systematic/stratified method, deterministic RNG stream, and reset weights to $1/N$.
- [ ] **Moving-horizon estimation.** Over window $j=k-N+1,\ldots,k$, minimize arrival cost plus $\sum_j\|y_j-h(x_j)\|_{R^{-1}}^2+\|w_j\|_{Q^{-1}}^2$ subject to $x_{j+1}=f(x_j,u_j,w_j)$ and state/noise bounds; define optimizer, warm start, deadline, and fallback.

### RLS tracking, attitude, and observers

Source family: `src/ucon/estimators/`, `src/ucon/aerospace/attitude/`, and observer modules. The shared RLS update appears above; each estimator's state/model and geometry remain module-specific.

- [ ] **Attitude error quaternion.** For unit quaternions $q,q_d$, compute $q_e=q_d^*\otimes q$ (or the explicitly selected inverse order); if its scalar part is negative, negate all components to select the short rotation.
- [ ] **Quaternion logarithmic error.** Set $s=\|q_{e,v}\|$, $\theta=2\operatorname{atan2}(s,q_{e,w})$; compute $e_R=q_{e,v}\theta/s$, with a stable small-$s$ limit.
- [ ] **Quaternion PD candidate.** Compute rate error $e_\omega=\omega-R(q_e)^T\omega_d$ under the selected frame convention; compute $\tau=-K_Re_R-K_\omega e_\omega+\tau_{ff}$; apply torque/momentum limits.
- [ ] **SO(3) attitude error.** Compute $R_e=R_d^TR$; compute $e_R=\tfrac12\operatorname{vee}(R_e-R_e^T)$; apply a principal-angle/log map near large errors and define the $\pi$-angle behavior.
- [ ] **B-dot detumble.** Estimate $\dot B_k=(B_k-B_{k-1})/T_s$ after calibration and frame alignment; compute magnetic dipole command $m=-k\dot B$; project $m$ into the plane orthogonal to $B$ via $m_\perp=m-B(B^Tm)/(B^TB)$; clamp by coil limits and reject invalid $B$.
- [ ] **Complementary attitude observer.** Predict orientation by $q^-_{k+1}=q_k\otimes\operatorname{Exp}(\tfrac12\omega_kT_s)$; form gravity/magnetic innovation in a common frame; correct with a declared gain; renormalize and handle zero/invalid sensors.
- [ ] **Luenberger observer.** Compute residual $r_k=y_k-C\hat x_k-Du_k$ and update $\hat x_{k+1}=A\hat x_k+Bu_k+Lr_k$; verify observability and designed poles.
- [ ] **Tracking differentiator.** Define the exact second-order dynamics and discrete update; record noise filtering, initialization, and derivative units rather than using a name-only contract.
- [ ] **Alpha-beta tracker.** Predict $\hat p^-_k=\hat p_{k-1}+T_s\hat v_{k-1}$ and $\hat v^-_k=\hat v_{k-1}$; compute residual $r_k=z_k-\hat p^-_k$; correct $\hat p_k=\hat p^-_k+\alpha r_k$ and $\hat v_k=\hat v^-_k+(\beta/T_s)r_k$; freeze gain tuning and missing-measurement behavior.
- [ ] **Alpha-beta-gamma tracker.** Predict $p^- =p+T_sv+\tfrac12T_s^2a$, $v^-=v+T_sa$, $a^-=a$; compute $r=z-p^-$; correct $p=p^-+\alpha r$, $v=v^-+(\beta/T_s)r$, $a=a^-+(2\gamma/T_s^2)r$; define the exact gamma normalization used by the selected convention.
- [ ] **RLS tracking safeguards.** Freeze forgetting factor, covariance initialization/bounds, excitation floor, parameter projection, covariance symmetry, and reset/reacquisition behavior.

### GNSS/INS, guidance, and aerospace algorithms

Source family: `src/ucon/aerospace/navigation/`, `src/ucon/aerospace/guidance/`, and flight-control modules. Sensor datum, frames, Earth model, and signal definitions are mandatory inputs.

- [ ] **Geodetic/ECEF conversion.** Declare ellipsoid semi-major axis $a$ and eccentricity $e$; compute $N(\varphi)=a/\sqrt{1-e^2\sin^2\varphi}$; map latitude, longitude, height to $x=(N+h)\cos\varphi\cos\lambda$, $y=(N+h)\cos\varphi\sin\lambda$, $z=(N(1-e^2)+h)\sin\varphi$.
- [ ] **ECEF-to-geodetic conversion.** Compute longitude with $\operatorname{atan2}(y,x)$; iterate latitude/height using the named bounded method; check pole and convergence behavior.
- [ ] **Strapdown attitude propagation.** Correct gyro bias/scale; integrate $q_{k+1}=q_k\otimes\operatorname{Exp}(\omega_{body}T_s/2)$ using the chosen coning correction; normalize and rotate vectors between named frames.
- [ ] **Velocity/position propagation.** Rotate specific force $f^n=R_b^nf^b$; subtract gravity and add Coriolis/transport terms under the chosen local-level or ECEF model; integrate velocity then position with the declared method.
- [ ] **GNSS pseudorange prediction.** For satellite ECEF position $r_s$ and receiver position $r$, compute geometric range $\rho=\|r_s-r\|$; apply Earth-rotation/Sagnac, clock, ionosphere, troposphere, and hardware corrections from the named model; predicted pseudorange is $\hat z=\rho+c(\delta t_r-\delta t_s)+$ corrections.
- [ ] **GNSS measurement residual.** Compute $r_i=z_i-\hat z_i$; assemble geometry Jacobian with line-of-sight derivatives and receiver-clock column; solve weighted least squares or update the selected filter; reject outliers by the specified rule.
- [ ] **Carrier/code loop discriminator.** From prompt correlators $I_E,Q_E,I_P,Q_P,I_L,Q_L$, calculate the declared DLL/PLL/FLL discriminator; normalize by the specified amplitude/atan2 expression; apply loop filter and NCO update with carrier/code units fixed.
- [ ] **INS/GNSS error-state update.** Define error state and linearized dynamics $\delta x_{k+1}=F_k\delta x_k+G_kw_k$; form measurement residual/Jacobian; run the selected KF sequence; inject position, velocity, attitude, and bias corrections; reset the error state and covariance consistently.
- [ ] **Proportional navigation.** Compute relative position $r$, relative velocity $v$, range $R=\|r\|$, LOS unit vector $\hat r=r/R$, LOS rate $\dot{\hat r}=(I-\hat r\hat r^T)v/R$; compute commanded acceleration $a_c=N V_c(\hat r\times\dot{\hat r})$ or the declared planar equivalent; define closing-speed sign and saturation.
- [ ] **Path following / lookahead.** Project vehicle position onto the path; compute cross-track error and tangent heading; select lookahead distance $L_d$; compute heading command $\psi_c=\psi_{path}+\operatorname{atan2}(-e_y,L_d)$ under the stated sign convention; apply curvature/speed constraints.
- [ ] **Pure-pursuit guidance.** Transform lookahead point $(x_L,y_L)$ into the vehicle frame; compute curvature $\kappa=2y_L/(x_L^2+y_L^2)$; command lateral acceleration $a_y=v^2\kappa$ or steering $\delta=\arctan(L_w\kappa)$ for wheelbase $L_w$; define reverse motion, zero-range, and saturation behavior.
- [ ] **Aerospace/INS blockers.** Freeze Earth model, datum, gravity, frame handedness, quaternion order, timing/synchronization, sensor error model, correction policy, navigation mode, and integrity limits.

### PLL/FLL/NCO, SOGI, DFT, and zero crossing

Source family: `src/ucon/estimators/frequency/`, `src/ucon/electrical/grid/`, and `src/ucon/aerospace/navigation/` tracking loops.

- [ ] **Phase detector.** For reference and feedback phasors, compute $e_\phi=\operatorname{atan2}(Q,I)$ or the selected normalized cross-product discriminator; define polarity and phase wrap.
- [ ] **PI loop filter.** Compute $I_{k+1}=I_k+K_iT_se_{\phi,k}$ (or trapezoidal form); compute $\omega_k=\omega_0+K_pe_{\phi,k}+I_k$; clamp and apply anti-windup as declared.
- [ ] **NCO phase update.** Compute $\theta_{k+1}=\operatorname{wrap}(\theta_k+T_s\omega_k)$; produce $\sin\theta$/$\cos\theta$ with the selected phase units and wrap interval.
- [ ] **FLL cross/dot discriminator.** From consecutive complex samples $z_k,z_{k-1}$, compute $c=I_kQ_{k-1}-Q_kI_{k-1}$ and $d=I_kI_{k-1}+Q_kQ_{k-1}$; compute $\Delta\phi=\operatorname{atan2}(c,d)$ and $\hat f=\Delta\phi/(2\pi T_s)$.
- [ ] **SOGI quadrature generator.** Declare center frequency $\omega_0$ and gain $k$; implement the selected continuous transfer functions $D(s)=k\omega_0s/(s^2+k\omega_0s+\omega_0^2)$ and $Q(s)=k\omega_0^2/(s^2+k\omega_0s+\omega_0^2)$; derive discrete coefficients by the named transform and compute each state/output recurrence.
- [ ] **Goertzel single-bin transform.** For target bin $k$, set $\omega=2\pi k/N$ and $c=2\cos\omega$; initialize $s_{-1}=s_{-2}=0$; for $n=0,\ldots,N-1$, compute $s_n=x_n+c s_{n-1}-s_{n-2}$; finish with $X_k=s_{N-1}-e^{-i\omega}s_{N-2}$; define scaling and windowing.
- [ ] **Sliding DFT update.** With $\omega_k=2\pi k/N$ and $X_k[n]=\sum_{r=0}^{N-1}x[n-r]e^{-i\omega_kr}$, compute $X_k[n]=e^{-i\omega_k}X_k[n-1]+x[n]-x[n-N]$; define initialization, roundoff-drift correction, and bin/window convention.
- [ ] **Synchronous demodulation.** Multiply $x_n$ by the reference quadratures $\cos\theta_n$ and $\sin\theta_n$; low-pass each product to obtain $I_n$ and $Q_n$; compute amplitude $A_n=2\sqrt{I_n^2+Q_n^2}$ and phase $\phi_n=\operatorname{atan2}(Q_n,I_n)$ under the selected peak/RMS normalization.
- [ ] **Single-bin DFT.** For frequency $\omega$, update $X_k=\sum_{n=0}^{N-1}x_n e^{-i\omega n}$ using the declared sliding recurrence; compute magnitude $|X|=\sqrt{I^2+Q^2}$ and phase $\operatorname{atan2}(Q,I)$.
- [ ] **Zero-crossing period.** Detect consecutive interpolated sign crossings; for samples $(x_{k-1},x_k)$, estimate crossing time $t_c=t_{k-1}+T_s\frac{-x_{k-1}}{x_k-x_{k-1}}$; compute period from same-polarity crossing times and frequency $f=1/T$.
- [ ] **Loop stability/tuning.** Derive discrete filter coefficients from the specified loop bandwidth, damping, detector gain, and NCO gain; record units and acquisition/lock thresholds; do not guess coefficient formulas from the algorithm label.

### Runtime filters and coefficient design

Source family: `src/ucon/filtering/runtime/`, `src/ucon/filtering/coefficient_design/`, and `src/ucon/infrastructure/blocks/` filter modules.

- [ ] **First-order low-pass candidate.** For $\tau>0$, compute $\alpha=T_s/(\tau+T_s)$; update $y_k=y_{k-1}+\alpha(x_k-y_{k-1})$; initialize and reset according to the declared policy.
- [ ] **Exponential smoothing candidate.** Compute $y_k=\alpha x_k+(1-\alpha)y_{k-1}$ for $0\le\alpha\le1$; document whether $\alpha$ comes from a time constant or is directly configured.
- [ ] **Moving average.** Update running sum $S_k=S_{k-1}+x_k-x_{k-N}$ using a ring buffer; compute $y_k=S_k/N$; specify startup fill and overflow-safe accumulator width.
- [ ] **FIR filter.** Compute $y_k=\sum_{j=0}^{M-1}b_jx_{k-j}$; state coefficient order, state-buffer rotation, scaling, and startup history.
- [ ] **Biquad / IIR recurrence.** Normalize coefficients by $a_0$; compute $y_k=(\sum_{j=0}^{M}b_jx_{k-j}-\sum_{j=1}^{N}a_jy_{k-j})/a_0$; define direct form, state layout, scaling, and denormal handling.
- [ ] **Second-order notch candidate.** From center frequency $\omega_0$, sample rate $f_s$, and quality factor $Q$, compute $\omega_d=2\pi f_0/f_s$, $\alpha=\sin\omega_d/(2Q)$; form $b=[1,-2\cos\omega_d,1]$, $a=[1+\alpha,-2\cos\omega_d,1-\alpha]$; normalize by $a_0$ and verify pole stability.
- [ ] **Bilinear coefficient design.** Substitute $s=\frac{2}{T_s}\frac{1-z^{-1}}{1+z^{-1}}$ in the named analog transfer function; expand numerator/denominator in $z^{-1}$; normalize by the leading denominator coefficient; apply prewarping if specified.
- [ ] **Rate limiter.** Compute $\Delta=x_k-y_{k-1}$; clamp it to $[-r_{down}T_s,r_{up}T_s]$; set $y_k=y_{k-1}+\Delta_{clamped}$.
- [ ] **Saturator/deadzone/hysteresis.** Saturator: $y=\min(u_{max},\max(u_{min},x))$. Deadzone: $y=0$ for $|x|\le d$, else $y=\operatorname{sign}(x)(|x|-d)$. Hysteresis: switch on at upper threshold and off at lower threshold; retain state between thresholds.
- [ ] **Filter contract.** Freeze filter topology, discretization/prewarp, coefficient precision, state initialization, bumpless transfer, stability margin, finite checks, and reset behavior.
- [ ] **Savitzky–Golay value/derivative filter.** For each centered window, build design matrix $A_{ij}=t_i^j$; solve $c=(A^TA)^{-1}A^Ty$ using a stable QR solve; return $c_0$ and derivatives $r!c_r$ at the center; fix polynomial order, window parity, edge handling, and rank failure policy.
- [ ] **Windowed RMS / peak tracking.** Maintain $S_k=S_{k-1}+x_k^2-x_{k-N}^2$ and return $RMS_k=\sqrt{\max(0,S_k/N)}$; update peak $p_k=\max(|x_k|,\lambda p_{k-1})$ for declared decay $0\le\lambda\le1$ (or use an exact window maximum); fix warm-up, reset, and dropped-sample rules.

### Electrical transforms, grid, and motor control

Source family: `src/ucon/electrical/transforms/`, `src/ucon/electrical/grid/`, and `src/ucon/electrical/motor_control/`. Confirm phase order, amplitude/power invariance, and electrical angle direction.

- [ ] **Clarke transform, amplitude-invariant.** For phase values $(a,b,c)$, compute $\alpha=\frac23(a-\frac12b-\frac12c)$ and $\beta=\frac23(\frac{\sqrt3}{2}(b-c))$; compute zero sequence $0=\frac13(a+b+c)$ if included.
- [ ] **Inverse Clarke.** Compute $a=\alpha+0$, $b=-\alpha/2+\sqrt3\beta/2+0$, $c=-\alpha/2-\sqrt3\beta/2+0$; include the scale factor required by the declared transform convention.
- [ ] **Park transform.** Given electrical angle $\theta$, compute $d=\alpha\cos\theta+\beta\sin\theta$ and $q=-\alpha\sin\theta+\beta\cos\theta$.
- [ ] **Inverse Park.** Compute $\alpha=d\cos\theta-q\sin\theta$, $\beta=d\sin\theta+q\cos\theta$.
- [ ] **Grid PLL.** Transform measured grid voltage into dq; form $e_\phi=v_q$ (or normalized $v_q/v_d$); run the selected PI and NCO updates; compute frequency and angle with limits and lock detection.
- [ ] **P/Q droop control.** Low-pass measured powers to $P,Q$; compute $\omega^*=\omega_0-m_p(P-P^*)$ and $V^*=V_0-n_q(Q-Q^*)$; define signs, filter discretization, sharing coefficients, and voltage/frequency limits.
- [ ] **Proportional-resonant controller.** For sinusoidal tracking frequency $\omega_0$, use $G_{PR}(s)=K_p+2K_r\omega_c s/(s^2+2\omega_c s+\omega_0^2)$; substitute the selected bilinear/prewarped mapping, derive discrete recurrence coefficients, and check pole stability and output limits.
- [ ] **PMSM dq model candidate.** State $v_d=R_si_d+L_d\dot i_d-\omega_eL_qi_q$ and $v_q=R_si_q+L_q\dot i_q+\omega_e(L_di_d+\psi_f)$; solve for current derivatives and discretize with the selected method.
- [ ] **PMSM torque candidate.** Compute $\tau_e=\tfrac32p[\psi_f i_q+(L_d-L_q)i_di_q]$ under the declared scaling convention.
- [ ] **FOC current control.** Compute $e_d=i_d^*-i_d$, $e_q=i_q^*-i_q$; update PI integrators; add decoupling/feedforward terms from the motor model; inverse Park and inverse Clarke; apply modulation and voltage-vector limits.
- [ ] **Space-vector PWM.** Compute reference vector magnitude $V$ and sector $s=\lfloor\theta/(\pi/3)\rfloor$; reduce angle to $\theta_s\in[0,\pi/3]$; compute $T_1=\sqrt3T_sV\sin(\pi/3-\theta_s)/V_{dc}$, $T_2=\sqrt3T_sV\sin\theta_s/V_{dc}$, and $T_0=T_s-T_1-T_2$; map centered dwell times to phase duties; define overmodulation and negative-zero-time handling.
- [ ] **DTC flux/torque estimate and switch selection.** Update stator flux $\psi_{\alpha\beta,k}=\psi_{\alpha\beta,k-1}+T_s(v_{\alpha\beta,k}-R_si_{\alpha\beta,k})$; compute $T_e=\tfrac32p(\psi_\alpha i_\beta-\psi_\beta i_\alpha)$ and sector $\theta_\psi=\operatorname{atan2}(\psi_\beta,\psi_\alpha)$; quantize flux/torque errors with hysteresis and select the voltage vector from a named switching table.
- [ ] **MTPA/flux weakening.** For the declared PMSM model, maximize $T_e(i_d,i_q)$ subject to $i_d^2+i_q^2\le I_{max}^2$; above base speed also enforce $v_d^2+v_q^2\le V_{max}^2$. Derive and bound the selected numerical or closed-form solution; include current/voltage slew and transition policy.
- [ ] **Electrical protection.** Compute current magnitude and thermal/power limits from the named hardware model; derate or trip according to the declared policy; all constants and units must come from a device/system specification.

### Trajectories, profiles, and infrastructure blocks

Source family: `src/ucon/trajectory/profiles/`, `src/ucon/infrastructure/blocks/`, and `src/ucon/infrastructure/fault_detection/`.

- [ ] **Cubic polynomial trajectory.** Set $p(t)=a_0+a_1t+a_2t^2+a_3t^3$; derive $v(t)=a_1+2a_2t+3a_3t^2$ and $a(t)=2a_2+6a_3t$; solve the four endpoint position/velocity conditions as a 4-by-4 linear system.
- [ ] **Quintic polynomial trajectory.** Set $p(t)=\sum_{j=0}^5a_jt^j$; derive velocity and acceleration termwise; solve the six endpoint position, velocity, and acceleration conditions; validate duration $T>0$ and boundary residuals.
- [ ] **Bezier trajectory.** Evaluate position using de Casteljau; derivative control points are $P'_i=n(P_{i+1}-P_i)$ and $p'(u)$ is the Bezier curve of those points; convert time derivatives by $du/dt=1/T$.
- [ ] **B-spline trajectory.** Compute basis functions $N_{i,p}(u)$ by Cox–de Boor; evaluate $p(u)=\sum_iN_{i,p}(u)P_i$; compute derivative basis using knot differences and convert derivatives to time units.
- [ ] **Trapezoid velocity profile.** Compute acceleration time $t_a=v_{max}/a_{max}$ and distance $d_a=v_{max}^2/(2a_{max})$; if distance permits, use accel/cruise/decel phases; otherwise solve triangular peak speed $v_p=\sqrt{a_{max}D}$ and use $t_a=v_p/a_{max}$; integrate position consistently across phase boundaries.
- [ ] **Jerk-limited S-curve.** For each constant-jerk phase $j$, integrate $a_{k+1}=a_k+jT_s$, $v_{k+1}=v_k+a_kT_s+\tfrac12jT_s^2$, $p_{k+1}=p_k+v_kT_s+\tfrac12a_kT_s^2+\tfrac16jT_s^3$; determine phase durations from endpoint and max jerk/acceleration/velocity constraints; verify continuity and exact stopping.
- [ ] **Reference model.** Specify model order and recurrence, e.g. $x_{k+1}=A_mx_k+B_mr_k$, $y_k=C_mx_k+D_mr_k$; derive discrete coefficients from the named continuous model and initialization.
- [ ] **Unit delay.** Output $y_k=x_{k-1}$; then store $x_k$ as next history; define initial output.
- [ ] **Sum junction.** Compute $y_k=\sum_i s_i x_{i,k}$ with explicit signs $s_i\in\{-1,+1\}$ and overflow behavior.
- [ ] **Sensor voter.** Compute pairwise residuals $r_{ij}=|x_i-x_j|$; compare with thresholds; select/average only eligible sensors according to the fixed voting and tie-break policy.
- [ ] **Signal validity / fault detection.** Compute residual/statistic from the declared model; compare to a threshold; update persistence counter; latch on specified consecutive failures; define reset and stale-data policy.
- [ ] **Multirate bridge.** Define source and destination periods; specify hold, interpolation, or decimation equation; calculate rate ratio and timestamp alignment; bound stale-data age.
- [ ] **Trajectory/infrastructure limits.** Check continuity, kinematic maxima, monotone time, deterministic phase switching, overflow, initialization, and limit/fault transition policies.
- [ ] **Minimum-snap trajectory.** Represent each segment as $p(t)=\sum_{j=0}^{7}a_jt^j$; derive velocity, acceleration, jerk, and snap termwise; minimize $\int_0^T[p^{(4)}(t)]^2dt$ subject to waypoint and continuity constraints; solve the resulting bounded quadratic system and check segment timing/conditioning.
- [ ] **Standalone control blocks.** Gain: $y=Kx$. Integrator candidates: $I_{k+1}=I_k+T_se_k$ (forward Euler) or $I_{k+1}=I_k+\tfrac12T_s(e_k+e_{k-1})$ (trapezoid). Back-calculation: $I_{k+1}=I_k+T_se_k+(T_s/T_t)(u_{track}-u_{raw})/K_i$ for $K_i\ne0$. Conditional integration freezes only increments driving farther into saturation. Bumpless transfer solves $I=(u_{target}-u_P-u_D-u_{ff})/K_i$ when $K_i\ne0$; define the zero-integral-gain case and state transfer.

### Medical profile controllers — contract blocker

Files: `src/ucon/medical/profiles/`. The repository does not specify an approved patient/plant model or safety law; the equations below are required deliverables, not guessed clinical controls.

- [ ] **Glucose control profile.** Obtain the approved glucose/insulin dynamics, units, sensor delay/noise, meal/disturbance model, actuation limits, hypoglycemia constraints, and clinician-approved fallback; only then write the prediction, controller, and safety-filter equations.
- [ ] **PK/PD target control.** Obtain the approved compartment/state equations, parameter ranges, concentration units, drug delivery constraints, pharmacodynamic target, and safety envelope; only then derive the dosing/controller recurrence.
- [ ] **Ventilator control profile.** Obtain the approved respiratory mechanics and gas-exchange model, sensor/actuator dynamics, modes, pressure/volume/rate bounds, alarm logic, and clinician-approved fallback; only then derive the control law.
- [ ] **Medical verification gate.** Record model provenance, validation domain, units, saturation, alarm thresholds, fault behavior, and independent review before any implementation is enabled.

### Future UCON families — standard candidates and blockers

- [ ] **Kalman innovation fault detector.** Compute $d^2=\nu^TS^{-1}\nu$; compare against the selected chi-square threshold; apply persistence/latch/reset policy.
- [ ] **CUSUM change detector.** For normalized scalar innovation $z_k$, update positive accumulator $g_k^+=\max(0,g_{k-1}^++z_k-\kappa)$ and negative accumulator $g_k^-=\max(0,g_{k-1}^--z_k-\kappa)$; alarm when either exceeds $h$; define baseline, normalization, warm-up, persistence, and rearm behavior.
- [ ] **Control barrier safety filter.** Solve the minimum-change constrained input problem described in the CLF/CBF section; define infeasibility fallback and timing bound.
- [ ] **Robust/MPC safety variants.** Specify uncertainty sets, tube/terminal sets, robust constraints, solver, and proof obligations; an algorithm name does not determine these values.
- [ ] **Fuzzy/neural controller.** Define membership functions/weights, inference and defuzzification or network layer equations, training artifact/version, bounded outputs, and runtime/fault policy.
- [ ] **Reinforcement-learning control.** Keep policy/value equations and action bounds explicit; define training/runtime separation, deterministic deployment, safety shield, and fallback before promotion.
- [ ] **Sensor fusion beyond listed filters.** State each sensor model, synchronization and covariance assumptions, residual gates, missing-data handling, and output integrity policy.
- [ ] **Infrastructure protocol/control blocks.** Every future block needs a state transition, reset, timing, saturation/threshold, and failure equation before registration.

## Other UETD equations and algorithms

### Additional UETD files with equations or algorithms

#### UCORE checksums, bit operations, and status

Files: [src/ucore/checksum.c](../../src/ucore/checksum.c), [src/ucore/bitops.c](../../src/ucore/bitops.c).

- [x] **CRC-8/MSB-first update.** Start from caller-supplied remainder $c$; for each byte $b$, set $c\leftarrow c\oplus b$; repeat 8 times: if the prior top bit is 1, set $c\leftarrow (c\ll1)\oplus0x07$, else set $c\leftarrow c\ll1$; keep the result in 8 bits. Return the running remainder.
- [x] **CRC-16/CCITT/MSB-first update.** Start from caller-supplied remainder $c$; for each byte $b$, set $c\leftarrow c\oplus(b\ll8)$; repeat 8 times: if the prior top bit is 1, set $c\leftarrow(c\ll1)\oplus0x1021$, else set $c\leftarrow c\ll1$; keep the result in 16 bits. Return the running remainder.
- [x] **CRC-32/reflected update.** Start from caller-supplied remainder $c$; for each byte $b$, set $c\leftarrow c\oplus b$; repeat 8 times: if the prior low bit is 1, set $c\leftarrow(c\gg1)\oplus0xEDB88320$, else set $c\leftarrow c\gg1$. Return the running remainder.
- [x] **CRC buffer edge case.** Process bytes in increasing index order; if $buffer=NULL$ and $length\ne0$, return the input remainder unchanged. These routines do not choose an initial seed, final XOR/reflection, or serialized byte order; the caller's protocol contract owns those steps.
- [x] **Next power of two.** Return 0 if $v=0$ or $v>2^{31}$; otherwise set $v\leftarrow v-1$, successively OR with right shifts by $1,2,4,8,16$, then return $v+1$.
- [x] **Floor binary logarithm.** Initialize $r=0$; while $v>1$, set $v\leftarrow v\gg1$ and $r\leftarrow r+1$; return $r$ (the current implementation returns 0 for input 0 as well as 1).
- [x] **16-bit byte swap.** Return $((v\ll8)\,|\,(v\gg8))$ narrowed to 16 bits.
- [x] **32-bit byte swap.** Return $((v\mathbin{\&}0x000000ff)\ll24)\,|\,((v\mathbin{\&}0x0000ff00)\ll8)\,|\,((v\mathbin{\&}0x00ff0000)\gg8)\,|\,((v\mathbin{\&}0xff000000)\gg24)$.
- [ ] **Optional bit masks/field packing.** If public helpers are added, validate bit range before shifting; define full-width mask handling, signed-field extension, insertion truncation, and byte order. These helpers are not in the current `bitops.h` API.
- [ ] **CRC protocol finalization.** If a protocol adapter needs a final reflection/XOR or byte serialization, spell it out per protocol; the three current CRC functions return raw incremental state and do not finalize it.

#### UOS time, deadlines, and periodic scheduling

Files: [src/uos/baremetal/baremetal.c](../../src/uos/baremetal/baremetal.c), [src/uos/freertos/freertos.c](../../src/uos/freertos/freertos.c), [src/uos/future/safertos/safertos.c](../../src/uos/future/safertos/safertos.c).

- [x] **Current-time conversion.** Bare metal returns the monotonic microseconds directly. FreeRTOS computes $t_{us}=\lfloor ticks\cdot10^6/f_{tick}\rfloor$; if tick rate is zero or multiplication would overflow, it returns 0.
- [x] **Convert finite wait duration to FreeRTOS ticks.** For $d>0$ and configured $f_{tick}>0$, check multiplication overflow, then compute $n=\lceil d f_{tick}/10^6\rceil=\lfloor(df_{tick}+999999)/10^6\rfloor$; raise a positive sub-tick duration to one tick; clamp to `portMAX_DELAY`. The backend assumes a positive configured tick rate.
- [x] **Special wait durations.** Map $d=0$ to zero ticks (the delay API yields); map `UOS_WAIT_FOREVER` to `portMAX_DELAY`.
- [x] **Periodic absolute schedule.** If $next>t_0$, wait for $next-t_0$; then advance $next\leftarrow next+period$. Let $t_1$ be the clock after waiting; if $next\le t_1$, skip missed releases with $m=\lfloor(t_1-next)/period\rfloor+1$ and, if that catch-up multiplication is representable, set $next\leftarrow next+m\,period$. The initial addition is unchecked for unsigned overflow in the current implementation.
- [x] **Static task stack conversion.** Require nonzero valid byte count and alignment; compute FreeRTOS depth $n_{words}=stack\_bytes/sizeof(StackType_t)$ after checking exact divisibility.
- [x] **Static queue storage size.** Require $item\_size>0$ and $depth>0$; verify $item\_size\cdot depth\le SIZE\_MAX$ before passing the caller's item buffer to FreeRTOS.
- [x] **Counting semaphore bounds.** Require $maximum>0$, $initial\le maximum$, and $maximum$ representable by FreeRTOS `UBaseType_t`.
- [ ] **Clock and wait contract.** Confirm configured tick rate, actual tick-period quantization, scheduler start, and backend-specific blocking/ISR rules for the selected FreeRTOS port; bare metal queue/event/task-creation calls are deliberate no-op/false stubs.

#### UMID sensor conversions, encoder, ring buffer, and health

Files: `src/umid/drivers/current.c`, `voltage.c`, `temp.c`, `pressure.c`, `imu.c`, `mag.c`, `encoder.c`, `gnss.c`, and `range.c`, plus `src/umid/health.c` and `ringbuf.c`.

- [ ] **ADC code to voltage.** For unsigned code $c$ of $N$ bits, compute $V=cV_{ref}/(2^N-1)$ (or the converter's exact denominator); include reference, gain, offset, quantization, and saturation calibration.
- [ ] **Voltage divider.** For divider resistors $R_t,R_b$, compute $V_{in}=V_{adc}(R_t+R_b)/R_b$; include tolerance and ADC input loading if required.
- [ ] **Shunt current.** Compute $I=(V_{adc}-V_0)/(G R_s)$ for amplifier gain $G$ and shunt $R_s$; apply the approved offset/gain calibration; check polarity and range.
- [ ] **Temperature conversion.** Apply the exact selected sensor datasheet transfer equation or lookup table; define units, interpolation, valid code range, and open/short detection.
- [ ] **Pressure/range conversion.** Apply the exact sensor datasheet scale/offset or nonlinear conversion; specify compensation, units, invalid range, and calibration coefficients.
- [ ] **IMU scale/bias calibration.** Compute $a_c=S_a(a_{raw}-b_a)$ and $\omega_c=S_g(\omega_{raw}-b_g)$; if axis misalignment is modeled, multiply by the declared 3-by-3 correction matrix and record frame/units.
- [ ] **Magnetometer calibration.** Compute $m_c=A_m(m_{raw}-b_m)$ with the calibrated hard-iron bias $b_m$ and soft-iron matrix $A_m$; define fit provenance and reject invalid matrices.
- [ ] **Encoder modular difference.** For modulus $M$, compute $\Delta c=(c_k-c_{k-1})\bmod M$; map values above $M/2$ to $\Delta c-M$; document ambiguity at exactly half-range.
- [ ] **Encoder position and velocity.** Compute $\theta=2\pi\Delta c_{total}/C$ for counts per revolution $C$; compute $\omega=2\pi\Delta c/(CT_s)$; include gear ratio and direction if present.
- [x] **Ring-buffer capacity and occupancy.** Accept only nonzero power-of-two capacity $N$; compute unsigned occupancy $used=head-tail$ modulo $2^{32}$; treat snapshots with $used>N$ as invalid; compute free capacity $N-used$.
- [x] **Ring-buffer write.** Compute $count=\min(length,N-used)$; for $i=0,\ldots,count-1$, copy $source_i$ to $buffer[(head+i)\mathbin{\&}(N-1)]$; publish the producer index once as $head\leftarrow head+count$ after copying.
- [x] **Ring-buffer read.** Compute $available=head-tail$ and $count=\min(length,available)$; copy $buffer[(tail+i)\mathbin{\&}(N-1)]$ into each destination element; release consumed slots by publishing $tail\leftarrow tail+count$ after copying.
- [x] **Ring-buffer peek/flush.** Peek copies the same bounded range without advancing $tail$; flush sets $tail\leftarrow head$ and requires a quiescent state with both SPSC sides stopped.
- [x] **Health aggregate status.** Initialize aggregate $s=OK$; under the registry critical section, visit each registered entry and set $s\leftarrow\max(s,status_i)$; return the maximum enum severity.
- [x] **Supervisor polling and persistence.** Convert check period as $T_{check}=1000\,check\_period_{ms}$ microseconds; skip a tick when $now-last<T_{check}$ (except the initial zero timestamp); inspect at most 32 registered entries. Reset counter to zero when an entry cannot be copied or has $status<FAULT$; otherwise increment up to threshold $H$ and call the hook exactly when the count first reaches $H$.
- [ ] **Sensor conversion candidates.** The current sensor files still contain scaffolds/TODOs. Keep ADC, voltage-divider, current, temperature, pressure, IMU, magnetometer, encoder, and GNSS conversion formulas unchecked until their corresponding source implements the specified device/calibration contract.
- [ ] **Device data blockers.** Resolve exact part number, ADC reference, register scaling, calibration, units, and datasheet revision for each sensor before calling a conversion normative.

#### UPROTO frame, checksum, and bit-packing calculations

Files: `src/uproto/ppm.c`, `dshot.c`, `sbus.c`, `crsf.c`, `mavlink.c`, `uavcan.c`, and future protocol sources.

- [x] **PPM initialization.** Require $0<min\_pulse\le max\_pulse<sync\_gap$, $0<min\_channels\le max\_channels\le16$; clear decoder state and copy the validated configuration.
- [x] **PPM sync-gap transition.** If $interval\ge sync\_gap$, consider the working frame complete iff $min\_channels\le working\_count\le max\_channels$; on completion increment sequence, copy channels/count/timestamp into the latest-frame slot, and set availability; always clear working count and enter synchronized state.
- [x] **PPM channel pulse transition.** For a non-sync interval, require $min\_pulse\le interval\le max\_pulse$; an invalid pulse clears the partial count and loses synchronization. If not synchronized, ignore the valid pulse. If channel capacity is already reached, clear count, lose synchronization, and report overflow; otherwise append the interval as the next channel pulse.
- [x] **PPM frame retrieval/reset.** Report availability only when a complete frame is pending; copy one latest frame and clear availability on successful get. Reset clears synchronization, pending frame, working channel count, and frame data.
- [ ] **DShot frame construction.** Pack throttle/command and telemetry bit; compute the protocol's 4-bit checksum over the prescribed nibbles; append checksum; map each bit to high-time $T_0$ or $T_1$ within the selected DShot rate.
- [ ] **SBUS unpacking.** Assemble each 11-bit channel from the specified adjacent payload bytes using little-endian bit offsets; extract digital channels and flags; apply the declared raw-to-unit mapping and failsafe policy.
- [ ] **CRSF CRC candidate.** Initialize CRC-8 with the protocol value; update each covered byte with the protocol polynomial/reflection; append/check CRC in exact frame order; validate length/type before payload access.
- [ ] **MAVLink X.25 checksum.** Initialize accumulator to 0xffff; for each covered header/payload byte apply the MAVLink X.25 accumulate operation; include CRC_EXTRA where the message definition requires it; compare/serialize low and high bytes in specified order.
- [ ] **UAVCAN/CAN frame.** Use the named UAVCAN revision and CAN identifier layout; compute transfer ID, tail byte, start/end/toggle bits, and CRC exactly per that revision; account for CAN-FD/classic payload limits and transport fragmentation.
- [ ] **Protocol decoding.** Validate framing, length, version, message ID, and checksum before field extraction; sign-extend signed packed fields; apply unit scale/offset; reject unknown flags and malformed sequences according to policy.
- [ ] **Protocol blocker.** Pin revision, dialect, generated message definitions, CRC coverage, bit/byte ordering, and timeout/reassembly rules; verify against official vectors or conformance tests before implementation.

#### UPAL CORDIC and hardware conversion candidates

Files: [src/upal/cordic/cordic.c](../../src/upal/cordic/cordic.c), plus ADC/DAC and timer modules where the target profile exposes numeric conversion.

- [ ] **CORDIC angle reduction.** Reduce input angle into the convergent interval using the declared $2\pi$ wrap and quadrant map; record fixed-point scale and saturation.
- [ ] **CORDIC rotation iteration.** Initialize $x_0=K_Nx_{in}$, $y_0=K_Ny_{in}$, $z_0=\theta$; for $i=0,\ldots,N-1$, choose $d_i=\operatorname{sign}(z_i)$, then $x_{i+1}=x_i-d_i y_i2^{-i}$, $y_{i+1}=y_i+d_i x_i2^{-i}$, $z_{i+1}=z_i-d_i\arctan(2^{-i})$; return $x_N,y_N$ and define the gain-compensation constant $K_N$.
- [ ] **CORDIC vectoring iteration.** Initialize $(x_0,y_0)$ from the vector; choose $d_i=\operatorname{sign}(y_i)$; update $x_{i+1}=x_i+d_i y_i2^{-i}$, $y_{i+1}=y_i-d_i x_i2^{-i}$, $z_{i+1}=z_i+d_i\arctan(2^{-i})$; return magnitude with gain correction and angle $z_N$.
- [ ] **ADC/DAC code mapping.** For target resolution $N$, derive code from full-scale voltage/current using the device transfer function; define endpoint, signed offset-binary/two's-complement, reference, gain, and saturation behavior from the exact MCU reference manual.
- [ ] **Timer frequency conversion.** Compute timer ticks $n=\operatorname{round}(f_{timer}t)$ or reload value from prescaler/ARR; account for inclusive count ($ARR+1$) and prove the resulting frequency/error bound.
- [ ] **Hardware blocker.** Register-level scaling and CORDIC peripheral iteration/latency vary by MCU; use the selected target reference manual and errata, not a portable guessed equation.

#### UAPP state and lifecycle transitions

Files: `src/uapp/fault.c`, `lifecycle.c`, `statemachine.c`, and `supervisor.c`.

- [x] **Queued state-machine dispatch.** Validate configured state/transition bounds and reject rows shadowed by an earlier unconditional duplicate; call initial entry; copy bounded event payloads into a fixed FIFO with drop-new overflow reported to the producer; process at most one event per call; use declaration order for guarded transitions; run exit/action/commit/entry in order; reject recursive processing; consume and report unhandled events; latch callback failures, expose the failure status, and clear pending events.
- [x] **Component initialization/start.** Permit init only from CREATED and start only from INITIALISED; publish INITIALISING/STARTING before callbacks; reach INITIALISED/RUNNING only on success; callback failures or in-callback fault reports remain FAULTED.
- [x] **Component stop/fault.** For RUNNING stop, publish STOPPING, invoke the status-returning stop callback, then set INITIALISED. Preserve the first fault code, set FAULTED before the callback, and never overwrite a fault during callback completion; recovery policy belongs to the caller.
- [x] **Supervisor threshold trigger.** The per-health-entry consecutive-fault threshold and one-shot crossing callback are implemented as described in the UMID health section; this source does not implement quorum/deadline aggregation.
- [x] **Fatal fault persistence.** Write fault code, zero PC/LR placeholders, and reset cause to backup storage; request system reset; call `abort()` if the reset call returns. Log accessors read the stored registers.
- [x] **Nonfatal fault queue and dispatch.** `uapp_fault_report` now copies bounded records into a fixed RAM queue, counts dropped-newest events, and exposes task-context pop/dispatch hooks. Records are not retained across reset.
- [ ] **Retained crash record and lifecycle dependency graph.** Architecture-captured PC/LR/register/fault-syndrome data, atomic retained commit, acknowledgement/clear, cross-component dependency ordering/rollback, and broader fault aggregation remain under `include/uapp/future/FAULT_HISTORY.md`.
- [ ] **Application blocker.** No generic application equation exists: record concrete state transitions, watchdog thresholds, and safe-state behavior per product.

#### HAL, platform, peripheral files without portable equations

The remaining `src/uhal/` and `src/upal/` GPIO, IRQ, clock, memory, DMA, serial, bus, flash, watchdog, and target-profile files are hardware register/ABI contracts. For each one, [ ] check the selected chip's reference manual, register scaling, timing, alignment, clock tree, and errata; use the target-specific equation only where the driver converts physical units or deadlines. No cross-target equation can be written without inventing hardware behavior.

## File-by-file index

Every UETD C source is listed here, with its public header linked where one exists. Follow the source link to the calculation checklist, or to the target-specific contract note when no portable equation can be stated.

### UMATH modules

- [src/umath/derivatives/dual.c](../../src/umath/derivatives/dual.c); public header [include/umath/derivatives/dual.h](../../include/umath/derivatives/dual.h) — [calculation steps](#automatic-differentiation-dual-numbers).
- [src/umath/complex.c](../../src/umath/complex.c); public header [include/umath/complex.h](../../include/umath/complex.h) — [calculation steps](#complex-arithmetic).
- [src/umath/derivatives/jacobian.c](../../src/umath/derivatives/jacobian.c); public header [include/umath/derivatives/jacobian.h](../../include/umath/derivatives/jacobian.h) — [calculation steps](#finite-difference-jacobians).
- [src/umath/fixed.c](../../src/umath/fixed.c); public header [include/umath/fixed.h](../../include/umath/fixed.h) — [calculation steps](#fixed-point-arithmetic).
- [src/umath/future/matrix/sparse_csr.c](../../src/umath/future/matrix/sparse_csr.c); public header [include/umath/future/matrix/sparse_csr.h](../../include/umath/future/matrix/sparse_csr.h) — [calculation steps](#compressed-sparse-row-matrices).
- [src/umath/interpolation/splines.c](../../src/umath/interpolation/splines.c); public header [include/umath/interpolation/splines.h](../../include/umath/interpolation/splines.h) — [calculation steps](#spline-interpolation).
- [src/umath/lie/se2.c](../../src/umath/lie/se2.c); public header [include/umath/lie/se2.h](../../include/umath/lie/se2.h) — [calculation steps](#se2).
- [src/umath/lie/se3.c](../../src/umath/lie/se3.c); public header [include/umath/lie/se3.h](../../include/umath/lie/se3.h) — [calculation steps](#se3).
- [src/umath/lie/so2.c](../../src/umath/lie/so2.c); public header [include/umath/lie/so2.h](../../include/umath/lie/so2.h) — [calculation steps](#so2).
- [src/umath/lie/so3.c](../../src/umath/lie/so3.c); public header [include/umath/lie/so3.h](../../include/umath/lie/so3.h) — [calculation steps](#so3).
- [src/umath/matrix/core.c](../../src/umath/matrix/core.c); public header [include/umath/matrix/core.h](../../include/umath/matrix/core.h) — [calculation steps](#dense-matrix-operations).
- [src/umath/matrix/cholesky.c](../../src/umath/matrix/cholesky.c); public header [include/umath/matrix/cholesky.h](../../include/umath/matrix/cholesky.h) — [calculation steps](#cholesky-factorization-and-solve).
- [src/umath/matrix/ldlt.c](../../src/umath/matrix/ldlt.c); public header [include/umath/matrix/ldlt.h](../../include/umath/matrix/ldlt.h) — [calculation steps](#pivoted-ldl-factorization).
- [src/umath/matrix/lu.c](../../src/umath/matrix/lu.c); public header [include/umath/matrix/lu.h](../../include/umath/matrix/lu.h) — [calculation steps](#pivoted-lu-factorization).
- [src/umath/matrix/qr.c](../../src/umath/matrix/qr.c); public header [include/umath/matrix/qr.h](../../include/umath/matrix/qr.h) — [calculation steps](#householder-qr-factorization).
- [src/umath/matrix/svd.c](../../src/umath/matrix/svd.c); public header [include/umath/matrix/svd.h](../../include/umath/matrix/svd.h) — [calculation steps](#one-sided-jacobi-singular-value-decomposition).
- [src/umath/matrix/symmetric_eigen.c](../../src/umath/matrix/symmetric_eigen.c); public header [include/umath/matrix/symmetric_eigen.h](../../include/umath/matrix/symmetric_eigen.h) — [calculation steps](#symmetric-jacobi-eigensolver).
- [src/umath/numerical/integration.c](../../src/umath/numerical/integration.c); public header [include/umath/numerical/integration.h](../../include/umath/numerical/integration.h) — [calculation steps](#numerical-integration-ode-steps).
- [src/umath/optimization/least_squares.c](../../src/umath/optimization/least_squares.c); public header [include/umath/optimization/least_squares.h](../../include/umath/optimization/least_squares.h) — [calculation steps](#least-squares-solvers).
- [src/umath/polynomial/core.c](../../src/umath/polynomial/core.c); public header [include/umath/polynomial/core.h](../../include/umath/polynomial/core.h) — [calculation steps](#polynomial-evaluation-and-calculus).
- [src/umath/polynomial/roots.c](../../src/umath/polynomial/roots.c); public header [include/umath/polynomial/roots.h](../../include/umath/polynomial/roots.h) — [calculation steps](#polynomial-roots).
- [src/umath/probability/distributions.c](../../src/umath/probability/distributions.c); public header [include/umath/probability/distributions.h](../../include/umath/probability/distributions.h) — [calculation steps](#scalar-probability-distributions).
- [src/umath/probability/multivariate_normal.c](../../src/umath/probability/multivariate_normal.c); public header [include/umath/probability/multivariate_normal.h](../../include/umath/probability/multivariate_normal.h) — [calculation steps](#multivariate-normal-distribution).
- [src/umath/geometry/quaternion.c](../../src/umath/geometry/quaternion.c); public header [include/umath/geometry/quaternion.h](../../include/umath/geometry/quaternion.h) — [calculation steps](#quaternion-operations).
- [src/umath/random/prng.c](../../src/umath/random/prng.c); public header [include/umath/random/prng.h](../../include/umath/random/prng.h) — [calculation steps](#pseudorandom-number-generation).
- [src/umath/scalar.c](../../src/umath/scalar.c); public header [include/umath/scalar.h](../../include/umath/scalar.h) — [calculation steps](#scalar-helpers-current-api).
- [src/umath/signal/fft.c](../../src/umath/signal/fft.c); public header [include/umath/signal/fft.h](../../include/umath/signal/fft.h) — [calculation steps](#fast-fourier-transform).
- [src/umath/signal/windows.c](../../src/umath/signal/windows.c); public header [include/umath/signal/windows.h](../../include/umath/signal/windows.h) — [calculation steps](#window-functions).
- [src/umath/special/functions.c](../../src/umath/special/functions.c); public header [include/umath/special/functions.h](../../include/umath/special/functions.h) — [calculation steps](#special-functions).
- [src/umath/statistics/streaming.c](../../src/umath/statistics/streaming.c); public header [include/umath/statistics/streaming.h](../../include/umath/statistics/streaming.h) — [calculation steps](#streaming-statistics).
- [src/umath/status.c](../../src/umath/status.c); public header [include/umath/status.h](../../include/umath/status.h) — [calculation steps](#shared-implementation-and-promotion-checklist).
- [src/umath/geometry/transform3.c](../../src/umath/geometry/transform3.c); public header [include/umath/geometry/transform3.h](../../include/umath/geometry/transform3.h) — [calculation steps](#rigid-transforms).
- [src/umath/vector/vector.c](../../src/umath/vector/vector.c); public header [include/umath/vector/vector.h](../../include/umath/vector/vector.h) — [calculation steps](#generic-vector-operations).
- [src/umath/vector/vector2.c](../../src/umath/vector/vector2.c); public header [include/umath/vector/vector2.h](../../include/umath/vector/vector2.h) — [calculation steps](#typed-2d-3d-and-4d-vector-operations).
- [src/umath/vector/vector3.c](../../src/umath/vector/vector3.c); public header [include/umath/vector/vector3.h](../../include/umath/vector/vector3.h) — [calculation steps](#typed-2d-3d-and-4d-vector-operations).
- [src/umath/vector/vector4.c](../../src/umath/vector/vector4.c); public header [include/umath/vector/vector4.h](../../include/umath/vector/vector4.h) — [calculation steps](#typed-2d-3d-and-4d-vector-operations).

### UCON modules

PID and lead-lag are the only registered UCON algorithm implementations. Other UCON modules remain fail-closed scaffolds; review the candidate steps and contract blockers before implementation.

- [src/ucon/aerospace/attitude/bdot_detumble.c](../../src/ucon/aerospace/attitude/bdot_detumble.c) - module `ucon/aerospace/attitude/bdot_detumble`; header [include/ucon/aerospace/attitude/bdot_detumble.h](../../include/ucon/aerospace/attitude/bdot_detumble.h) - attitude error/kinematics and actuator/magnetic model. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/aerospace/attitude/quaternion_pd.c](../../src/ucon/aerospace/attitude/quaternion_pd.c) - module `ucon/aerospace/attitude/quaternion_pd`; header [include/ucon/aerospace/attitude/quaternion_pd.h](../../include/ucon/aerospace/attitude/quaternion_pd.h) - attitude error/kinematics and actuator/magnetic model. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/aerospace/attitude/so3_attitude.c](../../src/ucon/aerospace/attitude/so3_attitude.c) - module `ucon/aerospace/attitude/so3_attitude`; header [include/ucon/aerospace/attitude/so3_attitude.h](../../include/ucon/aerospace/attitude/so3_attitude.h) - attitude error/kinematics and actuator/magnetic model. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/aerospace/guidance/path_following_guidance.c](../../src/ucon/aerospace/guidance/path_following_guidance.c) - module `ucon/aerospace/guidance/path_following_guidance`; header [include/ucon/aerospace/guidance/path_following_guidance.h](../../include/ucon/aerospace/guidance/path_following_guidance.h) - guidance geometry, lookahead/LOS, frame and saturation. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/aerospace/guidance/pn_guidance.c](../../src/ucon/aerospace/guidance/pn_guidance.c) - module `ucon/aerospace/guidance/pn_guidance`; header [include/ucon/aerospace/guidance/pn_guidance.h](../../include/ucon/aerospace/guidance/pn_guidance.h) - guidance geometry, lookahead/LOS, frame and saturation. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/aerospace/navigation/gnss_carrier_fll.c](../../src/ucon/aerospace/navigation/gnss_carrier_fll.c) - module `ucon/aerospace/navigation/gnss_carrier_fll`; header [include/ucon/aerospace/navigation/gnss_carrier_fll.h](../../include/ucon/aerospace/navigation/gnss_carrier_fll.h) - INS propagation and GNSS measurement/discriminator model. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/aerospace/navigation/gnss_carrier_pll.c](../../src/ucon/aerospace/navigation/gnss_carrier_pll.c) - module `ucon/aerospace/navigation/gnss_carrier_pll`; header [include/ucon/aerospace/navigation/gnss_carrier_pll.h](../../include/ucon/aerospace/navigation/gnss_carrier_pll.h) - INS propagation and GNSS measurement/discriminator model. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/aerospace/navigation/gnss_code_dll.c](../../src/ucon/aerospace/navigation/gnss_code_dll.c) - module `ucon/aerospace/navigation/gnss_code_dll`; header [include/ucon/aerospace/navigation/gnss_code_dll.h](../../include/ucon/aerospace/navigation/gnss_code_dll.h) - INS propagation and GNSS measurement/discriminator model. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/aerospace/navigation/ins_gnss.c](../../src/ucon/aerospace/navigation/ins_gnss.c) - module `ucon/aerospace/navigation/ins_gnss`; header [include/ucon/aerospace/navigation/ins_gnss.h](../../include/ucon/aerospace/navigation/ins_gnss.h) - INS propagation and GNSS measurement/discriminator model. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/aerospace/navigation/ins_gnss_tight.c](../../src/ucon/aerospace/navigation/ins_gnss_tight.c) - module `ucon/aerospace/navigation/ins_gnss_tight`; header [include/ucon/aerospace/navigation/ins_gnss_tight.h](../../include/ucon/aerospace/navigation/ins_gnss_tight.h) - INS propagation and GNSS measurement/discriminator model. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/control/adaptive/extremum_seek.c](../../src/ucon/control/adaptive/extremum_seek.c) - module `ucon/control/adaptive/extremum_seek`; header [include/ucon/control/adaptive/extremum_seek.h](../../include/ucon/control/adaptive/extremum_seek.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/adaptive/self_tune_reg.c](../../src/ucon/control/adaptive/self_tune_reg.c) - module `ucon/control/adaptive/self_tune_reg`; header [include/ucon/control/adaptive/self_tune_reg.h](../../include/ucon/control/adaptive/self_tune_reg.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/allocation/control_allocator.c](../../src/ucon/control/allocation/control_allocator.c) - module `ucon/control/allocation/control_allocator`; header [include/ucon/control/allocation/control_allocator.h](../../include/ucon/control/allocation/control_allocator.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/classical/imc.c](../../src/ucon/control/classical/imc.c) - module `ucon/control/classical/imc`; header [include/ucon/control/classical/imc.h](../../include/ucon/control/classical/imc.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/classical/smith_predictor.c](../../src/ucon/control/classical/smith_predictor.c) - module `ucon/control/classical/smith_predictor`; header [include/ucon/control/classical/smith_predictor.h](../../include/ucon/control/classical/smith_predictor.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/disturbance_rejection/adrc.c](../../src/ucon/control/disturbance_rejection/adrc.c) - module `ucon/control/disturbance_rejection/adrc`; header [include/ucon/control/disturbance_rejection/adrc.h](../../include/ucon/control/disturbance_rejection/adrc.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/classical/lead_lag.c](../../src/ucon/control/classical/lead_lag.c) - module `ucon/control/classical/lead_lag`; header [include/ucon/control/classical/lead_lag.h](../../include/ucon/control/classical/lead_lag.h) - bilinear lead-lag recurrence including static gain. [Formula steps](#lead-lag-spec-part-1682).
- [src/ucon/control/nonlinear/adaptive_bs.c](../../src/ucon/control/nonlinear/adaptive_bs.c) - module `ucon/control/nonlinear/adaptive_bs`; header [include/ucon/control/nonlinear/adaptive_bs.h](../../include/ucon/control/nonlinear/adaptive_bs.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/backstepping.c](../../src/ucon/control/nonlinear/backstepping.c) - module `ucon/control/nonlinear/backstepping`; header [include/ucon/control/nonlinear/backstepping.h](../../include/ucon/control/nonlinear/backstepping.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/cbf_qp.c](../../src/ucon/control/nonlinear/cbf_qp.c) - module `ucon/control/nonlinear/cbf_qp`; header [include/ucon/control/nonlinear/cbf_qp.h](../../include/ucon/control/nonlinear/cbf_qp.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/clf_cbf.c](../../src/ucon/control/nonlinear/clf_cbf.c) - module `ucon/control/nonlinear/clf_cbf`; header [include/ucon/control/nonlinear/clf_cbf.h](../../include/ucon/control/nonlinear/clf_cbf.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/clf_qp.c](../../src/ucon/control/nonlinear/clf_qp.c) - module `ucon/control/nonlinear/clf_qp`; header [include/ucon/control/nonlinear/clf_qp.h](../../include/ucon/control/nonlinear/clf_qp.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/computed_torque.c](../../src/ucon/control/nonlinear/computed_torque.c) - module `ucon/control/nonlinear/computed_torque`; header [include/ucon/control/nonlinear/computed_torque.h](../../include/ucon/control/nonlinear/computed_torque.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/feedback_lin.c](../../src/ucon/control/nonlinear/feedback_lin.c) - module `ucon/control/nonlinear/feedback_lin`; header [include/ucon/control/nonlinear/feedback_lin.h](../../include/ucon/control/nonlinear/feedback_lin.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/flatness.c](../../src/ucon/control/nonlinear/flatness.c) - module `ucon/control/nonlinear/flatness`; header [include/ucon/control/nonlinear/flatness.h](../../include/ucon/control/nonlinear/flatness.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/indi.c](../../src/ucon/control/nonlinear/indi.c) - module `ucon/control/nonlinear/indi`; header [include/ucon/control/nonlinear/indi.h](../../include/ucon/control/nonlinear/indi.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/indi2.c](../../src/ucon/control/nonlinear/indi2.c) - module `ucon/control/nonlinear/indi2`; header [include/ucon/control/nonlinear/indi2.h](../../include/ucon/control/nonlinear/indi2.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/nonlinear/ndi.c](../../src/ucon/control/nonlinear/ndi.c) - module `ucon/control/nonlinear/ndi`; header [include/ucon/control/nonlinear/ndi.h](../../include/ucon/control/nonlinear/ndi.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/control/classical/pid.c](../../src/ucon/control/classical/pid.c) - module `ucon/control/classical/pid`; header [include/ucon/control/classical/pid.h](../../include/ucon/control/classical/pid.h) - two-degree-of-freedom PID, saturation, selected anti-windup. [Formula steps](#two-degree-of-freedom-pid-spec-part-1681).
- [src/ucon/control/predictive/ddp.c](../../src/ucon/control/predictive/ddp.c) - module `ucon/control/predictive/ddp`; header [include/ucon/control/predictive/ddp.h](../../include/ucon/control/predictive/ddp.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/deadbeat_mpc.c](../../src/ucon/control/predictive/deadbeat_mpc.c) - module `ucon/control/predictive/deadbeat_mpc`; header [include/ucon/control/predictive/deadbeat_mpc.h](../../include/ucon/control/predictive/deadbeat_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/economic_mpc.c](../../src/ucon/control/predictive/economic_mpc.c) - module `ucon/control/predictive/economic_mpc`; header [include/ucon/control/predictive/economic_mpc.h](../../include/ucon/control/predictive/economic_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/explicit_mpc.c](../../src/ucon/control/predictive/explicit_mpc.c) - module `ucon/control/predictive/explicit_mpc`; header [include/ucon/control/predictive/explicit_mpc.h](../../include/ucon/control/predictive/explicit_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/fs_mpc.c](../../src/ucon/control/predictive/fs_mpc.c) - module `ucon/control/predictive/fs_mpc`; header [include/ucon/control/predictive/fs_mpc.h](../../include/ucon/control/predictive/fs_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/ilqr.c](../../src/ucon/control/predictive/ilqr.c) - module `ucon/control/predictive/ilqr`; header [include/ucon/control/predictive/ilqr.h](../../include/ucon/control/predictive/ilqr.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/linear_mpc.c](../../src/ucon/control/predictive/linear_mpc.c) - module `ucon/control/predictive/linear_mpc`; header [include/ucon/control/predictive/linear_mpc.h](../../include/ucon/control/predictive/linear_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/nmpc_rti.c](../../src/ucon/control/predictive/nmpc_rti.c) - module `ucon/control/predictive/nmpc_rti`; header [include/ucon/control/predictive/nmpc_rti.h](../../include/ucon/control/predictive/nmpc_rti.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/robust_mpc.c](../../src/ucon/control/predictive/robust_mpc.c) - module `ucon/control/predictive/robust_mpc`; header [include/ucon/control/predictive/robust_mpc.h](../../include/ucon/control/predictive/robust_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/sparse_mpc.c](../../src/ucon/control/predictive/sparse_mpc.c) - module `ucon/control/predictive/sparse_mpc`; header [include/ucon/control/predictive/sparse_mpc.h](../../include/ucon/control/predictive/sparse_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/stochastic_mpc.c](../../src/ucon/control/predictive/stochastic_mpc.c) - module `ucon/control/predictive/stochastic_mpc`; header [include/ucon/control/predictive/stochastic_mpc.h](../../include/ucon/control/predictive/stochastic_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/tracking_mpc.c](../../src/ucon/control/predictive/tracking_mpc.c) - module `ucon/control/predictive/tracking_mpc`; header [include/ucon/control/predictive/tracking_mpc.h](../../include/ucon/control/predictive/tracking_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/predictive/zone_mpc.c](../../src/ucon/control/predictive/zone_mpc.c) - module `ucon/control/predictive/zone_mpc`; header [include/ucon/control/predictive/zone_mpc.h](../../include/ucon/control/predictive/zone_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/control/safety/reference_governor.c](../../src/ucon/control/safety/reference_governor.c) - module `ucon/control/safety/reference_governor`; header [include/ucon/control/safety/reference_governor.h](../../include/ucon/control/safety/reference_governor.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/scheduled/gain_schedule.c](../../src/ucon/control/scheduled/gain_schedule.c) - module `ucon/control/scheduled/gain_schedule`; header [include/ucon/control/scheduled/gain_schedule.h](../../include/ucon/control/scheduled/gain_schedule.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/control/state_space/decoupler.c](../../src/ucon/control/state_space/decoupler.c) - module `ucon/control/state_space/decoupler`; header [include/ucon/control/state_space/decoupler.h](../../include/ucon/control/state_space/decoupler.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/control/state_space/lqg.c](../../src/ucon/control/state_space/lqg.c) - module `ucon/control/state_space/lqg`; header [include/ucon/control/state_space/lqg.h](../../include/ucon/control/state_space/lqg.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/control/state_space/lqi.c](../../src/ucon/control/state_space/lqi.c) - module `ucon/control/state_space/lqi`; header [include/ucon/control/state_space/lqi.h](../../include/ucon/control/state_space/lqi.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/control/state_space/lqr.c](../../src/ucon/control/state_space/lqr.c) - module `ucon/control/state_space/lqr`; header [include/ucon/control/state_space/lqr.h](../../include/ucon/control/state_space/lqr.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/control/state_space/state_feedback.c](../../src/ucon/control/state_space/state_feedback.c) - module `ucon/control/state_space/state_feedback`; header [include/ucon/control/state_space/state_feedback.h](../../include/ucon/control/state_space/state_feedback.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/control/tuning/pid_autotune.c](../../src/ucon/control/tuning/pid_autotune.c) - module `ucon/control/tuning/pid_autotune`; header [include/ucon/control/tuning/pid_autotune.h](../../include/ucon/control/tuning/pid_autotune.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/status.c](../../src/ucon/status.c) - module `ucon/core`; header [include/ucon/status.h](../../include/ucon/status.h) - shared scalar/status helpers; no controller equation. [Formula steps](#additional-uetd-files-with-equations-or-algorithms).
- [src/ucon/electrical/grid/droop_control.c](../../src/ucon/electrical/grid/droop_control.c) - module `ucon/electrical/grid/droop_control`; header [include/ucon/electrical/grid/droop_control.h](../../include/ucon/electrical/grid/droop_control.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/grid/resonant_pr.c](../../src/ucon/electrical/grid/resonant_pr.c) - module `ucon/electrical/grid/resonant_pr`; header [include/ucon/electrical/grid/resonant_pr.h](../../include/ucon/electrical/grid/resonant_pr.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/grid/sogi_fll.c](../../src/ucon/electrical/grid/sogi_fll.c) - module `ucon/electrical/grid/sogi_fll`; header [include/ucon/electrical/grid/sogi_fll.h](../../include/ucon/electrical/grid/sogi_fll.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/grid/sogi_pll.c](../../src/ucon/electrical/grid/sogi_pll.c) - module `ucon/electrical/grid/sogi_pll`; header [include/ucon/electrical/grid/sogi_pll.h](../../include/ucon/electrical/grid/sogi_pll.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/grid/srf_pll.c](../../src/ucon/electrical/grid/srf_pll.c) - module `ucon/electrical/grid/srf_pll`; header [include/ucon/electrical/grid/srf_pll.h](../../include/ucon/electrical/grid/srf_pll.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/motor_control/dtc_control.c](../../src/ucon/electrical/motor_control/dtc_control.c) - module `ucon/electrical/motor_control/dtc_control`; header [include/ucon/electrical/motor_control/dtc_control.h](../../include/ucon/electrical/motor_control/dtc_control.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/motor_control/dtc_flux_torque_estimator.c](../../src/ucon/electrical/motor_control/dtc_flux_torque_estimator.c) - module `ucon/electrical/motor_control/dtc_flux_torque_estimator`; header [include/ucon/electrical/motor_control/dtc_flux_torque_estimator.h](../../include/ucon/electrical/motor_control/dtc_flux_torque_estimator.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/motor_control/foc_current_control.c](../../src/ucon/electrical/motor_control/foc_current_control.c) - module `ucon/electrical/motor_control/foc_current_control`; header [include/ucon/electrical/motor_control/foc_current_control.h](../../include/ucon/electrical/motor_control/foc_current_control.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/motor_control/svpwm.c](../../src/ucon/electrical/motor_control/svpwm.c) - module `ucon/electrical/motor_control/svpwm`; header [include/ucon/electrical/motor_control/svpwm.h](../../include/ucon/electrical/motor_control/svpwm.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/electrical/transforms/clarke_park.c](../../src/ucon/electrical/transforms/clarke_park.c) - module `ucon/electrical/transforms/clarke_park`; header [include/ucon/electrical/transforms/clarke_park.h](../../include/ucon/electrical/transforms/clarke_park.h) - Clarke/Park forward and inverse scaling/phase order. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/estimators/attitude/complementary.c](../../src/ucon/estimators/attitude/complementary.c) - module `ucon/estimators/attitude/complementary`; header [include/ucon/estimators/attitude/complementary.h](../../include/ucon/estimators/attitude/complementary.h) - attitude propagation/residual/correction and frame/quaternion convention. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/attitude/madgwick.c](../../src/ucon/estimators/attitude/madgwick.c) - module `ucon/estimators/attitude/madgwick`; header [include/ucon/estimators/attitude/madgwick.h](../../include/ucon/estimators/attitude/madgwick.h) - attitude propagation/residual/correction and frame/quaternion convention. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/attitude/mahony.c](../../src/ucon/estimators/attitude/mahony.c) - module `ucon/estimators/attitude/mahony`; header [include/ucon/estimators/attitude/mahony.h](../../include/ucon/estimators/attitude/mahony.h) - attitude propagation/residual/correction and frame/quaternion convention. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/attitude/mekf.c](../../src/ucon/estimators/attitude/mekf.c) - module `ucon/estimators/attitude/mekf`; header [include/ucon/estimators/attitude/mekf.h](../../include/ucon/estimators/attitude/mekf.h) - attitude propagation/residual/correction and frame/quaternion convention. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/attitude/quest.c](../../src/ucon/estimators/attitude/quest.c) - module `ucon/estimators/attitude/quest`; header [include/ucon/estimators/attitude/quest.h](../../include/ucon/estimators/attitude/quest.h) - attitude propagation/residual/correction and frame/quaternion convention. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/attitude/triad.c](../../src/ucon/estimators/attitude/triad.c) - module `ucon/estimators/attitude/triad`; header [include/ucon/estimators/attitude/triad.h](../../include/ucon/estimators/attitude/triad.h) - attitude propagation/residual/correction and frame/quaternion convention. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/bayesian/particle.c](../../src/ucon/estimators/bayesian/particle.c) - module `ucon/estimators/bayesian/particle`; header [include/ucon/estimators/bayesian/particle.h](../../include/ucon/estimators/bayesian/particle.h) - particle likelihood, normalized weights, resampling, PRNG. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/estimators/frequency/fll.c](../../src/ucon/estimators/frequency/fll.c) - module `ucon/estimators/frequency/fll`; header [include/ucon/estimators/frequency/fll.h](../../include/ucon/estimators/frequency/fll.h) - phase/frequency detector, loop/filter recurrence, wrap/lock policy. [Formula steps](#pllfllnco-sogi-dft-and-zero-crossing).
- [src/ucon/estimators/frequency/nco.c](../../src/ucon/estimators/frequency/nco.c) - module `ucon/estimators/frequency/nco`; header [include/ucon/estimators/frequency/nco.h](../../include/ucon/estimators/frequency/nco.h) - phase/frequency detector, loop/filter recurrence, wrap/lock policy. [Formula steps](#pllfllnco-sogi-dft-and-zero-crossing).
- [src/ucon/estimators/frequency/pll.c](../../src/ucon/estimators/frequency/pll.c) - module `ucon/estimators/frequency/pll`; header [include/ucon/estimators/frequency/pll.h](../../include/ucon/estimators/frequency/pll.h) - phase/frequency detector, loop/filter recurrence, wrap/lock policy. [Formula steps](#pllfllnco-sogi-dft-and-zero-crossing).
- [src/ucon/estimators/frequency/sliding_dft.c](../../src/ucon/estimators/frequency/sliding_dft.c) - module `ucon/estimators/frequency/sliding_dft`; header [include/ucon/estimators/frequency/sliding_dft.h](../../include/ucon/estimators/frequency/sliding_dft.h) - phase/frequency detector, loop/filter recurrence, wrap/lock policy. [Formula steps](#pllfllnco-sogi-dft-and-zero-crossing).
- [src/ucon/estimators/frequency/sogi.c](../../src/ucon/estimators/frequency/sogi.c) - module `ucon/estimators/frequency/sogi`; header [include/ucon/estimators/frequency/sogi.h](../../include/ucon/estimators/frequency/sogi.h) - phase/frequency detector, loop/filter recurrence, wrap/lock policy. [Formula steps](#pllfllnco-sogi-dft-and-zero-crossing).
- [src/ucon/estimators/frequency/zero_cross_freq.c](../../src/ucon/estimators/frequency/zero_cross_freq.c) - module `ucon/estimators/frequency/zero_cross_freq`; header [include/ucon/estimators/frequency/zero_cross_freq.h](../../include/ucon/estimators/frequency/zero_cross_freq.h) - phase/frequency detector, loop/filter recurrence, wrap/lock policy. [Formula steps](#pllfllnco-sogi-dft-and-zero-crossing).
- [src/ucon/estimators/identification/rls.c](../../src/ucon/estimators/identification/rls.c) - module `ucon/estimators/identification/rls`; header [include/ucon/estimators/identification/rls.h](../../include/ucon/estimators/identification/rls.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/kalman/ckf.c](../../src/ucon/estimators/kalman/ckf.c) - module `ucon/estimators/kalman/ckf`; header [include/ucon/estimators/kalman/ckf.h](../../include/ucon/estimators/kalman/ckf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#ukf-merwe-ukf-ckf-imm-particle-mhe).
- [src/ucon/estimators/kalman/ekf.c](../../src/ucon/estimators/kalman/ekf.c) - module `ucon/estimators/kalman/ekf`; header [include/ucon/estimators/kalman/ekf.h](../../include/ucon/estimators/kalman/ekf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/error_state_ekf.c](../../src/ucon/estimators/kalman/error_state_ekf.c) - module `ucon/estimators/kalman/error_state_ekf`; header [include/ucon/estimators/kalman/error_state_ekf.h](../../include/ucon/estimators/kalman/error_state_ekf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/imm_ekf.c](../../src/ucon/estimators/kalman/imm_ekf.c) - module `ucon/estimators/kalman/imm_ekf`; header [include/ucon/estimators/kalman/imm_ekf.h](../../include/ucon/estimators/kalman/imm_ekf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#ukf-merwe-ukf-ckf-imm-particle-mhe).
- [src/ucon/estimators/kalman/invariant_ekf.c](../../src/ucon/estimators/kalman/invariant_ekf.c) - module `ucon/estimators/kalman/invariant_ekf`; header [include/ucon/estimators/kalman/invariant_ekf.h](../../include/ucon/estimators/kalman/invariant_ekf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/iterated_ekf.c](../../src/ucon/estimators/kalman/iterated_ekf.c) - module `ucon/estimators/kalman/iterated_ekf`; header [include/ucon/estimators/kalman/iterated_ekf.h](../../include/ucon/estimators/kalman/iterated_ekf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/linear_kf.c](../../src/ucon/estimators/kalman/linear_kf.c) - module `ucon/estimators/kalman/linear_kf`; header [include/ucon/estimators/kalman/linear_kf.h](../../include/ucon/estimators/kalman/linear_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/scalar_kf.c](../../src/ucon/estimators/kalman/scalar_kf.c) - module `ucon/estimators/kalman/scalar_kf`; header [include/ucon/estimators/kalman/scalar_kf.h](../../include/ucon/estimators/kalman/scalar_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/sqrt_ekf.c](../../src/ucon/estimators/kalman/sqrt_ekf.c) - module `ucon/estimators/kalman/sqrt_ekf`; header [include/ucon/estimators/kalman/sqrt_ekf.h](../../include/ucon/estimators/kalman/sqrt_ekf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/sqrt_ukf.c](../../src/ucon/estimators/kalman/sqrt_ukf.c) - module `ucon/estimators/kalman/sqrt_ukf`; header [include/ucon/estimators/kalman/sqrt_ukf.h](../../include/ucon/estimators/kalman/sqrt_ukf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/estimators/kalman/ukf.c](../../src/ucon/estimators/kalman/ukf.c) - module `ucon/estimators/kalman/ukf`; header [include/ucon/estimators/kalman/ukf.h](../../include/ucon/estimators/kalman/ukf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#ukf-merwe-ukf-ckf-imm-particle-mhe).
- [src/ucon/estimators/kalman/ukf_merwe.c](../../src/ucon/estimators/kalman/ukf_merwe.c) - module `ucon/estimators/kalman/ukf_merwe`; header [include/ucon/estimators/kalman/ukf_merwe.h](../../include/ucon/estimators/kalman/ukf_merwe.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#ukf-merwe-ukf-ckf-imm-particle-mhe).
- [src/ucon/estimators/moving_horizon/mhe.c](../../src/ucon/estimators/moving_horizon/mhe.c) - module `ucon/estimators/moving_horizon/mhe`; header [include/ucon/estimators/moving_horizon/mhe.h](../../include/ucon/estimators/moving_horizon/mhe.h) - finite-window residual/arrival-cost constrained optimization. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/estimators/observers/dob.c](../../src/ucon/estimators/observers/dob.c) - module `ucon/estimators/observers/dob`; header [include/ucon/estimators/observers/dob.h](../../include/ucon/estimators/observers/dob.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/observers/edob.c](../../src/ucon/estimators/observers/edob.c) - module `ucon/estimators/observers/edob`; header [include/ucon/estimators/observers/edob.h](../../include/ucon/estimators/observers/edob.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/observers/eso.c](../../src/ucon/estimators/observers/eso.c) - module `ucon/estimators/observers/eso`; header [include/ucon/estimators/observers/eso.h](../../include/ucon/estimators/observers/eso.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/observers/luenberger.c](../../src/ucon/estimators/observers/luenberger.c) - module `ucon/estimators/observers/luenberger`; header [include/ucon/estimators/observers/luenberger.h](../../include/ucon/estimators/observers/luenberger.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/tracking/alpha_beta.c](../../src/ucon/estimators/tracking/alpha_beta.c) - module `ucon/estimators/tracking/alpha_beta`; header [include/ucon/estimators/tracking/alpha_beta.h](../../include/ucon/estimators/tracking/alpha_beta.h) - constant-velocity/acceleration prediction and alpha-beta(-gamma) correction. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/estimators/tracking/alpha_beta_gamma.c](../../src/ucon/estimators/tracking/alpha_beta_gamma.c) - module `ucon/estimators/tracking/alpha_beta_gamma`; header [include/ucon/estimators/tracking/alpha_beta_gamma.h](../../include/ucon/estimators/tracking/alpha_beta_gamma.h) - constant-velocity/acceleration prediction and alpha-beta(-gamma) correction. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/filtering/coefficient_design/bessel.c](../../src/ucon/filtering/coefficient_design/bessel.c) - module `ucon/filtering/coefficient_design/bessel`; header [include/ucon/filtering/coefficient_design/bessel.h](../../include/ucon/filtering/coefficient_design/bessel.h) - prototype, frequency transform, discretization, window/ripple design. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/coefficient_design/butterworth.c](../../src/ucon/filtering/coefficient_design/butterworth.c) - module `ucon/filtering/coefficient_design/butterworth`; header [include/ucon/filtering/coefficient_design/butterworth.h](../../include/ucon/filtering/coefficient_design/butterworth.h) - prototype, frequency transform, discretization, window/ripple design. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/coefficient_design/chebyshev1.c](../../src/ucon/filtering/coefficient_design/chebyshev1.c) - module `ucon/filtering/coefficient_design/chebyshev1`; header [include/ucon/filtering/coefficient_design/chebyshev1.h](../../include/ucon/filtering/coefficient_design/chebyshev1.h) - prototype, frequency transform, discretization, window/ripple design. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/coefficient_design/chebyshev2.c](../../src/ucon/filtering/coefficient_design/chebyshev2.c) - module `ucon/filtering/coefficient_design/chebyshev2`; header [include/ucon/filtering/coefficient_design/chebyshev2.h](../../include/ucon/filtering/coefficient_design/chebyshev2.h) - prototype, frequency transform, discretization, window/ripple design. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/coefficient_design/elliptic.c](../../src/ucon/filtering/coefficient_design/elliptic.c) - module `ucon/filtering/coefficient_design/elliptic`; header [include/ucon/filtering/coefficient_design/elliptic.h](../../include/ucon/filtering/coefficient_design/elliptic.h) - prototype, frequency transform, discretization, window/ripple design. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/coefficient_design/fir_windowed.c](../../src/ucon/filtering/coefficient_design/fir_windowed.c) - module `ucon/filtering/coefficient_design/fir_windowed`; header [include/ucon/filtering/coefficient_design/fir_windowed.h](../../include/ucon/filtering/coefficient_design/fir_windowed.h) - prototype, frequency transform, discretization, window/ripple design. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/runtime/biquad_cascade.c](../../src/ucon/filtering/runtime/biquad_cascade.c) - module `ucon/filtering/runtime/biquad_cascade`; header [include/ucon/filtering/runtime/biquad_cascade.h](../../include/ucon/filtering/runtime/biquad_cascade.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/runtime/ema.c](../../src/ucon/filtering/runtime/ema.c) - module `ucon/filtering/runtime/ema`; header [include/ucon/filtering/runtime/ema.h](../../include/ucon/filtering/runtime/ema.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/runtime/fir.c](../../src/ucon/filtering/runtime/fir.c) - module `ucon/filtering/runtime/fir`; header [include/ucon/filtering/runtime/fir.h](../../include/ucon/filtering/runtime/fir.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/runtime/median.c](../../src/ucon/filtering/runtime/median.c) - module `ucon/filtering/runtime/median`; header [include/ucon/filtering/runtime/median.h](../../include/ucon/filtering/runtime/median.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/runtime/moving_average.c](../../src/ucon/filtering/runtime/moving_average.c) - module `ucon/filtering/runtime/moving_average`; header [include/ucon/filtering/runtime/moving_average.h](../../include/ucon/filtering/runtime/moving_average.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/filtering/runtime/notch.c](../../src/ucon/filtering/runtime/notch.c) - module `ucon/filtering/runtime/notch`; header [include/ucon/filtering/runtime/notch.h](../../include/ucon/filtering/runtime/notch.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/future/aerospace/guidance/los_guidance.c](../../src/ucon/future/aerospace/guidance/los_guidance.c) - module `ucon/future/aerospace/guidance/los_guidance`; header [include/ucon/future/aerospace/guidance/los_guidance.h](../../include/ucon/future/aerospace/guidance/los_guidance.h) - guidance geometry, lookahead/LOS, frame and saturation. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/future/aerospace/guidance/pure_pursuit_guidance.c](../../src/ucon/future/aerospace/guidance/pure_pursuit_guidance.c) - module `ucon/future/aerospace/guidance/pure_pursuit_guidance`; header [include/ucon/future/aerospace/guidance/pure_pursuit_guidance.h](../../include/ucon/future/aerospace/guidance/pure_pursuit_guidance.h) - guidance geometry, lookahead/LOS, frame and saturation. [Formula steps](#gnssins-guidance-and-aerospace-algorithms).
- [src/ucon/future/control/adaptive/ilc.c](../../src/ucon/future/control/adaptive/ilc.c) - module `ucon/future/control/adaptive/ilc`; header [include/ucon/future/control/adaptive/ilc.h](../../include/ucon/future/control/adaptive/ilc.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/adaptive/l1_adaptive.c](../../src/ucon/future/control/adaptive/l1_adaptive.c) - module `ucon/future/control/adaptive/l1_adaptive`; header [include/ucon/future/control/adaptive/l1_adaptive.h](../../include/ucon/future/control/adaptive/l1_adaptive.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/adaptive/mrac_composite.c](../../src/ucon/future/control/adaptive/mrac_composite.c) - module `ucon/future/control/adaptive/mrac_composite`; header [include/ucon/future/control/adaptive/mrac_composite.h](../../include/ucon/future/control/adaptive/mrac_composite.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/adaptive/mrac_lyapunov.c](../../src/ucon/future/control/adaptive/mrac_lyapunov.c) - module `ucon/future/control/adaptive/mrac_lyapunov`; header [include/ucon/future/control/adaptive/mrac_lyapunov.h](../../include/ucon/future/control/adaptive/mrac_lyapunov.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/adaptive/mrac_mit.c](../../src/ucon/future/control/adaptive/mrac_mit.c) - module `ucon/future/control/adaptive/mrac_mit`; header [include/ucon/future/control/adaptive/mrac_mit.h](../../include/ucon/future/control/adaptive/mrac_mit.h) - adaptive law, parameterization, projection/leakage/excitation policy. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/classical/dahlin.c](../../src/ucon/future/control/classical/dahlin.c) - module `ucon/future/control/classical/dahlin`; header [include/ucon/future/control/classical/dahlin.h](../../include/ucon/future/control/classical/dahlin.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/classical/deadbeat.c](../../src/ucon/future/control/classical/deadbeat.c) - module `ucon/future/control/classical/deadbeat`; header [include/ucon/future/control/classical/deadbeat.h](../../include/ucon/future/control/classical/deadbeat.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/classical/repetitive.c](../../src/ucon/future/control/classical/repetitive.c) - module `ucon/future/control/classical/repetitive`; header [include/ucon/future/control/classical/repetitive.h](../../include/ucon/future/control/classical/repetitive.h) - module-specific plant, tuning, allocation, or reference-governor equations. [Formula steps](#adaptive-classical-disturbance-rejection-and-tuning).
- [src/ucon/future/control/nonlinear/dynamic_surface_control.c](../../src/ucon/future/control/nonlinear/dynamic_surface_control.c) - module `ucon/future/control/nonlinear/dynamic_surface_control`; header [include/ucon/future/control/nonlinear/dynamic_surface_control.h](../../include/ucon/future/control/nonlinear/dynamic_surface_control.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/fractional_order_smc.c](../../src/ucon/future/control/nonlinear/fractional_order_smc.c) - module `ucon/future/control/nonlinear/fractional_order_smc`; header [include/ucon/future/control/nonlinear/fractional_order_smc.h](../../include/ucon/future/control/nonlinear/fractional_order_smc.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/fuzzy_controller.c](../../src/ucon/future/control/nonlinear/fuzzy_controller.c) - module `ucon/future/control/nonlinear/fuzzy_controller`; header [include/ucon/future/control/nonlinear/fuzzy_controller.h](../../include/ucon/future/control/nonlinear/fuzzy_controller.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/ida_pbc.c](../../src/ucon/future/control/nonlinear/ida_pbc.c) - module `ucon/future/control/nonlinear/ida_pbc`; header [include/ucon/future/control/nonlinear/ida_pbc.h](../../include/ucon/future/control/nonlinear/ida_pbc.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/integral_smc.c](../../src/ucon/future/control/nonlinear/integral_smc.c) - module `ucon/future/control/nonlinear/integral_smc`; header [include/ucon/future/control/nonlinear/integral_smc.h](../../include/ucon/future/control/nonlinear/integral_smc.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/rise.c](../../src/ucon/future/control/nonlinear/rise.c) - module `ucon/future/control/nonlinear/rise`; header [include/ucon/future/control/nonlinear/rise.h](../../include/ucon/future/control/nonlinear/rise.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/smc.c](../../src/ucon/future/control/nonlinear/smc.c) - module `ucon/future/control/nonlinear/smc`; header [include/ucon/future/control/nonlinear/smc.h](../../include/ucon/future/control/nonlinear/smc.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/super_twisting.c](../../src/ucon/future/control/nonlinear/super_twisting.c) - module `ucon/future/control/nonlinear/super_twisting`; header [include/ucon/future/control/nonlinear/super_twisting.h](../../include/ucon/future/control/nonlinear/super_twisting.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/nonlinear/terminal_smc.c](../../src/ucon/future/control/nonlinear/terminal_smc.c) - module `ucon/future/control/nonlinear/terminal_smc`; header [include/ucon/future/control/nonlinear/terminal_smc.h](../../include/ucon/future/control/nonlinear/terminal_smc.h) - nonlinear model, Lyapunov/CBF/sliding/feedback law selected by module. [Formula steps](#nonlinear-feedback-clfcbf-qp-and-allocation).
- [src/ucon/future/control/predictive/adaptive_mpc.c](../../src/ucon/future/control/predictive/adaptive_mpc.c) - module `ucon/future/control/predictive/adaptive_mpc`; header [include/ucon/future/control/predictive/adaptive_mpc.h](../../include/ucon/future/control/predictive/adaptive_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/future/control/predictive/gain_scheduled_mpc.c](../../src/ucon/future/control/predictive/gain_scheduled_mpc.c) - module `ucon/future/control/predictive/gain_scheduled_mpc`; header [include/ucon/future/control/predictive/gain_scheduled_mpc.h](../../include/ucon/future/control/predictive/gain_scheduled_mpc.h) - MPC objective/model/constraints; variant policy in predictive-control section. [Formula steps](#mpc-family-spec-parts-16831686).
- [src/ucon/future/control/state_space/h_inf_of.c](../../src/ucon/future/control/state_space/h_inf_of.c) - module `ucon/future/control/state_space/h_inf_of`; header [include/ucon/future/control/state_space/h_inf_of.h](../../include/ucon/future/control/state_space/h_inf_of.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/future/control/state_space/h_inf_sf.c](../../src/ucon/future/control/state_space/h_inf_sf.c) - module `ucon/future/control/state_space/h_inf_sf`; header [include/ucon/future/control/state_space/h_inf_sf.h](../../include/ucon/future/control/state_space/h_inf_sf.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/future/control/state_space/h2_state_feedback.c](../../src/ucon/future/control/state_space/h2_state_feedback.c) - module `ucon/future/control/state_space/h2_state_feedback`; header [include/ucon/future/control/state_space/h2_state_feedback.h](../../include/ucon/future/control/state_space/h2_state_feedback.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#state-feedback-lqrlqilqg-decoupling-pole-placement).
- [src/ucon/future/control/synthesis/pole_place.c](../../src/ucon/future/control/synthesis/pole_place.c) - module `ucon/future/control/synthesis/pole_place`; header [include/ucon/future/control/synthesis/pole_place.h](../../include/ucon/future/control/synthesis/pole_place.h) - state-space law, synthesis objective/Riccati or pole-placement contract. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/electrical/grid/ddsrf_pll.c](../../src/ucon/future/electrical/grid/ddsrf_pll.c) - module `ucon/future/electrical/grid/ddsrf_pll`; header [include/ucon/future/electrical/grid/ddsrf_pll.h](../../include/ucon/future/electrical/grid/ddsrf_pll.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/grid/dsogi_pll.c](../../src/ucon/future/electrical/grid/dsogi_pll.c) - module `ucon/future/electrical/grid/dsogi_pll`; header [include/ucon/future/electrical/grid/dsogi_pll.h](../../include/ucon/future/electrical/grid/dsogi_pll.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/grid/msogi_fll.c](../../src/ucon/future/electrical/grid/msogi_fll.c) - module `ucon/future/electrical/grid/msogi_fll`; header [include/ucon/future/electrical/grid/msogi_fll.h](../../include/ucon/future/electrical/grid/msogi_fll.h) - grid model, PLL/FLL detector/filter, droop or PR recurrence. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/motor_control/flux_weakening_control.c](../../src/ucon/future/electrical/motor_control/flux_weakening_control.c) - module `ucon/future/electrical/motor_control/flux_weakening_control`; header [include/ucon/future/electrical/motor_control/flux_weakening_control.h](../../include/ucon/future/electrical/motor_control/flux_weakening_control.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/motor_control/mtpa_control.c](../../src/ucon/future/electrical/motor_control/mtpa_control.c) - module `ucon/future/electrical/motor_control/mtpa_control`; header [include/ucon/future/electrical/motor_control/mtpa_control.h](../../include/ucon/future/electrical/motor_control/mtpa_control.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/motor_control/sensorless_foc_observer.c](../../src/ucon/future/electrical/motor_control/sensorless_foc_observer.c) - module `ucon/future/electrical/motor_control/sensorless_foc_observer`; header [include/ucon/future/electrical/motor_control/sensorless_foc_observer.h](../../include/ucon/future/electrical/motor_control/sensorless_foc_observer.h) - motor model, current/flux/torque law, modulation and limits. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/power_conversion/average_current_mode_control.c](../../src/ucon/future/electrical/power_conversion/average_current_mode_control.c) - module `ucon/future/electrical/power_conversion/average_current_mode_control`; header [include/ucon/future/electrical/power_conversion/average_current_mode_control.h](../../include/ucon/future/electrical/power_conversion/average_current_mode_control.h) - topology/timing/plant/candidate action/cost; resolve blockers first. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/power_conversion/half_cycle_predictive_current_control.c](../../src/ucon/future/electrical/power_conversion/half_cycle_predictive_current_control.c) - module `ucon/future/electrical/power_conversion/half_cycle_predictive_current_control`; header [include/ucon/future/electrical/power_conversion/half_cycle_predictive_current_control.h](../../include/ucon/future/electrical/power_conversion/half_cycle_predictive_current_control.h) - topology/timing/plant/candidate action/cost; resolve blockers first. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/power_conversion/hysteretic_current_control.c](../../src/ucon/future/electrical/power_conversion/hysteretic_current_control.c) - module `ucon/future/electrical/power_conversion/hysteretic_current_control`; header [include/ucon/future/electrical/power_conversion/hysteretic_current_control.h](../../include/ucon/future/electrical/power_conversion/hysteretic_current_control.h) - topology/timing/plant/candidate action/cost; resolve blockers first. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/power_conversion/peak_current_mode_control.c](../../src/ucon/future/electrical/power_conversion/peak_current_mode_control.c) - module `ucon/future/electrical/power_conversion/peak_current_mode_control`; header [include/ucon/future/electrical/power_conversion/peak_current_mode_control.h](../../include/ucon/future/electrical/power_conversion/peak_current_mode_control.h) - topology/timing/plant/candidate action/cost; resolve blockers first. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/electrical/power_conversion/phase_shift_full_bridge_control.c](../../src/ucon/future/electrical/power_conversion/phase_shift_full_bridge_control.c) - module `ucon/future/electrical/power_conversion/phase_shift_full_bridge_control`; header [include/ucon/future/electrical/power_conversion/phase_shift_full_bridge_control.h](../../include/ucon/future/electrical/power_conversion/phase_shift_full_bridge_control.h) - topology/timing/plant/candidate action/cost; resolve blockers first. [Formula steps](#electrical-transforms-grid-and-motor-control).
- [src/ucon/future/estimators/diagnostics/cusum_change_detector.c](../../src/ucon/future/estimators/diagnostics/cusum_change_detector.c) - module `ucon/future/estimators/diagnostics/cusum_change_detector`; header [include/ucon/future/estimators/diagnostics/cusum_change_detector.h](../../include/ucon/future/estimators/diagnostics/cusum_change_detector.h) - residual/statistic, threshold, persistence, latch/reset policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/estimators/diagnostics/innovation_monitor.c](../../src/ucon/future/estimators/diagnostics/innovation_monitor.c) - module `ucon/future/estimators/diagnostics/innovation_monitor`; header [include/ucon/future/estimators/diagnostics/innovation_monitor.h](../../include/ucon/future/estimators/diagnostics/innovation_monitor.h) - residual/statistic, threshold, persistence, latch/reset policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/estimators/identification/persistent_excitation_monitor.c](../../src/ucon/future/estimators/identification/persistent_excitation_monitor.c) - module `ucon/future/estimators/identification/persistent_excitation_monitor`; header [include/ucon/future/estimators/identification/persistent_excitation_monitor.h](../../include/ucon/future/estimators/identification/persistent_excitation_monitor.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_ar.c](../../src/ucon/future/estimators/identification/recursive_ar.c) - module `ucon/future/estimators/identification/recursive_ar`; header [include/ucon/future/estimators/identification/recursive_ar.h](../../include/ucon/future/estimators/identification/recursive_ar.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_arma.c](../../src/ucon/future/estimators/identification/recursive_arma.c) - module `ucon/future/estimators/identification/recursive_arma`; header [include/ucon/future/estimators/identification/recursive_arma.h](../../include/ucon/future/estimators/identification/recursive_arma.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_armax.c](../../src/ucon/future/estimators/identification/recursive_armax.c) - module `ucon/future/estimators/identification/recursive_armax`; header [include/ucon/future/estimators/identification/recursive_armax.h](../../include/ucon/future/estimators/identification/recursive_armax.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_arx.c](../../src/ucon/future/estimators/identification/recursive_arx.c) - module `ucon/future/estimators/identification/recursive_arx`; header [include/ucon/future/estimators/identification/recursive_arx.h](../../include/ucon/future/estimators/identification/recursive_arx.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_bj.c](../../src/ucon/future/estimators/identification/recursive_bj.c) - module `ucon/future/estimators/identification/recursive_bj`; header [include/ucon/future/estimators/identification/recursive_bj.h](../../include/ucon/future/estimators/identification/recursive_bj.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_iv.c](../../src/ucon/future/estimators/identification/recursive_iv.c) - module `ucon/future/estimators/identification/recursive_iv`; header [include/ucon/future/estimators/identification/recursive_iv.h](../../include/ucon/future/estimators/identification/recursive_iv.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/recursive_oe.c](../../src/ucon/future/estimators/identification/recursive_oe.c) - module `ucon/future/estimators/identification/recursive_oe`; header [include/ucon/future/estimators/identification/recursive_oe.h](../../include/ucon/future/estimators/identification/recursive_oe.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/identification/rpem.c](../../src/ucon/future/estimators/identification/rpem.c) - module `ucon/future/estimators/identification/rpem`; header [include/ucon/future/estimators/identification/rpem.h](../../include/ucon/future/estimators/identification/rpem.h) - regressor/prediction error and RLS or specified identification update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/kalman/adaptive_kf.c](../../src/ucon/future/estimators/kalman/adaptive_kf.c) - module `ucon/future/estimators/kalman/adaptive_kf`; header [include/ucon/future/estimators/kalman/adaptive_kf.h](../../include/ucon/future/estimators/kalman/adaptive_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/dual_kf.c](../../src/ucon/future/estimators/kalman/dual_kf.c) - module `ucon/future/estimators/kalman/dual_kf`; header [include/ucon/future/estimators/kalman/dual_kf.h](../../include/ucon/future/estimators/kalman/dual_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/fading_memory_kf.c](../../src/ucon/future/estimators/kalman/fading_memory_kf.c) - module `ucon/future/estimators/kalman/fading_memory_kf`; header [include/ucon/future/estimators/kalman/fading_memory_kf.h](../../include/ucon/future/estimators/kalman/fading_memory_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/gaussian_sum_filter.c](../../src/ucon/future/estimators/kalman/gaussian_sum_filter.c) - module `ucon/future/estimators/kalman/gaussian_sum_filter`; header [include/ucon/future/estimators/kalman/gaussian_sum_filter.h](../../include/ucon/future/estimators/kalman/gaussian_sum_filter.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/out_of_sequence_kf.c](../../src/ucon/future/estimators/kalman/out_of_sequence_kf.c) - module `ucon/future/estimators/kalman/out_of_sequence_kf`; header [include/ucon/future/estimators/kalman/out_of_sequence_kf.h](../../include/ucon/future/estimators/kalman/out_of_sequence_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/power_conversion/resonant_ekf_llc_src.c](../../src/ucon/future/estimators/kalman/power_conversion/resonant_ekf_llc_src.c) - module `ucon/future/estimators/kalman/power_conversion/resonant_ekf_llc_src`; header [include/ucon/future/estimators/kalman/power_conversion/resonant_ekf_llc_src.h](../../include/ucon/future/estimators/kalman/power_conversion/resonant_ekf_llc_src.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/robust_kf.c](../../src/ucon/future/estimators/kalman/robust_kf.c) - module `ucon/future/estimators/kalman/robust_kf`; header [include/ucon/future/estimators/kalman/robust_kf.h](../../include/ucon/future/estimators/kalman/robust_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/kalman/ud_kf.c](../../src/ucon/future/estimators/kalman/ud_kf.c) - module `ucon/future/estimators/kalman/ud_kf`; header [include/ucon/future/estimators/kalman/ud_kf.h](../../include/ucon/future/estimators/kalman/ud_kf.h) - KF-family predict/update plus this variants covariance/model policy. [Formula steps](#linear-kf-scalar-kf-ekf-variants-square-root-filters).
- [src/ucon/future/estimators/observers/h_inf_obs.c](../../src/ucon/future/estimators/observers/h_inf_obs.c) - module `ucon/future/estimators/observers/h_inf_obs`; header [include/ucon/future/estimators/observers/h_inf_obs.h](../../include/ucon/future/estimators/observers/h_inf_obs.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/observers/high_gain_observer.c](../../src/ucon/future/estimators/observers/high_gain_observer.c) - module `ucon/future/estimators/observers/high_gain_observer`; header [include/ucon/future/estimators/observers/high_gain_observer.h](../../include/ucon/future/estimators/observers/high_gain_observer.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/observers/reduced_obs.c](../../src/ucon/future/estimators/observers/reduced_obs.c) - module `ucon/future/estimators/observers/reduced_obs`; header [include/ucon/future/estimators/observers/reduced_obs.h](../../include/ucon/future/estimators/observers/reduced_obs.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/observers/sliding_mode_observer.c](../../src/ucon/future/estimators/observers/sliding_mode_observer.c) - module `ucon/future/estimators/observers/sliding_mode_observer`; header [include/ucon/future/estimators/observers/sliding_mode_observer.h](../../include/ucon/future/estimators/observers/sliding_mode_observer.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/estimators/observers/uio.c](../../src/ucon/future/estimators/observers/uio.c) - module `ucon/future/estimators/observers/uio`; header [include/ucon/future/estimators/observers/uio.h](../../include/ucon/future/estimators/observers/uio.h) - observer error/disturbance model and bounded update. [Formula steps](#rls-tracking-attitude-and-observers).
- [src/ucon/future/filtering/adaptive/lms.c](../../src/ucon/future/filtering/adaptive/lms.c) - module `ucon/future/filtering/adaptive/lms`; header [include/ucon/future/filtering/adaptive/lms.h](../../include/ucon/future/filtering/adaptive/lms.h) - adaptive/filter recurrence, window/order and boundary policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/filtering/adaptive/nlms.c](../../src/ucon/future/filtering/adaptive/nlms.c) - module `ucon/future/filtering/adaptive/nlms`; header [include/ucon/future/filtering/adaptive/nlms.h](../../include/ucon/future/filtering/adaptive/nlms.h) - adaptive/filter recurrence, window/order and boundary policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/filtering/runtime/savgol_filter.c](../../src/ucon/future/filtering/runtime/savgol_filter.c) - module `ucon/future/filtering/runtime/savgol_filter`; header [include/ucon/future/filtering/runtime/savgol_filter.h](../../include/ucon/future/filtering/runtime/savgol_filter.h) - runtime convolution/recurrence/window update. [Formula steps](#runtime-filters-and-coefficient-design).
- [src/ucon/future/infrastructure/blocks/aw_observer.c](../../src/ucon/future/infrastructure/blocks/aw_observer.c) - module `ucon/future/infrastructure/blocks/aw_observer`; header [include/ucon/future/infrastructure/blocks/aw_observer.h](../../include/ucon/future/infrastructure/blocks/aw_observer.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/future/optimization/qp/qp_active_set.c](../../src/ucon/future/optimization/qp/qp_active_set.c) - module `ucon/future/optimization/qp/qp_active_set`; header [include/ucon/future/optimization/qp/qp_active_set.h](../../include/ucon/future/optimization/qp/qp_active_set.h) - QP objective, constraints, solver update and termination. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/optimization/qp/qp_admm.c](../../src/ucon/future/optimization/qp/qp_admm.c) - module `ucon/future/optimization/qp/qp_admm`; header [include/ucon/future/optimization/qp/qp_admm.h](../../include/ucon/future/optimization/qp/qp_admm.h) - QP objective, constraints, solver update and termination. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/optimization/qp/qp_projected_gradient.c](../../src/ucon/future/optimization/qp/qp_projected_gradient.c) - module `ucon/future/optimization/qp/qp_projected_gradient`; header [include/ucon/future/optimization/qp/qp_projected_gradient.h](../../include/ucon/future/optimization/qp/qp_projected_gradient.h) - QP objective, constraints, solver update and termination. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/analysis/spectral_peak_interpolation.c](../../src/ucon/future/signal_processing/analysis/spectral_peak_interpolation.c) - module `ucon/future/signal_processing/analysis/spectral_peak_interpolation`; header [include/ucon/future/signal_processing/analysis/spectral_peak_interpolation.h](../../include/ucon/future/signal_processing/analysis/spectral_peak_interpolation.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/demodulation/synchronous_demod.c](../../src/ucon/future/signal_processing/demodulation/synchronous_demod.c) - module `ucon/future/signal_processing/demodulation/synchronous_demod`; header [include/ucon/future/signal_processing/demodulation/synchronous_demod.h](../../include/ucon/future/signal_processing/demodulation/synchronous_demod.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/resampling/cic_decimator.c](../../src/ucon/future/signal_processing/resampling/cic_decimator.c) - module `ucon/future/signal_processing/resampling/cic_decimator`; header [include/ucon/future/signal_processing/resampling/cic_decimator.h](../../include/ucon/future/signal_processing/resampling/cic_decimator.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/resampling/polyphase_resampler.c](../../src/ucon/future/signal_processing/resampling/polyphase_resampler.c) - module `ucon/future/signal_processing/resampling/polyphase_resampler`; header [include/ucon/future/signal_processing/resampling/polyphase_resampler.h](../../include/ucon/future/signal_processing/resampling/polyphase_resampler.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/statistics/peak_tracker.c](../../src/ucon/future/signal_processing/statistics/peak_tracker.c) - module `ucon/future/signal_processing/statistics/peak_tracker`; header [include/ucon/future/signal_processing/statistics/peak_tracker.h](../../include/ucon/future/signal_processing/statistics/peak_tracker.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/statistics/rms_window.c](../../src/ucon/future/signal_processing/statistics/rms_window.c) - module `ucon/future/signal_processing/statistics/rms_window`; header [include/ucon/future/signal_processing/statistics/rms_window.h](../../include/ucon/future/signal_processing/statistics/rms_window.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/transforms/fft_radix2.c](../../src/ucon/future/signal_processing/transforms/fft_radix2.c) - module `ucon/future/signal_processing/transforms/fft_radix2`; header [include/ucon/future/signal_processing/transforms/fft_radix2.h](../../include/ucon/future/signal_processing/transforms/fft_radix2.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/signal_processing/transforms/goertzel.c](../../src/ucon/future/signal_processing/transforms/goertzel.c) - module `ucon/future/signal_processing/transforms/goertzel`; header [include/ucon/future/signal_processing/transforms/goertzel.h](../../include/ucon/future/signal_processing/transforms/goertzel.h) - transform/resampling/statistic equations and fixed window/rate policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/future/trajectory/optimization/minimum_snap_traj.c](../../src/ucon/future/trajectory/optimization/minimum_snap_traj.c) - module `ucon/future/trajectory/optimization/minimum_snap_traj`; header [include/ucon/future/trajectory/optimization/minimum_snap_traj.h](../../include/ucon/future/trajectory/optimization/minimum_snap_traj.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/future/trajectory/profiles/time_optimal_profile.c](../../src/ucon/future/trajectory/profiles/time_optimal_profile.c) - module `ucon/future/trajectory/profiles/time_optimal_profile`; header [include/ucon/future/trajectory/profiles/time_optimal_profile.h](../../include/ucon/future/trajectory/profiles/time_optimal_profile.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/anti_windup_backcalc.c](../../src/ucon/infrastructure/blocks/anti_windup_backcalc.c) - module `ucon/infrastructure/blocks/anti_windup_backcalc`; header [include/ucon/infrastructure/blocks/anti_windup_backcalc.h](../../include/ucon/infrastructure/blocks/anti_windup_backcalc.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/anti_windup_clamp.c](../../src/ucon/infrastructure/blocks/anti_windup_clamp.c) - module `ucon/infrastructure/blocks/anti_windup_clamp`; header [include/ucon/infrastructure/blocks/anti_windup_clamp.h](../../include/ucon/infrastructure/blocks/anti_windup_clamp.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/anti_windup_cond.c](../../src/ucon/infrastructure/blocks/anti_windup_cond.c) - module `ucon/infrastructure/blocks/anti_windup_cond`; header [include/ucon/infrastructure/blocks/anti_windup_cond.h](../../include/ucon/infrastructure/blocks/anti_windup_cond.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/bumpless_transfer.c](../../src/ucon/infrastructure/blocks/bumpless_transfer.c) - module `ucon/infrastructure/blocks/bumpless_transfer`; header [include/ucon/infrastructure/blocks/bumpless_transfer.h](../../include/ucon/infrastructure/blocks/bumpless_transfer.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/deadband.c](../../src/ucon/infrastructure/blocks/deadband.c) - module `ucon/infrastructure/blocks/deadband`; header [include/ucon/infrastructure/blocks/deadband.h](../../include/ucon/infrastructure/blocks/deadband.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/gain_block.c](../../src/ucon/infrastructure/blocks/gain_block.c) - module `ucon/infrastructure/blocks/gain_block`; header [include/ucon/infrastructure/blocks/gain_block.h](../../include/ucon/infrastructure/blocks/gain_block.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/hysteresis.c](../../src/ucon/infrastructure/blocks/hysteresis.c) - module `ucon/infrastructure/blocks/hysteresis`; header [include/ucon/infrastructure/blocks/hysteresis.h](../../include/ucon/infrastructure/blocks/hysteresis.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/integrator.c](../../src/ucon/infrastructure/blocks/integrator.c) - module `ucon/infrastructure/blocks/integrator`; header [include/ucon/infrastructure/blocks/integrator.h](../../include/ucon/infrastructure/blocks/integrator.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/mode_select.c](../../src/ucon/infrastructure/blocks/mode_select.c) - module `ucon/infrastructure/blocks/mode_select`; header [include/ucon/infrastructure/blocks/mode_select.h](../../include/ucon/infrastructure/blocks/mode_select.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/multirate_bridge.c](../../src/ucon/infrastructure/blocks/multirate_bridge.c) - module `ucon/infrastructure/blocks/multirate_bridge`; header [include/ucon/infrastructure/blocks/multirate_bridge.h](../../include/ucon/infrastructure/blocks/multirate_bridge.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/rate_limiter.c](../../src/ucon/infrastructure/blocks/rate_limiter.c) - module `ucon/infrastructure/blocks/rate_limiter`; header [include/ucon/infrastructure/blocks/rate_limiter.h](../../include/ucon/infrastructure/blocks/rate_limiter.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/saturator.c](../../src/ucon/infrastructure/blocks/saturator.c) - module `ucon/infrastructure/blocks/saturator`; header [include/ucon/infrastructure/blocks/saturator.h](../../include/ucon/infrastructure/blocks/saturator.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/sensor_voter.c](../../src/ucon/infrastructure/blocks/sensor_voter.c) - module `ucon/infrastructure/blocks/sensor_voter`; header [include/ucon/infrastructure/blocks/sensor_voter.h](../../include/ucon/infrastructure/blocks/sensor_voter.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/signal_validity.c](../../src/ucon/infrastructure/blocks/signal_validity.c) - module `ucon/infrastructure/blocks/signal_validity`; header [include/ucon/infrastructure/blocks/signal_validity.h](../../include/ucon/infrastructure/blocks/signal_validity.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/sum_junction.c](../../src/ucon/infrastructure/blocks/sum_junction.c) - module `ucon/infrastructure/blocks/sum_junction`; header [include/ucon/infrastructure/blocks/sum_junction.h](../../include/ucon/infrastructure/blocks/sum_junction.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/blocks/unit_delay.c](../../src/ucon/infrastructure/blocks/unit_delay.c) - module `ucon/infrastructure/blocks/unit_delay`; header [include/ucon/infrastructure/blocks/unit_delay.h](../../include/ucon/infrastructure/blocks/unit_delay.h) - sampled block recurrence, thresholds, delay, or rate limit. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/infrastructure/fault_detection/sensor_fdi.c](../../src/ucon/infrastructure/fault_detection/sensor_fdi.c) - module `ucon/infrastructure/fault_detection/sensor_fdi`; header [include/ucon/infrastructure/fault_detection/sensor_fdi.h](../../include/ucon/infrastructure/fault_detection/sensor_fdi.h) - residual/statistic, threshold, persistence, latch/reset policy. [Formula steps](#future-ucon-families-standard-candidates-and-blockers).
- [src/ucon/medical/profiles/glucose_control_profile.c](../../src/ucon/medical/profiles/glucose_control_profile.c) - module `ucon/medical/profiles/glucose_control_profile`; header [include/ucon/medical/profiles/glucose_control_profile.h](../../include/ucon/medical/profiles/glucose_control_profile.h) - CONTRACT BLOCKER: approved plant, patient model, units and safety law are not specified. [Formula steps](#medical-profile-controllers-contract-blocker).
- [src/ucon/medical/profiles/pkpd_target_control.c](../../src/ucon/medical/profiles/pkpd_target_control.c) - module `ucon/medical/profiles/pkpd_target_control`; header [include/ucon/medical/profiles/pkpd_target_control.h](../../include/ucon/medical/profiles/pkpd_target_control.h) - CONTRACT BLOCKER: approved plant, patient model, units and safety law are not specified. [Formula steps](#medical-profile-controllers-contract-blocker).
- [src/ucon/medical/profiles/ventilator_control_profile.c](../../src/ucon/medical/profiles/ventilator_control_profile.c) - module `ucon/medical/profiles/ventilator_control_profile`; header [include/ucon/medical/profiles/ventilator_control_profile.h](../../include/ucon/medical/profiles/ventilator_control_profile.h) - CONTRACT BLOCKER: approved plant, patient model, units and safety law are not specified. [Formula steps](#medical-profile-controllers-contract-blocker).
- [src/ucon/trajectory/profiles/bezier_traj.c](../../src/ucon/trajectory/profiles/bezier_traj.c) - module `ucon/trajectory/profiles/bezier_traj`; header [include/ucon/trajectory/profiles/bezier_traj.h](../../include/ucon/trajectory/profiles/bezier_traj.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/bspline_traj.c](../../src/ucon/trajectory/profiles/bspline_traj.c) - module `ucon/trajectory/profiles/bspline_traj`; header [include/ucon/trajectory/profiles/bspline_traj.h](../../include/ucon/trajectory/profiles/bspline_traj.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/jerk_profile.c](../../src/ucon/trajectory/profiles/jerk_profile.c) - module `ucon/trajectory/profiles/jerk_profile`; header [include/ucon/trajectory/profiles/jerk_profile.h](../../include/ucon/trajectory/profiles/jerk_profile.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/poly_traj_3.c](../../src/ucon/trajectory/profiles/poly_traj_3.c) - module `ucon/trajectory/profiles/poly_traj_3`; header [include/ucon/trajectory/profiles/poly_traj_3.h](../../include/ucon/trajectory/profiles/poly_traj_3.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/poly_traj_5.c](../../src/ucon/trajectory/profiles/poly_traj_5.c) - module `ucon/trajectory/profiles/poly_traj_5`; header [include/ucon/trajectory/profiles/poly_traj_5.h](../../include/ucon/trajectory/profiles/poly_traj_5.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/ref_model_1.c](../../src/ucon/trajectory/profiles/ref_model_1.c) - module `ucon/trajectory/profiles/ref_model_1`; header [include/ucon/trajectory/profiles/ref_model_1.h](../../include/ucon/trajectory/profiles/ref_model_1.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/ref_model_2.c](../../src/ucon/trajectory/profiles/ref_model_2.c) - module `ucon/trajectory/profiles/ref_model_2`; header [include/ucon/trajectory/profiles/ref_model_2.h](../../include/ucon/trajectory/profiles/ref_model_2.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/s_curve.c](../../src/ucon/trajectory/profiles/s_curve.c) - module `ucon/trajectory/profiles/s_curve`; header [include/ucon/trajectory/profiles/s_curve.h](../../include/ucon/trajectory/profiles/s_curve.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).
- [src/ucon/trajectory/profiles/trapezoid_profile.c](../../src/ucon/trajectory/profiles/trapezoid_profile.c) - module `ucon/trajectory/profiles/trapezoid_profile`; header [include/ucon/trajectory/profiles/trapezoid_profile.h](../../include/ucon/trajectory/profiles/trapezoid_profile.h) - polynomial/spline/profile boundary conditions and kinematic limits. [Formula steps](#trajectories-profiles-and-infrastructure-blocks).

### Other UETD modules

- [src/uapp/fault.c](../../src/uapp/fault.c) — [calculation steps or target contract](#uapp-state-and-lifecycle-transitions).
- [src/uapp/lifecycle.c](../../src/uapp/lifecycle.c) — [calculation steps or target contract](#uapp-state-and-lifecycle-transitions).
- [src/uapp/statemachine.c](../../src/uapp/statemachine.c) — [calculation steps or target contract](#uapp-state-and-lifecycle-transitions).
- [src/uapp/supervisor.c](../../src/uapp/supervisor.c) — [calculation steps or target contract](#uapp-state-and-lifecycle-transitions).
- [src/ucore/assert.c](../../src/ucore/assert.c) — [calculation steps or target contract](#ucore-checksums-bit-operations-and-status).
- [src/ucore/bitops.c](../../src/ucore/bitops.c); public header [include/ucore/bitops.h](../../include/ucore/bitops.h) — [calculation steps or target contract](#ucore-checksums-bit-operations-and-status).
- [src/ucore/checksum.c](../../src/ucore/checksum.c); public header [include/ucore/checksum.h](../../include/ucore/checksum.h) — [calculation steps or target contract](#ucore-checksums-bit-operations-and-status).
- [src/ucore/status.c](../../src/ucore/status.c) — [calculation steps or target contract](#ucore-checksums-bit-operations-and-status).
- [src/uhal/arm_cm/atomic.c](../../src/uhal/arm_cm/atomic.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/cache.c](../../src/uhal/arm_cm/cache.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/clock.c](../../src/uhal/arm_cm/clock.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/core.c](../../src/uhal/arm_cm/core.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/fault.c](../../src/uhal/arm_cm/fault.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/gpio.c](../../src/uhal/arm_cm/gpio.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/irq.c](../../src/uhal/arm_cm/irq.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/memory.c](../../src/uhal/arm_cm/memory.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/arm_cm/timing.c](../../src/uhal/arm_cm/timing.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/target_profiles.c](../../src/uhal/target_profiles.c); public header [include/uhal/target_profiles.h](../../include/uhal/target_profiles.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/nrf52840/profile.c](../../src/uhal/targets/nrf52840/profile.c); public header [include/uhal/targets/nrf52840/profile.h](../../include/uhal/targets/nrf52840/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/stm32f4/profile.c](../../src/uhal/targets/stm32f4/profile.c); public header [include/uhal/targets/stm32f4/profile.h](../../include/uhal/targets/stm32f4/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/stm32f7/profile.c](../../src/uhal/targets/stm32f7/profile.c); public header [include/uhal/targets/stm32f7/profile.h](../../include/uhal/targets/stm32f7/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/stm32g0/profile.c](../../src/uhal/targets/stm32g0/profile.c); public header [include/uhal/targets/stm32g0/profile.h](../../include/uhal/targets/stm32g0/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/stm32g4/profile.c](../../src/uhal/targets/stm32g4/profile.c); public header [include/uhal/targets/stm32g4/profile.h](../../include/uhal/targets/stm32g4/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/stm32h7/profile.c](../../src/uhal/targets/stm32h7/profile.c); public header [include/uhal/targets/stm32h7/profile.h](../../include/uhal/targets/stm32h7/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/targets/stm32l4plus/profile.c](../../src/uhal/targets/stm32l4plus/profile.c); public header [include/uhal/targets/stm32l4plus/profile.h](../../include/uhal/targets/stm32l4plus/profile.h) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/atomic.c](../../src/uhal/x86/atomic.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/cache.c](../../src/uhal/x86/cache.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/clock.c](../../src/uhal/x86/clock.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/core.c](../../src/uhal/x86/core.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/fault.c](../../src/uhal/x86/fault.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/gpio.c](../../src/uhal/x86/gpio.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/irq.c](../../src/uhal/x86/irq.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/memory.c](../../src/uhal/x86/memory.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uhal/x86/timing.c](../../src/uhal/x86/timing.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/umid/drivers/current.c](../../src/umid/drivers/current.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/encoder.c](../../src/umid/drivers/encoder.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/fatfs.c](../../src/umid/fatfs.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/gnss.c](../../src/umid/drivers/gnss.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/health.c](../../src/umid/health.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/imu.c](../../src/umid/drivers/imu.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/log.c](../../src/umid/log.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/mag.c](../../src/umid/drivers/mag.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/pressure.c](../../src/umid/drivers/pressure.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/range.c](../../src/umid/drivers/range.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/ringbuf.c](../../src/umid/ringbuf.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/temp.c](../../src/umid/drivers/temp.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/umid/drivers/voltage.c](../../src/umid/drivers/voltage.c) — [calculation steps or target contract](#umid-sensor-conversions-encoder-ring-buffer-and-health).
- [src/uos/baremetal/baremetal.c](../../src/uos/baremetal/baremetal.c) — [calculation steps or target contract](#uos-time-deadlines-and-periodic-scheduling).
- [src/uos/freertos/freertos.c](../../src/uos/freertos/freertos.c) — [calculation steps or target contract](#uos-time-deadlines-and-periodic-scheduling).
- [src/uos/future/safertos/safertos.c](../../src/uos/future/safertos/safertos.c) — [calculation steps or target contract](#uos-time-deadlines-and-periodic-scheduling).
- [src/upal/adc/adc.c](../../src/upal/adc/adc.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/can/can.c](../../src/upal/can/can.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/comp/comp.c](../../src/upal/comp/comp.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/cordic/cordic.c](../../src/upal/cordic/cordic.c) — [calculation steps or target contract](#upal-cordic-and-hardware-conversion-candidates).
- [src/upal/dac/dac.c](../../src/upal/dac/dac.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/dma/dma.c](../../src/upal/dma/dma.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/fdcan_fd/fdcan_fd.c](../../src/upal/fdcan_fd/fdcan_fd.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/flash/flash.c](../../src/upal/flash/flash.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/fmac/fmac.c](../../src/upal/fmac/fmac.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/hrtim/hrtim.c](../../src/upal/hrtim/hrtim.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/i2c/i2c.c](../../src/upal/i2c/i2c.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/iwdg/iwdg.c](../../src/upal/iwdg/iwdg.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/opamp/opamp.c](../../src/upal/opamp/opamp.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/qspi/qspi.c](../../src/upal/qspi/qspi.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/rtc/rtc.c](../../src/upal/rtc/rtc.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/sdmmc/sdmmc.c](../../src/upal/sdmmc/sdmmc.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/spi/spi.c](../../src/upal/spi/spi.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/timer/timer.c](../../src/upal/timer/timer.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/uart/uart.c](../../src/upal/uart/uart.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/usb/usb.c](../../src/upal/usb/usb.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/upal/wwdg/wwdg.c](../../src/upal/wwdg/wwdg.c) — [calculation steps or target contract](#hal-platform-peripheral-files-without-portable-equations).
- [src/uproto/crsf.c](../../src/uproto/crsf.c) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/dshot.c](../../src/uproto/dshot.c) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/future/elrs/elrs.c](../../src/uproto/future/elrs/elrs.c); public header [include/uproto/future/elrs/elrs.h](../../include/uproto/future/elrs/elrs.h) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/future/iec60870/iec60870.c](../../src/uproto/future/iec60870/iec60870.c); public header [include/uproto/future/iec60870/iec60870.h](../../include/uproto/future/iec60870/iec60870.h) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/future/j1939/j1939.c](../../src/uproto/future/j1939/j1939.c); public header [include/uproto/future/j1939/j1939.h](../../include/uproto/future/j1939/j1939.h) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/future/lin/lin.c](../../src/uproto/future/lin/lin.c); public header [include/uproto/future/lin/lin.h](../../include/uproto/future/lin/lin.h) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/future/msp/msp.c](../../src/uproto/future/msp/msp.c); public header [include/uproto/future/msp/msp.h](../../include/uproto/future/msp/msp.h) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/future/uavcan_v1/uavcan_v1.c](../../src/uproto/future/uavcan_v1/uavcan_v1.c); public header [include/uproto/future/uavcan_v1/uavcan_v1.h](../../include/uproto/future/uavcan_v1/uavcan_v1.h) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/mavlink.c](../../src/uproto/mavlink.c) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/ppm.c](../../src/uproto/ppm.c) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/sbus.c](../../src/uproto/sbus.c) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).
- [src/uproto/uavcan.c](../../src/uproto/uavcan.c) — [calculation steps or target contract](#uproto-frame-checksum-and-bit-packing-calculations).

