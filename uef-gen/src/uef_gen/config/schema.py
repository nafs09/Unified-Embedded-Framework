from __future__ import annotations

from pathlib import Path
from typing import Any

from uef_gen.diagnostics import Diagnostic, error

_SCHEMA_PATH = Path(__file__).resolve().parents[1] / "schemas" / "project.schema.json"


def validate_configuration(config: dict[str, Any]) -> list[Diagnostic]:
    """Apply schema validation plus safety checks that remain available offline."""
    diagnostics = _minimal_validation(config)
    try:
        import jsonschema

        document = _schema()
        validator_type = jsonschema.validators.validator_for(document)
        validator = validator_type(document)
        issues = sorted(
            (
                issue
                for issue in validator.iter_errors(config)
                # The offline validator emits a more actionable diagnostic
                # for unsupported `uef.*` keys, so avoid a duplicate generic
                # additionalProperties message for the same object.
                if not (
                    issue.validator == "additionalProperties"
                    and tuple(issue.absolute_path) == ("uef",)
                )
            ),
            key=lambda issue: list(map(str, issue.absolute_path)),
        )
        diagnostics.extend(
            error(
                "hardware_config_schema",
                issue.message,
                ".".join(map(str, issue.absolute_path)),
            )
            for issue in issues
        )
    except ImportError:
        # Minimal checks keep malformed roots/types from crashing resource resolution.
        # Full nested validation remains required in the supported installation.
        pass
    return diagnostics


def _schema() -> dict[str, Any]:
    """Read the package-local schema so installed wheels need no source checkout."""
    import json

    return json.loads(_SCHEMA_PATH.read_text(encoding="utf-8"))


def _minimal_validation(config: dict[str, Any]) -> list[Diagnostic]:
    """Check safety-critical shapes even when the optional schema engine is absent."""
    target = config.get("target")
    if not isinstance(target, dict) or not isinstance(target.get("chip"), str) or not target["chip"]:
        return [error("hardware_config_target", "target.chip is required")]
    diagnostics: list[Diagnostic] = []
    if "toolchain" in target and (
        not isinstance(target["toolchain"], str) or not target["toolchain"].strip()
    ):
        diagnostics.append(
            error("hardware_config_toolchain", "target.toolchain must be a non-empty profile name", "target.toolchain")
        )
    freertos = target.get("freertos")
    if freertos is not None:
        if not isinstance(freertos, dict):
            diagnostics.append(error("hardware_config_type", "target.freertos must be an object", "target.freertos"))
        elif "tick_hz" in freertos and (
            not isinstance(freertos["tick_hz"], (int, float))
            or isinstance(freertos["tick_hz"], bool)
            or freertos["tick_hz"] <= 0
        ):
            diagnostics.append(error("hardware_config_tick_invalid", "target.freertos.tick_hz must be positive", "target.freertos.tick_hz"))
    for key in ("modules", "required_capabilities", "compiler_flags"):
        if key in config and not isinstance(config[key], list):
            diagnostics.append(error("hardware_config_type", f"{key} must be an array", key))
    for key in ("protocols", "tasks"):
        if key in config and not isinstance(config[key], (list, dict)):
            diagnostics.append(error("hardware_config_type", f"{key} must be an array or mapping", key))
    if "peripherals" in config and not isinstance(config["peripherals"], (list, dict)):
        diagnostics.append(error("hardware_config_type", "peripherals must be an array or mapping", "peripherals"))
    if "ucon" in config:
        diagnostics.append(error(
            "hardware_config_ucon_removed",
            "UCON details are supplied through NEXUS ControlIR and UEF-owned templates, not hardware configuration",
            "ucon",
        ))
    if "uef" in config:
        uef = config["uef"]
        if not isinstance(uef, dict):
            diagnostics.append(error("hardware_config_type", "uef must be an object", "uef"))
        else:
            unsupported = sorted(set(uef) - {"path", "extra_modules"})
            for key in unsupported:
                diagnostics.append(error(
                    "hardware_config_uef_option_unsupported",
                    f"uef.{key} is not a supported project option; locate and version UEF with UEF_PATH/uef.path and its version.json",
                    f"uef.{key}",
                ))
            if "path" in uef and (not isinstance(uef["path"], str) or not uef["path"].strip()):
                diagnostics.append(error("hardware_config_uef_path", "uef.path must be a non-empty path string", "uef.path"))
            extra_modules = uef.get("extra_modules", [])
            if not isinstance(extra_modules, list) or any(
                not isinstance(module, str) or not module.strip()
                for module in extra_modules
            ):
                diagnostics.append(error(
                    "hardware_config_uef_modules",
                    "uef.extra_modules must be an array of non-empty UEF module names",
                    "uef.extra_modules",
                ))
    rtos = config.get("rtos")
    if rtos is not None and not isinstance(rtos, (str, dict)):
        diagnostics.append(error("hardware_config_type", "rtos must be a name or settings object", "rtos"))
    if isinstance(rtos, dict) and "tick_us" in rtos and (
        not isinstance(rtos["tick_us"], int) or rtos["tick_us"] <= 0
    ):
        diagnostics.append(error("hardware_config_tick_invalid", "rtos.tick_us must be a positive integer", "rtos.tick_us"))
    defines = config.get("defines", [])
    if not isinstance(defines, list):
        diagnostics.append(error("hardware_config_defines", "defines must be an array of compiler definitions", "defines"))
    else:
        import re
        for index, define in enumerate(defines):
            scalar = r"(?:[A-Za-z_][A-Za-z0-9_]*|-?(?:0[xX][0-9A-Fa-f]+|(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?))"
            definition_pattern = r"[A-Za-z_][A-Za-z0-9_]*(?:=" + scalar + r")?"
            if not isinstance(define, str) or not re.fullmatch(definition_pattern, define):
                diagnostics.append(
                    error(
                        "hardware_config_define_unsafe",
                        "Compiler definitions must be NAME or NAME with one identifier or numeric literal value",
                        f"defines[{index}]",
                    )
                )
    flags = config.get("compiler_flags", [])
    if isinstance(flags, list):
        import re
        for index, flag in enumerate(flags):
            if not isinstance(flag, str) or not re.fullmatch(r"[-/A-Za-z0-9_.,:=+]+", flag):
                diagnostics.append(
                    error(
                        "hardware_config_flag_unsafe",
                        "Compiler flags must be single safe command-line tokens",
                        f"compiler_flags[{index}]",
                    )
                )
    return diagnostics
