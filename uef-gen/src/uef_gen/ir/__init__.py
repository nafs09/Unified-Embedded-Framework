"""Typed AlgorithmIR contracts and self-registering node IR builders."""

from uef_gen.ir.algorithms import (
    AlgorithmIR,
    EKFAlgorithmIR,
    KalmanFilterAlgorithmIR,
    LinearMPCAlgorithmIR,
    LuenbergerObserverIR,
    MahonyFilterAlgorithmIR,
    MatrixExpr,
    PIDAlgorithmIR,
    StateFeedbackAlgorithmIR,
    UKFAlgorithmIR,
    VectorExpr,
    algorithm_type_for,
    validate_algorithm_ir,
)
from uef_gen.ir.registry import NodeIRBuilder, UnknownNodeTypeError, build_node_ir, registered_builders

__all__ = [
    "AlgorithmIR",
    "EKFAlgorithmIR",
    "KalmanFilterAlgorithmIR",
    "LinearMPCAlgorithmIR",
    "LuenbergerObserverIR",
    "MahonyFilterAlgorithmIR",
    "MatrixExpr",
    "NodeIRBuilder",
    "PIDAlgorithmIR",
    "StateFeedbackAlgorithmIR",
    "UKFAlgorithmIR",
    "UnknownNodeTypeError",
    "VectorExpr",
    "algorithm_type_for",
    "build_node_ir",
    "registered_builders",
    "validate_algorithm_ir",
]
"""AlgorithmIR builder extension points and ControlIR validation."""
