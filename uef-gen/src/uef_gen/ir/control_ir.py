# ControlIR is serialized by NEXUS and passed to uef-gen as versioned JSON.
# This module validates the inter-process payload only; it does not define
# algorithm models or translate graph semantics into generator-owned classes.

from __future__ import annotations
import json
from pathlib import Path
from typing import Any
from uef_gen.diagnostics import Diagnostic, error

_SCHEMA_PATH = Path(__file__).resolve().parents[1] / "schemas" / "control_ir.schema.json"

def validate_control_ir(value: Any) -> list[Diagnostic]:
    if not isinstance(value, dict):
        return [error("control_ir_root", "ControlIR root must be an object")]
    try:
        import jsonschema
        schema = json.loads(_SCHEMA_PATH.read_text(encoding="utf-8"))
        validator = jsonschema.validators.validator_for(schema)(schema)
        issues = sorted(validator.iter_errors(value), key=lambda issue: list(map(str, issue.absolute_path)))
        return [error("control_ir_schema", issue.message, ".".join(map(str, issue.absolute_path))) for issue in issues]
    except ImportError:
        required = ("schema_version", "subsystem", "target", "execution_groups", "nodes", "edges", "interface", "composite_nodes")
        missing = [key for key in required if key not in value]
        return [error("control_ir_field_missing", f"Missing ControlIR field: {key}") for key in missing]
