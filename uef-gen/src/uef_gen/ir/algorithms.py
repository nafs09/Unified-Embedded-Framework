"""Transitional typed representations of the earlier V8 AlgorithmIR contract.

These data classes describe algorithms; they deliberately do not implement an
algorithm or provide a default template. Project-owned builders and templates
remain responsible for those domain-specific choices. The current ownership
direction moves UCON algorithms and their reusable definitions to UEF. Do not
extend this generator-side list until the replacement UEF/NEXUS contract is set.
"""

from __future__ import annotations

from dataclasses import dataclass, fields, is_dataclass
import re
from typing import Any, Literal, TypeAlias

from uef_gen.diagnostics import Diagnostic, error


@dataclass(frozen=True)
class MatrixExpr:
    """Matrix of emitted C expressions; zero entries are marked explicitly."""

    rows: int
    cols: int
    exprs: list[list[str]]
    is_zero: list[list[bool]]

    @classmethod
    def from_symengine(cls, sym_matrix: Any, var_map: Any, emitter: Any) -> "MatrixExpr":
        """Lower a SymEngine-like matrix through the caller's C expression emitter."""

        rows, cols = sym_matrix.shape
        expressions: list[list[str]] = []
        zero_mask: list[list[bool]] = []
        for row in range(rows):
            expression_row: list[str] = []
            zero_row: list[bool] = []
            for column in range(cols):
                expression = sym_matrix[row, column]
                if expression == 0:
                    expression_row.append("0")
                    zero_row.append(True)
                else:
                    expression_row.append(emitter.emit(expression, var_map))
                    zero_row.append(False)
            expressions.append(expression_row)
            zero_mask.append(zero_row)
        return cls(rows, cols, expressions, zero_mask)

    def __post_init__(self) -> None:
        if self.rows < 0 or self.cols < 0:
            raise ValueError("MatrixExpr dimensions cannot be negative")
        if len(self.exprs) != self.rows or len(self.is_zero) != self.rows:
            raise ValueError("MatrixExpr row count does not match its declared shape")
        for expressions, zeroes in zip(self.exprs, self.is_zero, strict=True):
            if len(expressions) != self.cols or len(zeroes) != self.cols:
                raise ValueError("MatrixExpr column count does not match its declared shape")
            if any(not isinstance(value, str) for value in expressions):
                raise TypeError("MatrixExpr entries must be C expression strings")
            if any(not isinstance(value, bool) for value in zeroes):
                raise TypeError("MatrixExpr is_zero entries must be booleans")


@dataclass(frozen=True)
class VectorExpr:
    length: int
    exprs: list[str]
    is_zero: list[bool]

    def __post_init__(self) -> None:
        if self.length < 0 or len(self.exprs) != self.length or len(self.is_zero) != self.length:
            raise ValueError("VectorExpr entries do not match its declared length")
        if any(not isinstance(value, str) for value in self.exprs):
            raise TypeError("VectorExpr entries must be C expression strings")
        if any(not isinstance(value, bool) for value in self.is_zero):
            raise TypeError("VectorExpr is_zero entries must be booleans")


@dataclass(frozen=True)
class PIDAlgorithmIR:
    instance_name: str
    num_type: str
    sample_time_expr: str
    has_integral: bool
    has_derivative: bool
    has_feedforward: bool
    has_output_sat: bool
    has_deadband: bool
    has_setpoint_weighting: bool
    has_rate_limit: bool
    anti_windup: Literal["none", "clamp", "back_calculation", "conditional"]
    kd_b0_expr: str | None
    kd_b1_expr: str | None
    ki_half_ts_expr: str | None
    back_calc_bt_expr: str | None
    config_params: list[str]


@dataclass(frozen=True)
class StateFeedbackAlgorithmIR:
    instance_name: str
    num_type: str
    n_states: int
    n_inputs: int
    K: MatrixExpr
    has_reference_tracking: bool
    has_integral_action: bool
    has_output_sat: bool
    precompute_offline: bool
    A_discrete: MatrixExpr | None
    B_discrete: MatrixExpr | None
    config_params: list[str]


@dataclass(frozen=True)
class KalmanFilterAlgorithmIR:
    instance_name: str
    num_type: str
    n_states: int
    n_meas: int
    n_inputs: int
    A: MatrixExpr
    B: MatrixExpr | None
    H: MatrixExpr
    P0_exprs: list[list[str]]
    update_method: Literal["standard", "joseph", "sqrt_cholesky"]
    has_inputs: bool
    has_innovation_gate: bool
    config_params: list[str]


@dataclass(frozen=True)
class EKFAlgorithmIR:
    instance_name: str
    num_type: str
    n_states: int
    n_meas: int
    n_inputs: int
    f_exprs: VectorExpr
    h_exprs: VectorExpr
    F_jacobian: MatrixExpr
    H_jacobian: MatrixExpr
    Fu_jacobian: MatrixExpr | None
    F_nnz: int
    H_nnz: int
    update_method: Literal["standard", "joseph", "sqrt_cholesky"]
    has_inputs: bool
    has_innovation_gate: bool
    use_sparse_F: bool
    use_sparse_H: bool
    P0_exprs: list[list[str]]
    x0_exprs: list[str]
    config_params: list[str]


@dataclass(frozen=True)
class UKFAlgorithmIR:
    instance_name: str
    num_type: str
    n_states: int
    n_meas: int
    n_inputs: int
    f_exprs: VectorExpr
    h_exprs: VectorExpr
    sigma_scheme: Literal["julier_uhlmann", "merwe_scaled"]
    alpha_expr: str
    beta_expr: str
    kappa_expr: str
    n_sigma: int
    Wm_exprs: list[str]
    Wc_exprs: list[str]
    has_inputs: bool
    P0_exprs: list[list[str]]
    x0_exprs: list[str]
    config_params: list[str]


@dataclass(frozen=True)
class MahonyFilterAlgorithmIR:
    instance_name: str
    num_type: str
    dt_expr: str
    has_magnetometer: bool
    has_bias_estimation: bool
    gravity_ref_expr: str
    mag_ref_expr: str | None
    config_params: list[str]


@dataclass(frozen=True)
class LuenbergerObserverIR:
    instance_name: str
    num_type: str
    n_states: int
    n_meas: int
    n_inputs: int
    A: MatrixExpr
    B: MatrixExpr | None
    C: MatrixExpr
    L: MatrixExpr
    ALC: MatrixExpr
    config_params: list[str]


@dataclass(frozen=True)
class LinearMPCAlgorithmIR:
    instance_name: str
    num_type: str
    n_states: int
    n_inputs: int
    n_outputs: int
    horizon_p: int
    horizon_c: int
    Phi: MatrixExpr
    Gamma: MatrixExpr
    Psi: MatrixExpr
    Theta: MatrixExpr
    H_qp: MatrixExpr
    qp_solver: Literal["gradient_projection", "active_set", "osqp"]
    has_terminal_cost: bool
    has_output_constraints: bool
    has_delta_u_constraints: bool
    warm_start: bool
    n_ineq: int
    config_params: list[str]


AlgorithmIR: TypeAlias = (
    PIDAlgorithmIR
    | StateFeedbackAlgorithmIR
    | KalmanFilterAlgorithmIR
    | EKFAlgorithmIR
    | UKFAlgorithmIR
    | MahonyFilterAlgorithmIR
    | LuenbergerObserverIR
    | LinearMPCAlgorithmIR
)

_ALGORITHM_NAMES: dict[type[Any], str] = {
    PIDAlgorithmIR: "PID",
    StateFeedbackAlgorithmIR: "STATE_FEEDBACK",
    KalmanFilterAlgorithmIR: "KALMAN_FILTER",
    EKFAlgorithmIR: "EKF",
    UKFAlgorithmIR: "UKF",
    MahonyFilterAlgorithmIR: "MAHONY_FILTER",
    LuenbergerObserverIR: "LUENBERGER_OBSERVER",
    LinearMPCAlgorithmIR: "LINEAR_MPC",
}
_C_IDENTIFIER = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


def algorithm_type_for(value: Any) -> str:
    """Return the stable template-dispatch key for a typed AlgorithmIR."""

    for algorithm_class, name in _ALGORITHM_NAMES.items():
        if isinstance(value, algorithm_class):
            return name
    if isinstance(value, dict) and isinstance(value.get("algorithm_type"), str):
        return value["algorithm_type"].upper()
    raise TypeError(f"unsupported AlgorithmIR value: {type(value).__name__}")


def _matrix(value: Any, rows: int, cols: int, location: str, diagnostics: list[Diagnostic]) -> None:
    if not isinstance(value, MatrixExpr):
        diagnostics.append(error("algorithm_ir_matrix_type", "Expected MatrixExpr", location))
    elif (value.rows, value.cols) != (rows, cols):
        diagnostics.append(error("algorithm_ir_matrix_shape", f"Expected {rows}x{cols}, received {value.rows}x{value.cols}", location))


def _vector(value: Any, length: int, location: str, diagnostics: list[Diagnostic]) -> None:
    if not isinstance(value, VectorExpr):
        diagnostics.append(error("algorithm_ir_vector_type", "Expected VectorExpr", location))
    elif value.length != length:
        diagnostics.append(error("algorithm_ir_vector_shape", f"Expected length {length}, received {value.length}", location))


def _square_expressions(value: Any, size: int, location: str, diagnostics: list[Diagnostic]) -> None:
    if not isinstance(value, list) or len(value) != size or any(
        not isinstance(row, list) or len(row) != size or any(not isinstance(item, str) for item in row)
        for row in value
    ):
        diagnostics.append(error("algorithm_ir_expression_matrix_shape", f"Expected a {size}x{size} expression matrix", location))


def _expression_vector(value: Any, length: int, location: str, diagnostics: list[Diagnostic]) -> None:
    if not isinstance(value, list) or len(value) != length or any(not isinstance(item, str) for item in value):
        diagnostics.append(error("algorithm_ir_vector_shape", f"Expected {length} C expression strings", location))


def validate_algorithm_ir(value: Any, location: str = "") -> list[Diagnostic]:
    """Check typed IR dimensions and required identifiers before templates run."""

    if not is_dataclass(value) or isinstance(value, type) or type(value) not in _ALGORITHM_NAMES:
        return [error("algorithm_ir_type", "NodeIRBuilder must return one of the declared typed AlgorithmIR values", location)]
    diagnostics: list[Diagnostic] = []
    name = getattr(value, "instance_name", "")
    if not isinstance(name, str) or not _C_IDENTIFIER.fullmatch(name):
        diagnostics.append(error("algorithm_ir_instance_name", "instance_name must be a valid C identifier", location))
    if not isinstance(value.num_type, str) or not value.num_type:
        diagnostics.append(error("algorithm_ir_num_type", "num_type must be a non-empty C type", location))
    params = getattr(value, "config_params", [])
    if not isinstance(params, list) or any(
        not isinstance(item, str) or not _C_IDENTIFIER.fullmatch(item)
        for item in params
    ):
        diagnostics.append(error("algorithm_ir_config_params", "config_params must contain C identifiers", location))
    elif len(params) != len(set(params)):
        diagnostics.append(error("algorithm_ir_config_params_duplicate", "config_params must not contain duplicates", location))

    positive_fields = ("n_states", "n_meas", "n_inputs", "n_outputs", "horizon_p", "horizon_c")
    for field_name in positive_fields:
        if hasattr(value, field_name):
            item = getattr(value, field_name)
            if not isinstance(item, int) or isinstance(item, bool) or item <= 0:
                diagnostics.append(error("algorithm_ir_dimension", f"{field_name} must be a positive integer", f"{location}.{field_name}"))

    if isinstance(value, PIDAlgorithmIR):
        if value.anti_windup not in {"none", "clamp", "back_calculation", "conditional"}:
            diagnostics.append(error("algorithm_ir_enum", "Unknown PID anti_windup mode", f"{location}.anti_windup"))
    elif isinstance(value, StateFeedbackAlgorithmIR):
        _matrix(value.K, value.n_inputs, value.n_states, f"{location}.K", diagnostics)
        if value.A_discrete is not None:
            _matrix(value.A_discrete, value.n_states, value.n_states, f"{location}.A_discrete", diagnostics)
        if value.B_discrete is not None:
            _matrix(value.B_discrete, value.n_states, value.n_inputs, f"{location}.B_discrete", diagnostics)
    elif isinstance(value, KalmanFilterAlgorithmIR):
        _matrix(value.A, value.n_states, value.n_states, f"{location}.A", diagnostics)
        _matrix(value.H, value.n_meas, value.n_states, f"{location}.H", diagnostics)
        if value.B is not None:
            _matrix(value.B, value.n_states, value.n_inputs, f"{location}.B", diagnostics)
        _square_expressions(value.P0_exprs, value.n_states, f"{location}.P0_exprs", diagnostics)
        if value.update_method not in {"standard", "joseph", "sqrt_cholesky"}:
            diagnostics.append(error("algorithm_ir_enum", "Unknown Kalman update_method", f"{location}.update_method"))
    elif isinstance(value, EKFAlgorithmIR):
        _vector(value.f_exprs, value.n_states, f"{location}.f_exprs", diagnostics)
        _vector(value.h_exprs, value.n_meas, f"{location}.h_exprs", diagnostics)
        _matrix(value.F_jacobian, value.n_states, value.n_states, f"{location}.F_jacobian", diagnostics)
        _matrix(value.H_jacobian, value.n_meas, value.n_states, f"{location}.H_jacobian", diagnostics)
        if value.Fu_jacobian is not None:
            _matrix(value.Fu_jacobian, value.n_states, value.n_inputs, f"{location}.Fu_jacobian", diagnostics)
        _square_expressions(value.P0_exprs, value.n_states, f"{location}.P0_exprs", diagnostics)
        _expression_vector(value.x0_exprs, value.n_states, f"{location}.x0_exprs", diagnostics)
        if value.update_method not in {"standard", "joseph", "sqrt_cholesky"}:
            diagnostics.append(error("algorithm_ir_enum", "Unknown EKF update_method", f"{location}.update_method"))
        for matrix_name, matrix_value, nnz in (("F_jacobian", value.F_jacobian, value.F_nnz), ("H_jacobian", value.H_jacobian, value.H_nnz)):
            if isinstance(nnz, bool) or not isinstance(nnz, int) or nnz < 0:
                diagnostics.append(error("algorithm_ir_nnz", f"{matrix_name} nnz must be a non-negative integer", f"{location}.{matrix_name}"))
            elif isinstance(matrix_value, MatrixExpr) and nnz != sum(not zero for row in matrix_value.is_zero for zero in row):
                diagnostics.append(error("algorithm_ir_nnz_mismatch", f"{matrix_name} nnz does not match its zero mask", f"{location}.{matrix_name}"))
    elif isinstance(value, UKFAlgorithmIR):
        _vector(value.f_exprs, value.n_states, f"{location}.f_exprs", diagnostics)
        _vector(value.h_exprs, value.n_meas, f"{location}.h_exprs", diagnostics)
        _square_expressions(value.P0_exprs, value.n_states, f"{location}.P0_exprs", diagnostics)
        _expression_vector(value.x0_exprs, value.n_states, f"{location}.x0_exprs", diagnostics)
        _expression_vector(value.Wm_exprs, value.n_sigma, f"{location}.Wm_exprs", diagnostics)
        _expression_vector(value.Wc_exprs, value.n_sigma, f"{location}.Wc_exprs", diagnostics)
        if value.n_sigma != 2 * value.n_states + 1:
            diagnostics.append(error("algorithm_ir_sigma_count", "n_sigma must equal 2*n_states + 1", f"{location}.n_sigma"))
        if (not isinstance(value.n_sigma, int) or isinstance(value.n_sigma, bool)
                or value.n_sigma != 2 * value.n_states + 1):
            diagnostics.append(error("algorithm_ir_sigma_count", "n_sigma must equal 2*n_states + 1", f"{location}.n_sigma"))
        if value.sigma_scheme not in {"julier_uhlmann", "merwe_scaled"}:
            diagnostics.append(error("algorithm_ir_enum", "Unknown UKF sigma_scheme", f"{location}.sigma_scheme"))
    elif isinstance(value, LuenbergerObserverIR):
        _matrix(value.A, value.n_states, value.n_states, f"{location}.A", diagnostics)
        if value.B is not None:
            _matrix(value.B, value.n_states, value.n_inputs, f"{location}.B", diagnostics)
        _matrix(value.C, value.n_meas, value.n_states, f"{location}.C", diagnostics)
        _matrix(value.L, value.n_states, value.n_meas, f"{location}.L", diagnostics)
        _matrix(value.ALC, value.n_states, value.n_states, f"{location}.ALC", diagnostics)
    elif isinstance(value, LinearMPCAlgorithmIR):
        _matrix(value.Phi, value.n_states * value.horizon_p, value.n_states, f"{location}.Phi", diagnostics)
        _matrix(value.Gamma, value.n_states * value.horizon_p, value.n_inputs * value.horizon_c, f"{location}.Gamma", diagnostics)
        _matrix(value.Psi, value.n_outputs * value.horizon_p, value.n_states, f"{location}.Psi", diagnostics)
        _matrix(value.Theta, value.n_outputs * value.horizon_p, value.n_inputs * value.horizon_c, f"{location}.Theta", diagnostics)
        _matrix(value.H_qp, value.n_inputs * value.horizon_c, value.n_inputs * value.horizon_c, f"{location}.H_qp", diagnostics)
        if value.qp_solver not in {"gradient_projection", "active_set", "osqp"}:
            diagnostics.append(error("algorithm_ir_enum", "Unknown Linear MPC qp_solver", f"{location}.qp_solver"))
        if not isinstance(value.n_ineq, int) or isinstance(value.n_ineq, bool) or value.n_ineq < 0:
            diagnostics.append(error("algorithm_ir_n_ineq", "n_ineq must be a non-negative integer", f"{location}.n_ineq"))

    # Keep this import/use visible to static checkers when fields evolve.
    for item in fields(value):
        if item.name.endswith("_expr") and getattr(value, item.name) is not None and not isinstance(getattr(value, item.name), str):
            diagnostics.append(error("algorithm_ir_expression", f"{item.name} must be a string expression", f"{location}.{item.name}"))
    return diagnostics
