from __future__ import annotations

from pathlib import Path
from typing import Any

from uef_gen.diagnostics import Diagnostic, error
from uef_gen.generation.pipeline import generate_project

CONTRACT_VERSION = "1.0"
CONTROL_IR_SCHEMA_VERSION = "1.0"


def handle_request(request: dict[str, Any]) -> dict[str, Any]:
    """Validate one versioned JSON request and return one JSON-safe response.

    Keep this transport boundary independent of NEXUS so CI and other host tools
    can use the same stable contract. New UCON ownership fields must be versioned
    with the UEF/NEXUS migration rather than silently changing this payload.
    """
    if not isinstance(request, dict):
        return _response(False, [error("request_invalid", "Request root must be an object")])
    if request.get("contract_version") != CONTRACT_VERSION:
        return _response(
            False,
            [error("unsupported_contract_version", "Expected contract_version 1.0")],
        )
    if request.get("operation") != "generate":
        return _response(
            False,
            [error("unsupported_operation", "Expected operation 'generate'")],
        )
    required = ("hardware_configuration", "output_directory", "work_directory")
    missing = [key for key in required if key not in request]
    if missing:
        return _response(
            False,
            [
                error("missing_request_field", f"Required request field is missing: {key}")
                for key in missing
            ],
        )
    config = request["hardware_configuration"]
    if not isinstance(config, dict):
        return _response(
            False,
            [
                error(
                    "hardware_configuration_invalid",
                    "hardware_configuration must be a JSON object",
                )
            ],
        )
    control_ir = request.get("control_ir")
    if control_ir is not None and not isinstance(control_ir, dict):
        return _response(
            False,
            [error("control_ir_invalid", "control_ir must be a JSON object when supplied")],
        )
    if control_ir is not None and control_ir.get("schema_version") != CONTROL_IR_SCHEMA_VERSION:
        return _response(
            False,
            [
                error(
                    "unsupported_control_ir_version",
                    "Expected ControlIR schema_version 1.0",
                )
            ],
        )
    output = request["output_directory"]
    work = request["work_directory"]
    if not isinstance(output, str) or not output.strip() or not isinstance(work, str) or not work.strip():
        return _response(
            False,
            [
                error(
                    "request_path_invalid",
                    "output_directory and work_directory must be non-empty strings",
                )
            ],
        )
    result = generate_project(config, control_ir, Path(output), Path(work))
    return _response(
        result.ok,
        result.diagnostics,
        result.manifest if result.ok else None,
        str(result.output_directory),
    )


def _response(ok: bool, diagnostics: list[Diagnostic], manifest: dict | None = None,
              output_directory: str | None = None) -> dict[str, Any]:
    """Normalize response fields so stdout has one predictable shape."""
    response: dict[str, Any] = {
        "contract_version": CONTRACT_VERSION,
        "ok": bool(ok),
        "diagnostics": [item.to_dict() for item in diagnostics],
    }
    if manifest is not None:
        response["manifest"] = manifest
    if output_directory is not None:
        response["output_directory"] = output_directory
    return response
