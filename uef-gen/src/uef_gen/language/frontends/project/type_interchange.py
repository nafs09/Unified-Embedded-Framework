"""Shared deterministic type resolution and expression checking for UEF-PL."""

from __future__ import annotations
import math
import re
from dataclasses import dataclass, field, replace
from fractions import Fraction
from typing import Any, Literal
from uef_gen.language.frontends.project.ast import (
    BinaryExpression, CastExpression, FlagsOperationExpression, Identifier, IndexExpression,
    ListValue, Literal as ValueLiteral, MatchExpression, MatrixLiteral, MathCallExpression,
    ProjectDocument, RecordValue, Reference, SelectValue, SourceSpan, TypeSpec, UnaryExpression,
    UnitConversionExpression, Value, VariantConstructorExpression,
    VariantDefinition,
)
from uef_gen.language.frontends.project.diagnostics import LanguageDiagnostic, source_error, source_warning
from uef_gen.language.frontends.project.registry_contracts import (
    AuthoringCatalogue,
    UETDMathFunctionContract,
    math_contract_parameter_types,
    math_contract_result_type,
    resolve_math_overload,
)
from uef_gen.language.registry.loader import LanguageRegistry

TypeOwner = Literal["language", "project", "system_contract", "uetd"]
TypeBoundary = Literal["value", "configuration", "module_abi", "umath_abi", "wire", "shared_memory", "persistent"]

@dataclass(frozen=True)
class TypeIdentity:
    owner: TypeOwner
    canonical_name: str
    source_name: str
    kind: str
    c_name: str | None = None
    owner_id: str | None = None
    source_path: str | None = None
    visibility: str | None = None
    readiness: str | None = None
    language_contract: str | None = None
    enum_members: tuple[str, ...] = ()
    c_kind: str | None = None

@dataclass(frozen=True)
class ResolvedType:
    identity: TypeIdentity
    arguments: tuple[ResolvedType, ...] = ()
    shape: tuple[int, ...] = ()
    unit: str | None = None
    unit_dimensions: tuple[tuple[str, int], ...] = ()
    affine: bool = False
    boundary: TypeBoundary = "value"
    storage_bits: int | None = None
    signed: bool | None = None
    fractional_bits: int | None = None
    maximum_encoded_bytes: int | None = None
    rounding: str | None = None
    overflow: str | None = None
    unit_scale: tuple[int, int, int] = (1, 1, 0)
    unit_offset: tuple[int, int] = (0, 1)

@dataclass(frozen=True)
class TypedExpressionFacts:
    expression: Any
    resolved_type: ResolvedType
    effect: str = "pure"
    worst_case_operations: int | None = None
    maximum_stack_bytes: int | None = None
    may_fail: bool = False

@dataclass
class TypeEnvironment:
    language_types: dict[str, ResolvedType] = field(default_factory=dict)
    language_registry: LanguageRegistry | None = None
    project_types: dict[str, ResolvedType] = field(default_factory=dict)
    project_definitions: dict[str, Any] = field(default_factory=dict)
    imported_types: dict[str, ResolvedType] = field(default_factory=dict)
    imported_definitions: dict[str, Any] = field(default_factory=dict)
    uetd_types_by_dsl_name: dict[str, ResolvedType] = field(default_factory=dict)
    uetd_types_by_c_name: dict[str, tuple[ResolvedType, ...]] = field(default_factory=dict)
    unit_catalogue: dict[str, Any] = field(default_factory=dict)
    math_catalogue: dict[str, tuple[UETDMathFunctionContract, ...]] = field(default_factory=dict)
    math_operator_catalogue: dict[str, tuple[UETDMathFunctionContract, ...]] = field(default_factory=dict)
    math_catalogue_by_c_name: dict[str, Any] = field(default_factory=dict)
    math_call_contexts: dict[int, str] = field(default_factory=dict)
    resolved_math_bindings: dict[int, UETDMathFunctionContract] = field(default_factory=dict)
    resolved_math_shape_bindings: dict[int, dict[str, int]] = field(default_factory=dict)
    module_contracts: dict[str, Any] = field(default_factory=dict)
    imported_contracts: tuple[Any, ...] = ()
    project_parameters: tuple[Any, ...] = ()
    numeric_policy: dict[str, Any] = field(default_factory=dict)
    target_id: str | None = None
    runtime_id: str | None = None

_SCALARS = {
    "bool": ("boolean", None, None),
    "i8": ("integer", 8, True), "i16": ("integer", 16, True), "i32": ("integer", 32, True), "i64": ("integer", 64, True),
    "u8": ("integer", 8, False), "u16": ("integer", 16, False), "u32": ("integer", 32, False), "u64": ("integer", 64, False),
    "f32": ("float", 32, True), "f64": ("float", 64, True),
    "q15": ("fixed", 16, True), "q16": ("fixed", 32, True), "q31": ("fixed", 32, True), "q63": ("fixed", 64, True),
}

def _math_result_type_spec(value: Any, span: SourceSpan | None, shape_bindings: dict[str, int] | None = None) -> TypeSpec | None:
    """Translate a registry-owned logical result descriptor into a source type node."""
    if isinstance(value, str):
        return TypeSpec(value, span=span)
    if not isinstance(value, dict):
        return None
    name = value.get("name", value.get("c_name", value.get("type")))
    if not isinstance(name, str) or not name:
        return None
    if name == "numeric_scalar":
        name = "scalar"
    shape = value.get("shape", ())
    if not isinstance(shape, (list, tuple)) or any(not isinstance(item, (int, str)) for item in shape):
        return None
    resolved_shape: list[int | str] = []
    for dimension in shape:
        if isinstance(dimension, str) and dimension in (shape_bindings or {}):
            resolved_shape.append((shape_bindings or {})[dimension])
        elif isinstance(dimension, str):
            return None
        else:
            resolved_shape.append(dimension)
    unit = value.get("unit") if isinstance(value.get("unit"), str) else None
    arguments = value.get("arguments", ())
    if not isinstance(arguments, (list, tuple)):
        return None
    type_arguments: list[TypeSpec] = []
    for argument in arguments:
        nested = _math_result_type_spec(argument, span, shape_bindings)
        if nested is None:
            return None
        type_arguments.append(nested)
    return TypeSpec(name, tuple(resolved_shape), unit, span, tuple(type_arguments))


def _apply_math_result_unit(result: ResolvedType, descriptor: Any, arguments: tuple[ResolvedType, ...],
                            span: SourceSpan | None) -> tuple[ResolvedType, list[LanguageDiagnostic]]:
    """Apply reviewed unit propagation for UMATH result contracts."""
    if not isinstance(descriptor, dict):
        return result, []
    product = descriptor.get("unit_product_parameters")
    square_root = descriptor.get("unit_sqrt_parameter")
    if isinstance(product, list) and product:
        dimensions: tuple[tuple[str, int], ...] = ()
        scale = (1, 1, 0)
        units: list[str] = []
        for index in product:
            if not isinstance(index, int) or not 0 <= index < len(arguments):
                return result, [source_warning("math_result_unit_contract_invalid", "The UETD UMATH result unit contract references a missing argument.", span)]
            argument = arguments[index]
            if argument.affine:
                return result, [source_error("math_affine_product", "This UMATH operation cannot multiply affine absolute quantities.", span)]
            dimensions = _combine_dimensions(dimensions, argument.unit_dimensions, 1)
            scale = _combine_unit_scales(scale, argument.unit_scale, 1)
            if argument.unit and argument.unit != "1":
                units.append(argument.unit)
        unit = "*".join(units) if units else "1"
        return replace(result, unit=unit, unit_dimensions=dimensions, affine=False,
                       unit_scale=scale, unit_offset=(0, 1)), []
    if isinstance(square_root, int):
        if not 0 <= square_root < len(arguments):
            return result, [source_warning("math_result_unit_contract_invalid", "The UETD UMATH result unit contract references a missing argument.", span)]
        argument = arguments[square_root]
        numerator = math.isqrt(argument.unit_scale[0])
        denominator = math.isqrt(argument.unit_scale[1])
        if (argument.affine or any(power % 2 for _, power in argument.unit_dimensions)
                or numerator * numerator != argument.unit_scale[0]
                or denominator * denominator != argument.unit_scale[1]
                or argument.unit_scale[2] % 2):
            return result, [source_error("sqrt_unit_unrepresentable", "sqrt() requires unit dimensions and exact scales that have representable square roots.", span)]
        unit_pattern = re.compile(r"([*/]?)\s*([A-Za-z_][A-Za-z0-9_]*|1)(?:\^([+-]?\d+))?")
        pieces = list(unit_pattern.finditer(argument.unit or "1"))
        if not pieces or "".join(piece.group(0).replace("*", "") for piece in pieces) != (argument.unit or "1").replace("*", ""):
            return result, [source_warning("math_result_unit_contract_invalid", "The unit expression passed to sqrt() cannot be represented in the registered unit grammar.", span)]
        unit_parts: list[str] = []
        for piece in pieces:
            exponent = int(piece.group(3) or "1")
            if piece.group(2) == "1":
                continue
            if exponent % 2:
                return result, [source_error("sqrt_unit_unrepresentable", "sqrt() requires even powers in the input unit expression.", span)]
            power = exponent // 2
            atom = piece.group(2)
            unit_parts.append(f"{piece.group(1)}{atom}" + (f"^{power}" if power != 1 else ""))
        unit = "".join(unit_parts) or "1"
        return replace(result, unit=unit, unit_dimensions=tuple((name, power // 2) for name, power in argument.unit_dimensions if power // 2),
                       affine=False, unit_scale=(numerator, denominator, argument.unit_scale[2] // 2), unit_offset=(0, 1)), []
    return result, []


def _check_math_argument_units(contract: UETDMathFunctionContract, arguments: tuple[ResolvedType, ...],
                              environment: TypeEnvironment, span: SourceSpan | None) -> list[LanguageDiagnostic]:
    """Validate fixed unit requirements using UETD's exact unit catalogue."""
    diagnostics: list[LanguageDiagnostic] = []
    parameters = math_contract_parameter_types(contract)
    if parameters is None:
        return diagnostics
    for index, parameter in enumerate(parameters):
        if index >= len(arguments) or not isinstance(parameter, dict):
            continue
        expected_unit = parameter.get("unit")
        if not isinstance(expected_unit, str):
            continue
        dims, affine, scale, offset, error = _unit_mapping_facts(expected_unit, environment.unit_catalogue)
        if error:
            diagnostics.append(source_warning("math_unit_contract_invalid", f"UMATH contract for {contract.dsl_name!r} references invalid unit {expected_unit!r}: {error}", span))
            continue
        actual = arguments[index]
        if (actual.unit_dimensions != dims or actual.affine != affine
                or actual.unit_scale != scale or actual.unit_offset != offset):
            diagnostics.append(source_error("unit_mismatch", f"Unit {actual.unit or '1'} is incompatible with the {expected_unit} required by {contract.dsl_name}().", span))
    return diagnostics

def build_type_environment(document: ProjectDocument, language_registry: LanguageRegistry, catalogue: AuthoringCatalogue,
                           imported_contracts: tuple[Any, ...] = ()) -> tuple[TypeEnvironment, list[LanguageDiagnostic]]:
    """Build the visible type namespace once, preserving each source owner."""
    env = TypeEnvironment(language_registry=language_registry, unit_catalogue=catalogue.units,
                          math_catalogue=catalogue.math_functions,
                          math_operator_catalogue=catalogue.math_operators,
                          math_catalogue_by_c_name=catalogue.math_functions_by_c_name,
                          module_contracts=catalogue.dsl_descriptors,
                          imported_contracts=imported_contracts, project_parameters=document.parameters)
    numeric = next((item for item in document.declarations if item.kind == "numeric"), None)
    if numeric:
        env.numeric_policy = {prop.name: _constant_value(prop.value) for prop in numeric.properties}
    diagnostics: list[LanguageDiagnostic] = []
    kind, bits, signed = _SCALARS["bool"]
    env.language_types["bool"] = ResolvedType(TypeIdentity("language", "bool", "bool", kind), storage_bits=bits, signed=signed,
                                               unit="1", unit_dimensions=())
    project_definitions: list[tuple[str, Any]] = []
    project_definitions.extend((item.name, item) for item in document.records)
    project_definitions.extend((item.name, item) for item in document.enums)
    project_definitions.extend((item.name, item) for item in document.variants)
    project_definitions.extend((item.name, item) for item in document.flags)
    project_definitions.extend((item.name, item) for item in document.fixed_point_types)
    for name, definition in project_definitions:
        if name in env.language_types or name in env.project_types:
            diagnostics.append(source_error("type_name_collision", f"Project type {name!r} conflicts with a language type or another project type.", getattr(definition, "span", None)))
            continue
        if name in env.imported_types:
            diagnostics.append(source_error("type_name_collision", f"Project type {name!r} conflicts with an imported system type.", getattr(definition, "span", None)))
            continue
        kind = type(definition).__name__.removesuffix("Definition").lower()
        if kind == "fixedpoint":
            kind = "fixed"
        storage = getattr(definition, "storage_type", None) or getattr(definition, "tag_type", None)
        base = env.language_types.get(storage) if storage else None
        env.project_types[name] = ResolvedType(TypeIdentity("project", name, name, kind, owner_id=document.name,
                                                               source_path=str(document.source_path) if document.source_path else None),
                                               storage_bits=base.storage_bits if base else None,
                                               signed=base.signed if base else None,
                                               fractional_bits=getattr(definition, "fractional_bits", None),
                                               unit=getattr(definition, "unit", None),
                                               rounding=getattr(definition, "rounding", None),
                                               overflow=getattr(definition, "overflow", None))
        env.project_definitions[name] = definition
    for contract in imported_contracts:
        for item in (*contract.records, *contract.enums, *contract.variants, *contract.flags_types, *contract.fixed_point_types):
            name = item.name
            if name in env.language_types or name in env.project_types or name in env.imported_types:
                diagnostics.append(source_error("type_name_collision", f"Imported type {name!r} is ambiguous or shadowed.", getattr(item, "span", None)))
                continue
            kind = type(item).__name__.removesuffix("Definition").lower()
            # System-contract records come from the .uefs parser and use the
            # concrete AST name SystemRecordSpec.  Normalize both historical
            # and current names to the project type kind so member access on
            # imported types (including Received<T>.value.field) follows the
            # same field-resolution path as locally declared records.
            if kind in {"systemrecord", "systemrecordspec"}: kind = "record"
            if kind == "fixedpoint": kind = "fixed"
            storage_type = getattr(item, "storage_type", None) or getattr(item, "tag_type", None)
            storage = env.language_types.get(storage_type or "")
            env.imported_types[name] = ResolvedType(TypeIdentity("system_contract", name, name, kind,
                                                                   owner_id=contract.system_name, source_path=str(contract.source_path)),
                                                    storage_bits=storage.storage_bits if storage else None,
                                                    signed=storage.signed if storage else None,
                                                    fractional_bits=getattr(item, "fractional_bits", None),
                                                    unit=getattr(item, "unit", None),
                                                    rounding=getattr(item, "rounding", None),
                                                    overflow=getattr(item, "overflow", None))
            env.imported_definitions[name] = item
    for c_name, declarations in catalogue.types.items():
        resolved: list[ResolvedType] = []
        for item in declarations:
            if item.language_contract == "numeric":
                numeric = _SCALARS.get(item.dsl_name or "")
                if numeric is None or item.dsl_name == "bool":
                    diagnostics.append(source_error("invalid_uetd_numeric_type", f"UETD type {c_name!r} has an unsupported numeric DSL spelling."))
                    continue
                type_kind, storage_bits, signed = numeric
                identity_kind = type_kind
            elif item.language_contract == "fixed_numeric_vector":
                if item.vector_dimension is None or item.vector_dimension < 2:
                    diagnostics.append(source_error("invalid_uetd_vector_type", f"UETD vector type {c_name!r} has no valid fixed component count."))
                    continue
                identity_kind = "vector"
                scalar_name = str(env.numeric_policy.get("scalar", "f32"))
                scalar_type = env.language_types.get(scalar_name)
                storage_bits = scalar_type.storage_bits if scalar_type else None
                signed = scalar_type.signed if scalar_type else None
            elif item.language_contract == "enum":
                identity_kind, storage_bits, signed = "enum", None, None
            elif item.language_contract == "opaque":
                identity_kind, storage_bits, signed = "opaque", None, None
            else:
                identity_kind, storage_bits, signed = item.kind, None, None
            entry = ResolvedType(
                TypeIdentity("uetd", c_name, item.dsl_name or c_name, identity_kind, c_name=c_name,
                             source_path=item.header, visibility=item.language_visibility, readiness=item.source_status,
                             language_contract=item.language_contract, enum_members=item.enum_members, c_kind=item.kind),
                shape=(item.vector_dimension,) if item.language_contract == "fixed_numeric_vector" and item.vector_dimension is not None else (),
                storage_bits=storage_bits, signed=signed, unit="1", unit_dimensions=(),
            )
            resolved.append(entry)
            if item.dsl_name and item.language_contract:
                name_collision = item.dsl_name in env.language_types or item.dsl_name in env.project_types or item.dsl_name in env.imported_types
                duplicate_uetd_name = item.dsl_name in env.uetd_types_by_dsl_name
                if duplicate_uetd_name:
                    diagnostics.append(source_error("ambiguous_uetd_type_alias", f"UETD DSL type name {item.dsl_name!r} maps to more than one C type."))
                elif name_collision:
                    diagnostics.append(source_error("uetd_type_name_collision", f"UETD DSL type name {item.dsl_name!r} conflicts with another visible type."))
                else:
                    env.uetd_types_by_dsl_name[item.dsl_name] = entry
                if item.language_contract == "numeric" and not name_collision and not duplicate_uetd_name:
                    env.language_types[item.dsl_name] = entry
        env.uetd_types_by_c_name[c_name] = tuple(resolved)
    for name in _SCALARS.keys() - {"bool"}:
        if name not in env.language_types:
            diagnostics.append(source_error("missing_uetd_numeric_type", f"The UETD type registry does not provide the required numeric spelling {name!r}."))
    target = _find_property(document, "target")
    env.target_id = _value_name(target)
    env.runtime_id = _value_name(_find_nested_property(document, "operatingsystem", "runtime"))
    return env, diagnostics

def resolve_type_spec(type_spec: TypeSpec, environment: TypeEnvironment, *, boundary: TypeBoundary = "value") -> tuple[ResolvedType | None, list[LanguageDiagnostic]]:
    """Resolve a type spelling and enforce static bounds and boundary use."""
    name = type_spec.name
    diagnostics: list[LanguageDiagnostic] = []
    if name in {"Bytes", "Utf8"}:
        if len(type_spec.shape) != 1 or not isinstance(type_spec.shape[0], int) or type_spec.shape[0] <= 0:
            return None, [source_error("invalid_bounded_type", f"{name} requires one positive compile-time byte capacity.", type_spec.span)]
        item = ResolvedType(TypeIdentity("language", name, name, "bounded_bytes" if name == "Bytes" else "utf8"),
                            shape=(type_spec.shape[0],), boundary=boundary, maximum_encoded_bytes=type_spec.shape[0])
        return item, diagnostics
    if name in {"Received", "Option", "Result"}:
        arity = {"Received": 1, "Option": 1, "Result": 2}[name]
        if len(type_spec.type_arguments) != arity:
            return None, [source_error("generic_type_arity", f"{name} expects {arity} type argument(s).", type_spec.span)]
        args: list[ResolvedType] = []
        for argument in type_spec.type_arguments:
            resolved, errors = resolve_type_spec(argument, environment, boundary=boundary); diagnostics.extend(errors)
            if resolved is not None: args.append(resolved)
        if len(args) != arity: return None, diagnostics
        if name == "Received" and boundary not in {"value", "configuration"}:
            diagnostics.append(source_error("received_boundary", "Received<T> is runtime metadata and cannot be a wire/shared/persistent layout.", type_spec.span))
        return ResolvedType(TypeIdentity("language", name, name, "generic"), tuple(args), tuple(_resolved_shape(type_spec, environment, diagnostics)), boundary=boundary), diagnostics
    if name == "scalar":
        scalar_name = environment.numeric_policy.get("scalar", "f32")
        candidate = environment.language_types.get(scalar_name)
        if candidate is None:
            return None, [source_error("invalid_numeric_scalar", f"Numeric profile selects unknown scalar type {scalar_name!r}.", type_spec.span)]
    else:
        candidate = None
    candidate = (candidate or environment.language_types.get(name) or environment.project_types.get(name) or environment.imported_types.get(name)
                 or environment.uetd_types_by_dsl_name.get(name))
    if candidate is None:
        candidates = environment.uetd_types_by_c_name.get(name, ())
        if len(candidates) == 1:
            candidate = candidates[0]
            if candidate.identity.readiness == "future_header":
                diagnostics.append(source_warning("uetd_type_not_ready", f"UETD type {name!r} is declared only in a future header; it may not be available to generation.", type_spec.span))
                return None, diagnostics
            if candidate.identity.language_contract not in {"numeric", "enum", "opaque", "fixed_numeric_vector"}:
                diagnostics.append(source_error("uetd_type_not_visible", f"UETD type {name!r} is discoverable but has no by-value project-language contract.", type_spec.span))
                return None, diagnostics
        elif len(candidates) > 1:
            diagnostics.append(source_error("ambiguous_uetd_type", f"UETD C type name {name!r} is ambiguous.", type_spec.span)); return None, diagnostics
    if candidate is None:
        return None, [source_error("unknown_type", f"Type {name!r} is not a language, project, imported, or visible UETD type.", type_spec.span)]
    shape = _resolved_shape(type_spec, environment, diagnostics)
    if candidate.identity.language_contract == "fixed_numeric_vector":
        contract_shape = candidate.shape
        if shape and shape != contract_shape:
            diagnostics.append(source_error("uetd_vector_shape_mismatch", f"UETD vector {name!r} has fixed shape {contract_shape}; it cannot be redeclared with shape {shape}.", type_spec.span))
        shape = contract_shape
    unit = type_spec.unit or candidate.unit or "1"
    dimensions, affine, unit_scale, unit_offset, unit_error = _unit_mapping_facts(unit, environment.unit_catalogue)
    if unit_error: diagnostics.append(source_error("unknown_unit", unit_error, type_spec.span))
    if candidate.identity.kind in {"record", "enum", "variant", "flags", "fixedpoint", "opaque"} and type_spec.unit:
        diagnostics.append(source_error("unit_on_nominal_type", "Units may be attached only to numeric scalar types.", type_spec.span))
    result = replace(candidate, shape=shape, unit=unit, unit_dimensions=dimensions, affine=affine,
                     unit_scale=unit_scale, unit_offset=unit_offset, boundary=boundary)
    return result, diagnostics

def resolve_uetd_type(source_name: str, environment: TypeEnvironment, *, owning_module: str | None,
                     boundary: TypeBoundary, span: SourceSpan | None) -> tuple[ResolvedType | None, list[LanguageDiagnostic]]:
    """Resolve an exact UETD public type and require an owning contract."""
    del owning_module
    item = environment.uetd_types_by_dsl_name.get(source_name)
    if item is None:
        matches = environment.uetd_types_by_c_name.get(source_name, ())
        if len(matches) == 1: item = matches[0]
        elif len(matches) > 1: return None, [source_error("ambiguous_uetd_type", f"UETD type {source_name!r} is ambiguous.", span)]
    if item is None: return None, [source_error("unknown_uetd_type", f"UETD does not register a type named {source_name!r}.", span)]
    if item.identity.readiness != "public_header": return None, [source_warning("uetd_type_not_ready", f"UETD type {source_name!r} is not in a ready public header; generation may be unavailable.", span)]
    if item.identity.language_contract not in {"numeric", "enum", "opaque", "fixed_numeric_vector"}:
        return None, [source_error("uetd_type_not_value", f"UETD type {source_name!r} has no UEF-PL representation contract.", span)]
    return replace(item, boundary=boundary), []

def infer_expression_type(expression: Value, scope: dict[str, ResolvedType], environment: TypeEnvironment,
                          *, expected_type: ResolvedType | None = None) -> tuple[TypedExpressionFacts | None, list[LanguageDiagnostic]]:
    """Infer an expression recursively, rejecting implicit value conversions.

    Numeric literals are context-sensitive: ``500 ms`` compared with a
    ``u32<ms>`` value is an unsigned 32-bit literal, and ``0.05`` compared
    with an ``f32`` value is an ``f32`` literal. This is the same limited
    contextual typing used for literals in C-like languages; it never changes
    the type of a named value. Arithmetic keeps unitless scale factors
    unitless (for example ``2 * frequency``), while addition and comparison
    require matching dimensions.
    """
    errors: list[LanguageDiagnostic] = []
    span = getattr(expression, "span", None)

    def numeric_literal(value: Any) -> bool:
        if isinstance(value, ValueLiteral):
            return isinstance(value.value, (int, float)) and not isinstance(value.value, bool)
        if isinstance(value, UnaryExpression):
            return value.operator in {"+", "-"} and numeric_literal(value.operand)
        if isinstance(value, BinaryExpression):
            return value.operator in {"+", "-", "*", "/", "**"} and numeric_literal(value.left) and numeric_literal(value.right)
        if isinstance(value, SelectValue):
            return numeric_literal(value.if_true) and numeric_literal(value.if_false)
        if isinstance(value, ListValue):
            return bool(value.items) and all(numeric_literal(item) for item in value.items)
        if isinstance(value, MatrixLiteral):
            return bool(value.rows) and all(numeric_literal(item) for row in value.rows for item in row)
        return False

    def numeric_literal_values(value: Any) -> list[ValueLiteral]:
        if isinstance(value, ValueLiteral):
            return [value] if isinstance(value.value, (int, float)) and not isinstance(value.value, bool) else []
        if isinstance(value, UnaryExpression) and value.operator in {"+", "-"}:
            return numeric_literal_values(value.operand)
        if isinstance(value, BinaryExpression) and value.operator in {"+", "-", "*", "/", "**"}:
            return numeric_literal_values(value.left) + numeric_literal_values(value.right)
        if isinstance(value, SelectValue):
            return numeric_literal_values(value.if_true) + numeric_literal_values(value.if_false)
        if isinstance(value, ListValue):
            return [leaf for item in value.items for leaf in numeric_literal_values(item)]
        if isinstance(value, MatrixLiteral):
            return [leaf for row in value.rows for item in row for leaf in numeric_literal_values(item)]
        return []

    def scalar_numeric_type(value_type: ResolvedType) -> ResolvedType | None:
        if value_type.identity.kind in {"integer", "float", "fixed"}:
            return replace(value_type, shape=())
        if value_type.identity.kind == "vector":
            scalar_name = str(environment.numeric_policy.get("scalar", "f32"))
            return environment.language_types.get(scalar_name)
        return None

    def numeric_value_type(value_type: ResolvedType) -> bool:
        return value_type.identity.kind in {"integer", "float", "fixed", "vector"} or (
            value_type.identity.kind in {"integer", "float", "fixed"} and bool(value_type.shape)
        )

    def literal_context(value_type: ResolvedType, literal: Any, *, dimensionless: bool) -> ResolvedType | None:
        """Select a literal's scalar representation and preserve its authored unit."""
        scalar = scalar_numeric_type(value_type)
        if scalar is None:
            return None
        literal_value = literal
        while isinstance(literal_value, UnaryExpression) and literal_value.operator in {"+", "-"}:
            literal_value = literal_value.operand
        authored_unit = literal_value.unit if isinstance(literal_value, ValueLiteral) else None
        if authored_unit:
            dims, affine, unit_scale, unit_offset, message = _unit_mapping_facts(authored_unit, environment.unit_catalogue)
            if message:
                return replace(scalar, unit=authored_unit)
            return replace(scalar, unit=authored_unit, unit_dimensions=dims, affine=affine,
                           unit_scale=unit_scale, unit_offset=unit_offset)
        if dimensionless:
            return replace(scalar, unit="1", unit_dimensions=(), affine=False,
                           unit_scale=(1, 1, 0), unit_offset=(0, 1))
        return replace(scalar, unit=value_type.unit, unit_dimensions=value_type.unit_dimensions,
                       affine=value_type.affine, unit_scale=value_type.unit_scale,
                       unit_offset=value_type.unit_offset)

    def contextual_literal_type(value_type: ResolvedType, literal: Any, *, dimensionless: bool) -> ResolvedType | None:
        """Contextualize a scalar literal or a fixed aggregate of literals."""
        if isinstance(literal, (ListValue, MatrixLiteral)):
            return value_type
        return literal_context(value_type, literal, dimensionless=dimensionless)

    def infer(value: Any, expected: ResolvedType | None = None) -> ResolvedType | None:
        nonlocal errors
        if isinstance(value, ValueLiteral):
            if isinstance(value.value, bool): return environment.language_types["bool"]
            if isinstance(value.value, str):
                if expected and expected.identity.kind == "utf8":
                    encoded_size = len(value.value.encode("utf-8"))
                    if encoded_size > (expected.maximum_encoded_bytes or 0):
                        errors.append(source_error("utf8_literal_too_long", f"UTF-8 literal uses {encoded_size} bytes but the declared capacity is {expected.maximum_encoded_bytes}.", value.span))
                        return None
                    return expected
                return ResolvedType(TypeIdentity("language", "string", "string", "compile_time_string"))
            unit = value.unit or "1"
            dims, affine, unit_scale, unit_offset, message = _unit_mapping_facts(unit, environment.unit_catalogue)
            if message: errors.append(source_error("unknown_unit", message, value.span))
            if expected and expected.identity.kind in {"integer", "float", "fixed"}:
                if value.unit and (dims != expected.unit_dimensions or unit_scale != expected.unit_scale or unit_offset != expected.unit_offset):
                    errors.append(source_error("unit_mismatch", "A unit-bearing literal must use the expected unit scale and offset; use convert_unit explicitly for conversion.", value.span))
                # Keep checking in the surrounding representation after a unit
                # error so users receive the specific unit diagnostic instead
                # of a redundant signedness/float-width mismatch as well.
                return expected
            primitive = "bool" if isinstance(value.value, bool) else ("i32" if isinstance(value.value, int) else "f64")
            base = environment.language_types[primitive]
            return replace(base, unit=unit, unit_dimensions=dims, affine=affine, unit_scale=unit_scale, unit_offset=unit_offset)
        if isinstance(value, (Identifier, Reference)):
            key = value.value if isinstance(value, Identifier) else ".".join(value.path)
            found = scope.get(key)
            if found is None and isinstance(value, Reference) and len(value.path) >= 2:
                # Composition outputs and module ports are indexed by their
                # qualified owner (for example `estimator.health`). Resolve
                # the longest registered prefix, then walk the record fields.
                for prefix_length in range(len(value.path) - 1, 0, -1):
                    base = scope.get(".".join(value.path[:prefix_length]))
                    if base is None:
                        continue
                    found, member_errors = _resolve_record_member(base, value.path[prefix_length:], environment)
                    errors.extend(member_errors)
                    if found is not None:
                        break
                if found is None:
                    enum_type = (environment.project_types.get(value.path[0]) or environment.imported_types.get(value.path[0])
                                 or environment.uetd_types_by_dsl_name.get(value.path[0]))
                    if enum_type is None:
                        candidates = environment.uetd_types_by_c_name.get(value.path[0], ())
                        if len(candidates) == 1:
                            enum_type = candidates[0]
                    if enum_type is not None and enum_type.identity.kind == "enum":
                        definition = environment.project_definitions.get(value.path[0]) or environment.imported_definitions.get(value.path[0])
                        case_names = (set(enum_type.identity.enum_members) if enum_type.identity.owner == "uetd"
                                      else {item.name for item in getattr(definition, "variants", ())})
                        if value.path[1] in case_names and len(value.path) == 2:
                            found = enum_type
                    elif enum_type is not None and enum_type.identity.kind == "flags":
                        definition = environment.project_definitions.get(value.path[0]) or environment.imported_definitions.get(value.path[0])
                        member_names = {item.name for item in getattr(definition, "members", ())}
                        if value.path[1] in member_names and len(value.path) == 2:
                            found = enum_type
            if found is None:
                errors.append(source_error("unknown_value", f"Value {key!r} is not declared in this expression scope.", span)); return None
            return found
        if isinstance(value, UnaryExpression):
            operand = infer(value.operand, expected if value.operator in {"+", "-"} else None)
            if operand is None: return None
            if value.operator == "not":
                if operand.identity.canonical_name != "bool": errors.append(source_error("boolean_operand_required", "'not' requires a bool operand.", value.span))
                return environment.language_types["bool"]
            if operand.identity.kind not in {"integer", "float", "fixed", "vector"} and not operand.shape:
                errors.append(source_error("numeric_operand_required", "Unary sign requires a numeric scalar, vector, or fixed-shape numeric array.", value.span))
            return operand
        if isinstance(value, BinaryExpression):
            op = value.operator
            left: ResolvedType | None
            right: ResolvedType | None
            if op in {"==", "!=", "<", "<=", ">", ">=", "+", "-", "*", "/"}:
                # A bare number next to a typed value adopts only the numeric
                # representation of that value. Multiplication/division use
                # a dimensionless literal factor unless the literal spells a
                # unit explicitly.
                if numeric_literal(value.left) and not numeric_literal(value.right):
                    right = infer(value.right)
                    left_expected = contextual_literal_type(
                        right,
                        value.left,
                        dimensionless=op in {"*", "/"},
                    ) if right is not None else None
                    left = infer(value.left, left_expected)
                else:
                    left = infer(value.left)
                    if numeric_literal(value.right) and left is not None:
                        right_expected = contextual_literal_type(
                            left,
                            value.right,
                            dimensionless=op in {"*", "/"},
                        )
                    else:
                        right_expected = None
                    right = infer(value.right, right_expected)
            else:
                left = infer(value.left); right = infer(value.right)
            if left is None or right is None: return None
            operator_contracts = environment.math_operator_catalogue.get(op, ())
            if operator_contracts:
                shape_bindings: dict[str, int] = {}
                try:
                    operator_contract = resolve_math_overload(
                        op,
                        operator_contracts,
                        argument_types=(left, right),
                        context=environment.math_call_contexts.get(id(value), "configuration"),
                        require_ready=False,
                        shape_bindings=shape_bindings,
                        numeric_scalar_name=str(environment.numeric_policy.get("scalar", "f32")),
                    )
                except TypeError as exc:
                    if "No overload" not in str(exc):
                        errors.append(source_error("ambiguous_math_operator", str(exc), value.span))
                        return None
                except ValueError as exc:
                    if "not permitted" not in str(exc):
                        errors.append(source_error("math_operator_contract_missing", str(exc), value.span))
                        return None
                else:
                    environment.resolved_math_bindings[id(value)] = operator_contract
                    environment.resolved_math_shape_bindings[id(value)] = shape_bindings
                    errors.extend(_check_math_argument_units(operator_contract, (left, right), environment, value.span))
                    if not operator_contract.available_for_generation or operator_contract.dsl_binding_status != "ready":
                        errors.append(source_warning(
                            "math_function_not_ready",
                            f"UMATH operator overload {operator_contract.dsl_signature or operator_contract.c_name!r} is registered, but its implementation/binding readiness is {operator_contract.implementation_status}/{operator_contract.dsl_binding_status}.",
                            value.span,
                        ))
                    result_descriptor = math_contract_result_type(operator_contract, (left, right))
                    result_spec = _math_result_type_spec(result_descriptor, value.span, shape_bindings)
                    if result_spec is None:
                        errors.append(source_warning("math_result_contract_missing", f"UMATH operator overload {operator_contract.c_name!r} has no result type available for frontend inference; generation requires a reviewed binding contract.", value.span))
                        return None
                    result, more = resolve_type_spec(result_spec, environment)
                    errors.extend(more)
                    if result is not None:
                        result, more = _apply_math_result_unit(result, result_descriptor, (left, right), value.span)
                        errors.extend(more)
                    return result
            if op in {"and", "or"}:
                if left.identity.canonical_name != "bool" or right.identity.canonical_name != "bool": errors.append(source_error("boolean_operands_required", f"{op!r} requires bool operands.", value.span))
                return environment.language_types["bool"]
            if op in {"==", "!=", "<", "<=", ">", ">="}:
                mismatch = check_assignable(left, right, span=value.span)
                errors.extend(mismatch)
                if op not in {"==", "!="} and left.identity.kind not in {"integer", "float", "fixed"}:
                    errors.append(source_error("ordered_comparison_requires_numeric", "Ordered comparisons require numeric scalars.", value.span))
                return environment.language_types["bool"]
            if op in {"|", "&", "^"}:
                result, more = check_flags_operation(op, left, right, span=value.span); errors.extend(more); return result
            if not numeric_value_type(left) or not numeric_value_type(right):
                errors.append(source_error("numeric_operands_required", f"Operator {op!r} requires numeric scalars, vectors, or fixed-shape arrays.", value.span)); return None
            if op in {"+", "-"}:
                if left.identity.kind == right.identity.kind == "fixed":
                    result, _metadata, more = check_fixed_point_operation(op, (left, right), numeric_policy=environment.numeric_policy, span=value.span)
                    errors.extend(more)
                    return result
                if left.affine or right.affine:
                    if op == "+" or not (left.affine and right.affine and left.unit_scale == right.unit_scale and left.unit_offset == right.unit_offset):
                        errors.append(source_error("affine_arithmetic", "Affine absolute quantities cannot be added or mixed with linear values.", value.span)); return None
                    delta_name = f"delta_{left.unit}"
                    delta_dims, delta_affine, delta_scale, delta_offset, delta_error = _unit_mapping_facts(delta_name, environment.unit_catalogue)
                    if delta_error or delta_affine or delta_dims != left.unit_dimensions or delta_scale != left.unit_scale:
                        errors.append(source_error("affine_delta_unit_missing", f"UETD must register a matching linear delta unit for {left.unit!r}.", value.span)); return None
                    return replace(left, unit=delta_name, affine=False, unit_offset=delta_offset)
                errors.extend(check_assignable(right, left, span=value.span)); return left
            if op == "*":
                if left.identity.kind == right.identity.kind == "fixed":
                    result, _metadata, more = check_fixed_point_operation(op, (left, right), numeric_policy=environment.numeric_policy, span=value.span)
                    errors.extend(more)
                    if result is None: return None
                    dims = _combine_dimensions(left.unit_dimensions, right.unit_dimensions, 1)
                    return replace(result, unit=_combine_unit_expression(left.unit, right.unit, 1), unit_dimensions=dims, affine=False,
                                   unit_scale=_combine_unit_scales(left.unit_scale, right.unit_scale, 1), unit_offset=(0, 1))
                # Scaling is componentwise for a scalar and one vector/matrix;
                # vector-by-vector multiplication is deliberately not guessed
                # as a dot product. Matrix products use their declared shapes.
                if left.shape and not right.shape:
                    shape = left.shape
                    result_base = left
                elif right.shape and not left.shape:
                    shape = right.shape
                    result_base = right
                elif left.shape and right.shape:
                    if len(left.shape) == 2 and len(right.shape) == 2 and left.shape[1] == right.shape[0]:
                        shape = (left.shape[0], right.shape[1])
                        result_base = left
                    elif len(left.shape) == 2 and len(right.shape) == 1 and left.shape[1] == right.shape[0]:
                        shape = (left.shape[0],)
                        result_base = right if right.identity.kind == "vector" else left
                    else:
                        errors.append(source_error("matrix_shape_mismatch", "Matrix product dimensions do not align; vector-by-vector multiplication is not defined.", value.span)); return None
                else:
                    shape = ()
                    result_base = left
                if left.affine or right.affine:
                    errors.append(source_error("affine_product", "Affine absolute quantities cannot be multiplied.", value.span)); return None
                dims = _combine_dimensions(left.unit_dimensions, right.unit_dimensions, 1)
                scale = _combine_unit_scales(left.unit_scale, right.unit_scale, 1)
                return replace(result_base, shape=shape, unit=_combine_unit_expression(left.unit, right.unit, 1), unit_dimensions=dims, affine=False,
                               unit_scale=scale, unit_offset=(0, 1))
            if op == "/":
                if left.identity.kind == right.identity.kind == "fixed":
                    result, _metadata, more = check_fixed_point_operation(op, (left, right), numeric_policy=environment.numeric_policy, span=value.span)
                    errors.extend(more)
                    if result is None: return None
                    dims = _combine_dimensions(left.unit_dimensions, right.unit_dimensions, -1)
                    return replace(result, unit=_combine_unit_expression(left.unit, right.unit, -1), unit_dimensions=dims, affine=False,
                                   unit_scale=_combine_unit_scales(left.unit_scale, right.unit_scale, -1), unit_offset=(0, 1))
                if right.shape: errors.append(source_error("matrix_division_unsupported", "Division by a matrix or vector is not supported.", value.span)); return None
                if left.affine or right.affine:
                    errors.append(source_error("affine_product", "Affine absolute quantities cannot be divided.", value.span)); return None
                dims = _combine_dimensions(left.unit_dimensions, right.unit_dimensions, -1)
                scale = _combine_unit_scales(left.unit_scale, right.unit_scale, -1)
                return replace(left, unit=_combine_unit_expression(left.unit, right.unit, -1), unit_dimensions=dims, affine=False,
                               unit_scale=scale, unit_offset=(0, 1))
            if op == "**":
                exponent = value.right.value if isinstance(value.right, ValueLiteral) else None
                if not isinstance(exponent, int) or not -16 <= exponent <= 16: errors.append(source_error("power_requires_bounded_integer", "Exponentiation requires a constant integer exponent from -16 through 16.", value.span)); return None
                if left.affine:
                    errors.append(source_error("affine_power", "Affine absolute quantities cannot be exponentiated.", value.span)); return None
                dims = tuple((dim, power * exponent) for dim, power in left.unit_dimensions if power * exponent)
                scale = _power_unit_scale(left.unit_scale, exponent)
                return replace(left, unit=_power_unit_expression(left.unit, exponent), unit_dimensions=dims, affine=False,
                               unit_scale=scale, unit_offset=(0, 1))
        if isinstance(value, CastExpression):
            destination, more = resolve_type_spec(value.destination, environment); errors.extend(more)
            # The destination gives a record constructor the nominal type it
            # needs before checking the fields inside map(...).
            source = infer(value.value, destination)
            if source is None or destination is None: return None
            facts = TypedExpressionFacts(value.value, source)
            cast, more = check_explicit_cast(facts, destination, span=value.span); errors.extend(more)
            return cast.resolved_type if cast else None
        if isinstance(value, UnitConversionExpression):
            source = infer(value.value)
            if source is None: return None
            converted, more = check_unit_conversion(TypedExpressionFacts(value.value, source), value.target_unit, environment, span=value.span); errors.extend(more)
            return converted.resolved_type if converted else None
        if isinstance(value, IndexExpression):
            base = infer(value.value)
            if base is None: return None
            if not base.shape: errors.append(source_error("index_non_array", "Only fixed-shape arrays can be indexed.", value.span)); return None
            if len(value.subscripts) > len(base.shape): errors.append(source_error("too_many_indices", "The index has more dimensions than the value.", value.span)); return None
            result_shape = list(base.shape)
            for index, subscript in enumerate(value.subscripts):
                idx = infer(subscript.start)
                if idx and idx.identity.kind != "integer": errors.append(source_error("integer_index_required", "Array indices must be integers.", value.span))
                if subscript.end is not None:
                    end_type = infer(subscript.end)
                    if end_type and end_type.identity.kind != "integer": errors.append(source_error("integer_slice_bound_required", "Slice bounds must be integers.", value.span))
                else: result_shape[index] = 0
            return replace(base, shape=tuple(size for size in result_shape if size))
        if isinstance(value, ListValue):
            if not value.items: errors.append(source_error("empty_vector", "A vector literal cannot be empty.", value.span)); return None
            if expected is not None and expected.identity.kind == "vector":
                scalar_name = str(environment.numeric_policy.get("scalar", "f32"))
                scalar_type = environment.language_types.get(scalar_name)
                expected_item = replace(scalar_type, unit=expected.unit, unit_dimensions=expected.unit_dimensions,
                                        affine=expected.affine, unit_scale=expected.unit_scale,
                                        unit_offset=expected.unit_offset, boundary=expected.boundary) if scalar_type else None
            else:
                expected_item = replace(expected, shape=()) if expected is not None and expected.shape else (expected.arguments[0] if expected and expected.arguments else None)
            if expected_item is None and numeric_literal(value):
                leaves = numeric_literal_values(value)
                scalar_name = "f64" if any(isinstance(item.value, float) for item in leaves) else "i32"
                scalar = environment.language_types.get(scalar_name)
                if scalar is not None and leaves:
                    expected_item = literal_context(scalar, leaves[0], dimensionless=False)
            item_types = [infer(item, expected_item) for item in value.items]
            first = next((item for item in item_types if item is not None), None)
            for item in item_types:
                if item and first: errors.extend(check_assignable(item, first, span=value.span))
            shape = expected.shape if expected is not None and expected.shape else (len(value.items),)
            if shape != (len(value.items),):
                errors.append(source_error("vector_shape_mismatch", f"Vector has {len(value.items)} entries but the declared shape is {shape}.", value.span))
            if expected is not None and expected.identity.kind == "vector":
                return expected
            return replace(first, shape=shape) if first else None
        if isinstance(value, MatrixLiteral):
            if not value.rows or not value.rows[0]: errors.append(source_error("empty_matrix", "A matrix literal must have at least one non-empty row.", value.span)); return None
            sizes = {len(row) for row in value.rows}
            if len(sizes) != 1: errors.append(source_error("ragged_matrix", "Matrix rows must have equal lengths.", value.span)); return None
            expected_item = replace(expected, shape=()) if expected is not None and expected.shape else None
            if expected_item is None and numeric_literal(value):
                leaves = numeric_literal_values(value)
                scalar_name = "f64" if any(isinstance(item.value, float) for item in leaves) else "i32"
                scalar = environment.language_types.get(scalar_name)
                if scalar is not None and leaves:
                    expected_item = literal_context(scalar, leaves[0], dimensionless=False)
            cells = [infer(cell, expected_item) for row in value.rows for cell in row]
            first = next((item for item in cells if item is not None), None)
            for item in cells:
                if item and first: errors.extend(check_assignable(item, first, span=value.span))
            shape = (len(value.rows), len(value.rows[0]))
            if expected is not None and expected.shape and expected.shape != shape:
                errors.append(source_error("matrix_shape_mismatch", f"Matrix literal has shape {shape}; the declared shape is {expected.shape}.", value.span))
            return replace(first, shape=shape) if first else None
        if isinstance(value, RecordValue):
            if expected is None or expected.identity.kind != "record": errors.append(source_error("map_requires_expected_record", "map(...) needs an expected record type from its declaration or output contract.", value.span)); return None
            definition = environment.project_definitions.get(expected.identity.canonical_name) or environment.imported_definitions.get(expected.identity.canonical_name)
            expected_fields = {
                name: type_spec
                for name, type_spec in (_record_field_parts(field) for field in getattr(definition, "fields", ()))
            }
            supplied_fields = {key for key, _ in value.fields}
            missing_fields = sorted(set(expected_fields) - supplied_fields)
            if missing_fields: errors.append(source_error("missing_record_fields", f"Record value is missing fields: {', '.join(missing_fields)}.", value.span))
            for key, item in value.fields:
                if key not in expected_fields: errors.append(source_error("unknown_record_field", f"Record has no field {key!r}.", value.span)); continue
                field_type, more = resolve_type_spec(expected_fields[key], environment); errors.extend(more)
                actual = infer(item, field_type)
                if actual and field_type: errors.extend(check_assignable(actual, field_type, span=getattr(item, "span", None)))
            return expected
        if isinstance(value, SelectValue):
            condition = infer(value.condition)
            if numeric_literal(value.if_true) and not numeric_literal(value.if_false):
                no = infer(value.if_false, expected)
                yes_expected = contextual_literal_type(no, value.if_true, dimensionless=False) if no is not None else expected
                yes = infer(value.if_true, yes_expected or expected)
            else:
                yes = infer(value.if_true, expected)
                no_expected = expected
                if numeric_literal(value.if_false) and yes is not None:
                    no_expected = contextual_literal_type(yes, value.if_false, dimensionless=False) or expected
                no = infer(value.if_false, no_expected)
            if condition and condition.identity.canonical_name != "bool": errors.append(source_error("select_condition_bool", "select condition must be bool.", value.condition.span))
            if yes and no: errors.extend(check_assignable(no, yes, span=value.span))
            return yes
        if isinstance(value, MathCallExpression):
            contracts = environment.math_catalogue.get(value.callee, ())
            if not contracts:
                if value.callee == "jacobian" and value.callee in environment.module_contracts:
                    errors.append(source_warning(
                        "uetd_module_not_ready",
                        "Jacobian construction is present in the UETD module registry, but no callable UEF-PL binding is available yet.",
                        value.span,
                    ))
                    return None
                hidden = environment.math_catalogue_by_c_name.get(value.callee)
                message = (f"Use DSL name {hidden.dsl_name!r}; raw UMATH C names are hidden."
                           if hidden else f"UMATH function {value.callee!r} is not registered by its DSL name.")
                errors.append(source_error("unknown_math_function", message, value.span)); return None
            call_context = environment.math_call_contexts.get(id(value), "configuration")
            parameter_contracts = [math_contract_parameter_types(item) for item in contracts
                                   if call_context in item.callable_contexts]
            argument_types_list: list[ResolvedType | None] = [None] * len(value.arguments)
            # Resolve the nonliteral arguments first. A contract can then lend
            # an unadorned bound literal the representation and unit of its
            # paired value (for example clamp(speed, 0, limit)).
            for index, argument in enumerate(value.arguments):
                if not numeric_literal(argument):
                    argument_types_list[index] = infer(argument)
            for index, argument in enumerate(value.arguments):
                if argument_types_list[index] is not None or not numeric_literal(argument):
                    continue
                references: set[int] = set()
                for parameters in parameter_contracts:
                    if parameters is None or len(parameters) != len(value.arguments):
                        continue
                    descriptor = parameters[index]
                    linked = descriptor.get("unit_from_parameter") if isinstance(descriptor, dict) else None
                    if isinstance(linked, int) and linked != index:
                        references.add(linked)
                    for other_index, other in enumerate(parameters):
                        reverse = other.get("unit_from_parameter") if isinstance(other, dict) else None
                        if reverse == index and other_index != index:
                            references.add(other_index)
                if len(references) == 1:
                    reference = next(iter(references))
                    reference_type = argument_types_list[reference]
                    if reference_type is not None:
                        expected = contextual_literal_type(reference_type, argument, dimensionless=False)
                        argument_types_list[index] = infer(argument, expected)
                if argument_types_list[index] is None:
                    argument_types_list[index] = infer(argument)
            argument_types = tuple(argument_types_list)
            if any(item is None for item in argument_types):
                return None
            shape_bindings: dict[str, int] = {}
            try:
                contract = resolve_math_overload(
                    value.callee, contracts, argument_types=argument_types,
                    context=call_context,
                    require_ready=False, shape_bindings=shape_bindings,
                    numeric_scalar_name=str(environment.numeric_policy.get("scalar", "f32")),
                )
            except TypeError as exc:
                message = str(exc)
                code = "math_argument_type_mismatch" if "No overload" in message else "ambiguous_math_overload"
                errors.append(source_error(code, message, value.span))
                return None
            except ValueError as exc:
                code = "math_overload_contract_missing" if "metadata" in str(exc) else "math_overload_context"
                errors.append(source_warning(code, str(exc), value.span))
                return None
            environment.resolved_math_bindings[id(value)] = contract
            environment.resolved_math_shape_bindings[id(value)] = shape_bindings
            errors.extend(_check_math_argument_units(contract, argument_types, environment, value.span))
            if not contract.available_for_generation or contract.dsl_binding_status != "ready":
                errors.append(source_warning(
                    "math_function_not_ready",
                    f"UMATH overload {contract.dsl_signature or contract.c_name!r} is registered, but its implementation/binding readiness is {contract.implementation_status}/{contract.dsl_binding_status}.",
                    value.span,
                ))
            result_descriptor = math_contract_result_type(contract, argument_types)
            result_spec = _math_result_type_spec(result_descriptor, value.span, shape_bindings)
            if result_spec is None:
                errors.append(source_warning("math_result_contract_missing", f"UMATH overload {contract.c_name!r} has no result type available for frontend inference; generation requires a reviewed binding contract.", value.span))
                return None
            result, more = resolve_type_spec(result_spec, environment); errors.extend(more)
            if result is not None:
                result, more = _apply_math_result_unit(result, result_descriptor, argument_types, value.span)
                errors.extend(more)
            return result
        if isinstance(value, VariantConstructorExpression):
            if expected is None or expected.identity.kind not in {"variant", "generic"}:
                errors.append(source_error("variant_expected_type", "Variant constructors require an expected declared variant, Option, or Result type.", value.span)); return None
            path = tuple(value.constructor.split("."))
            constructor_name = path[-1]
            if expected.identity.kind == "generic" and expected.identity.canonical_name in {"Option", "Result"}:
                generic = expected.identity.canonical_name
                cases = {"Option": {"Some", "None"}, "Result": {"Ok", "Err"}}[generic]
                if constructor_name not in cases:
                    errors.append(source_error("unknown_variant_constructor", f"{generic} has no constructor {constructor_name!r}.", value.span)); return None
                if len(path) > 1 and path[-2] != generic:
                    errors.append(source_error("variant_constructor_owner", f"Constructor {value.constructor!r} does not belong to {generic}.", value.span)); return None
                payload_index = ({"Some": 0}.get(constructor_name) if generic == "Option" else {"Ok": 0, "Err": 1}.get(constructor_name))
                if payload_index is None:
                    if value.positional_value is not None or value.fields:
                        errors.append(source_error("empty_variant_payload", f"{generic}.{constructor_name} has no payload.", value.span))
                else:
                    if value.positional_value is None or value.fields or len(expected.arguments) <= payload_index:
                        errors.append(source_error("variant_payload_required", f"{generic}.{constructor_name} requires one positional payload value.", value.span))
                    else:
                        infer(value.positional_value, expected.arguments[payload_index])
                return expected
            definition = environment.project_definitions.get(expected.identity.canonical_name) or environment.imported_definitions.get(expected.identity.canonical_name)
            if len(path) != 2 or path[0] != expected.identity.canonical_name:
                errors.append(source_error("qualified_variant_constructor_required", f"Use {expected.identity.canonical_name}.case(...) to construct this variant.", value.span)); return None
            cases = {case.name: case for case in getattr(definition, "cases", ())}
            case = cases.get(constructor_name)
            if case is None:
                errors.append(source_error("unknown_variant_constructor", f"Variant {expected.identity.canonical_name!r} has no case {constructor_name!r}.", value.span)); return None
            fields = {item.name: item for item in case.fields}
            if value.positional_value is not None:
                if value.fields or len(fields) != 1:
                    errors.append(source_error("variant_positional_payload", "Positional construction is allowed only for a case with exactly one field.", value.span))
                else:
                    field = next(iter(fields.values()))
                    field_type, more = resolve_type_spec(field.type_spec, environment); errors.extend(more)
                    if field_type: infer(value.positional_value, field_type)
            else:
                supplied = dict(value.fields)
                if set(supplied) != set(fields):
                    missing = sorted(set(fields) - set(supplied)); unknown = sorted(set(supplied) - set(fields))
                    errors.append(source_error("variant_payload_fields", f"Variant payload fields do not match; missing={missing}, unknown={unknown}.", value.span))
                for name, expression in value.fields:
                    field = fields.get(name)
                    if field is None: continue
                    field_type, more = resolve_type_spec(field.type_spec, environment); errors.extend(more)
                    if field_type: infer(expression, field_type)
            return expected
        if isinstance(value, MatchExpression):
            subject = infer(value.subject)
            if subject is None: return None
            match, more = check_variant_match(value, subject, scope, environment, expected_result_type=expected); errors.extend(more)
            return match.resolved_type if match else None
        if isinstance(value, FlagsOperationExpression):
            operand = infer(value.value)
            if operand is None: return None
            type_name, separator, member = value.member.partition(".")
            if not separator or type_name != operand.identity.canonical_name:
                errors.append(source_error("flags_member_type_mismatch", "Flags member must be qualified by the operand's nominal flags type.", value.span))
            else:
                definition = environment.project_definitions.get(type_name) or environment.imported_definitions.get(type_name)
                if member not in {item.name for item in getattr(definition, "members", ())}:
                    errors.append(source_error("unknown_flags_member", f"Flags type {type_name!r} has no member {member!r}.", value.span))
            result, more = check_flags_operation(value.operation, operand, operand, span=value.span); errors.extend(more)
            return environment.language_types["bool"] if value.operation == "has" else result
        errors.append(source_error("unsupported_expression", f"Expression node {type(value).__name__} has no type rule.", span))
        return None
    result = infer(expression, expected_type)
    if result is None: return None, errors
    if expected_type is not None: errors.extend(check_assignable(result, expected_type, span=span))
    if any(item.severity == "error" for item in errors): return None, errors
    operations = _expression_cost(expression)
    return TypedExpressionFacts(expression, result, worst_case_operations=operations), errors

def check_assignable(actual: ResolvedType, expected: ResolvedType, *, span: SourceSpan | None) -> list[LanguageDiagnostic]:
    """Require nominal type, shape, and dimensional unit identity."""
    if actual.identity != expected.identity:
        return [source_error("type_mismatch", f"Cannot use {actual.identity.source_name} where {expected.identity.source_name} is required.", span)]
    if actual.arguments != expected.arguments:
        return [source_error("generic_type_mismatch", "Generic payload types must match exactly.", span)]
    if actual.shape != expected.shape:
        return [source_error("shape_mismatch", f"Shape {actual.shape} does not match required shape {expected.shape}.", span)]
    if (actual.unit_dimensions != expected.unit_dimensions or actual.affine != expected.affine
            or actual.unit_scale != expected.unit_scale or actual.unit_offset != expected.unit_offset):
        return [source_error("unit_mismatch", f"Unit {actual.unit or '1'} is incompatible with {expected.unit or '1'}.", span)]
    return []

def check_explicit_cast(value: TypedExpressionFacts, destination: ResolvedType, *, span: SourceSpan | None) -> tuple[TypedExpressionFacts | None, list[LanguageDiagnostic]]:
    """Allow checked numeric scalar representation casts only."""
    source = value.resolved_type
    numeric = {"integer", "float", "fixed"}
    if source.identity.kind not in numeric or destination.identity.kind not in numeric or source.shape or destination.shape:
        return None, [source_error("invalid_cast", "cast<T>(value) is permitted only between numeric scalar representations.", span)]
    if (source.unit_dimensions != destination.unit_dimensions or source.affine != destination.affine
            or source.unit_scale != destination.unit_scale or source.unit_offset != destination.unit_offset):
        return None, [source_error("cast_unit_mismatch", "A numeric cast cannot change physical units; use convert_unit<T> for that.", span)]
    return TypedExpressionFacts(value.expression, replace(destination, unit=source.unit, unit_dimensions=source.unit_dimensions,
                                                          affine=source.affine, unit_scale=source.unit_scale,
                                                          unit_offset=source.unit_offset), may_fail=True), []

def check_unit_conversion(value: TypedExpressionFacts, destination_unit: str, environment: TypeEnvironment,
                          *, span: SourceSpan | None) -> tuple[TypedExpressionFacts | None, list[LanguageDiagnostic]]:
    """Convert between registered units with identical dimensions."""
    dims, affine, unit_scale, unit_offset, error = _unit_mapping_facts(destination_unit, environment.unit_catalogue)
    if error: return None, [source_error("unknown_unit", error, span)]
    source = value.resolved_type
    if source.identity.kind not in {"integer", "float", "fixed"}:
        return None, [source_error("unit_conversion_requires_numeric", "Unit conversion requires a numeric value.", span)]
    if source.unit_dimensions != dims:
        return None, [source_error("unit_dimension_mismatch", f"Cannot convert {source.unit or '1'} to dimensionally different {destination_unit}.", span)]
    if source.affine != affine:
        return None, [source_error("affine_unit_kind_ambiguous", "UETD must distinguish absolute and delta quantities before converting between affine and linear units.", span)]
    converted = replace(source, unit=destination_unit, unit_dimensions=dims, affine=affine,
                        unit_scale=unit_scale, unit_offset=unit_offset)
    return TypedExpressionFacts(value.expression, converted, may_fail=True), []

def check_variant_match(match_expression: MatchExpression, subject_type: ResolvedType,
                        scope: dict[str, ResolvedType], environment: TypeEnvironment,
                        *, expected_result_type: ResolvedType | None = None) -> tuple[TypedExpressionFacts | None, list[LanguageDiagnostic]]:
    """Require unique, exhaustive variant cases and identical result types."""
    errors: list[LanguageDiagnostic] = []
    definition = environment.project_definitions.get(subject_type.identity.canonical_name) or environment.imported_definitions.get(subject_type.identity.canonical_name)
    if subject_type.identity.kind == "generic" and subject_type.identity.canonical_name in {"Option", "Result"}:
        generic = subject_type.identity.canonical_name
        cases = {"Option": {"Some", "None"}, "Result": {"Ok", "Err"}}[generic]
        payloads: dict[str, dict[str, ResolvedType]] = {}
        if generic == "Option" and subject_type.arguments:
            payloads["Some"] = {"value": subject_type.arguments[0]}
            payloads["None"] = {}
        elif generic == "Result" and len(subject_type.arguments) == 2:
            payloads["Ok"] = {"value": subject_type.arguments[0]}
            payloads["Err"] = {"error": subject_type.arguments[1]}
    else:
        variant_cases = {case.name: case for case in getattr(definition, "cases", ())}
        enum_cases = {case.name for case in getattr(definition, "variants", ())}
        if subject_type.identity.owner == "uetd" and subject_type.identity.kind == "enum":
            enum_cases = set(subject_type.identity.enum_members)
        cases = set(variant_cases) or enum_cases
        payloads = {}
        for name, case in variant_cases.items():
            values: dict[str, ResolvedType] = {}
            for field in case.fields:
                resolved, more = resolve_type_spec(field.type_spec, environment)
                errors.extend(more)
                if resolved is not None:
                    values[field.name] = resolved
            payloads[name] = values
    if not cases:
        errors.append(source_error("match_subject_not_matchable", f"Type {subject_type.identity.source_name!r} has no declared match cases.", match_expression.span))
    names = [arm.case_name.rsplit(".", 1)[-1] for arm in match_expression.arms]
    if len(names) != len(set(names)): errors.append(source_error("duplicate_match_arm", "Each match case may appear only once.", match_expression.span))
    unknown = set(names) - cases
    if unknown: errors.append(source_error("unknown_match_case", f"Unknown match case(s): {', '.join(sorted(unknown))}.", match_expression.span))
    if match_expression.default_expression is None and set(names) != cases:
        errors.append(source_error("non_exhaustive_match", f"Match must cover: {', '.join(sorted(cases - set(names)))}.", match_expression.span))
    results: list[ResolvedType] = []
    for arm in match_expression.arms:
        parts = arm.case_name.split(".")
        if subject_type.identity.kind == "generic":
            owner = subject_type.identity.canonical_name
            if len(parts) > 2 or (len(parts) == 2 and parts[0] != owner):
                errors.append(source_error("match_case_owner", f"{arm.case_name!r} is not a case of {owner}.", arm.span))
        elif subject_type.identity.kind in {"variant", "enum"} and (len(parts) != 2 or parts[0] != subject_type.identity.canonical_name):
            errors.append(source_error("qualified_match_case_required", f"Use {subject_type.identity.canonical_name}.case in this match.", arm.span))
        arm_scope = dict(scope)
        case_payloads = payloads.get(arm.case_name.rsplit(".", 1)[-1], {})
        if arm.binding_name:
            if len(case_payloads) != 1:
                errors.append(source_error("match_positional_binding_arity", "A positional match binding is allowed only for a case with exactly one payload field.", arm.span))
            else:
                arm_scope[arm.binding_name] = next(iter(case_payloads.values()))
        bound_fields: set[str] = set()
        for field_name, local_name in arm.field_bindings:
            field_type = case_payloads.get(field_name)
            if field_type is None:
                errors.append(source_error("unknown_match_payload_field", f"Case {arm.case_name!r} has no payload field {field_name!r}.", arm.span))
            else:
                if field_name in bound_fields:
                    errors.append(source_error("duplicate_match_payload_binding", f"Payload field {field_name!r} is bound more than once.", arm.span))
                bound_fields.add(field_name)
                arm_scope[local_name] = field_type
        value, more = infer_expression_type(arm.expression, arm_scope, environment, expected_type=expected_result_type); errors.extend(more)
        if value: results.append(value.resolved_type)
    if match_expression.default_expression is not None:
        value, more = infer_expression_type(match_expression.default_expression, scope, environment, expected_type=expected_result_type); errors.extend(more)
        if value: results.append(value.resolved_type)
    if not results: return None, errors
    for candidate in results[1:]: errors.extend(check_assignable(candidate, results[0], span=match_expression.span))
    if errors: return None, errors
    return TypedExpressionFacts(match_expression, results[0], worst_case_operations=max(1, len(results))), []

def check_flags_operation(operation: str, left: ResolvedType, right: ResolvedType | None,
                          *, span: SourceSpan | None) -> tuple[ResolvedType | None, list[LanguageDiagnostic]]:
    """Permit operations only on one nominal flags type."""
    if left.identity.kind != "flags" or (right is not None and right.identity != left.identity):
        return None, [source_error("flags_type_mismatch", "Flags operations require operands of the same nominal flags type.", span)]
    if operation not in {"|", "&", "^", "has", "set", "clear"}:
        return None, [source_error("invalid_flags_operation", f"Operation {operation!r} is not permitted for flags.", span)]
    return left, []

def check_fixed_point_operation(operation: str, operands: tuple[ResolvedType, ...], *,
                                numeric_policy: dict[str, Any], span: SourceSpan | None) -> tuple[ResolvedType | None, dict[str, Any] | None, list[LanguageDiagnostic]]:
    """Return deterministic fixed-point intermediate-width requirements."""
    if not operands or any(item.identity.kind != "fixed" for item in operands):
        return None, None, [source_error("fixed_point_operand_required", "Fixed-point arithmetic requires fixed-point operands.", span)]
    result = operands[0]
    if any(item.identity != result.identity or item.shape != result.shape or item.unit_dimensions != result.unit_dimensions
           or item.unit_scale != result.unit_scale or item.unit_offset != result.unit_offset
           for item in operands[1:]):
        return None, None, [source_error("fixed_point_operand_mismatch", "Fixed-point operands must use the same nominal type, shape, and exact unit mapping.", span)]
    if operation in {"*", "/"} and result.identity.owner in {"project", "system_contract"}:
        return None, None, [source_error("fixed_point_result_type_required", "Multiplication or division of a project-defined fixed-point type needs an explicitly declared result type.", span)]
    bits = max(item.storage_bits or 0 for item in operands)
    frac = max(item.fractional_bits or 0 for item in operands)
    intermediate = min(128, bits * (2 if operation in {"*", "/"} else 1))
    if intermediate > 64 and numeric_policy.get("allow_wide_intermediate") is not True:
        return None, None, [source_error("fixed_point_intermediate_unsupported", "The selected numeric policy does not authorize this wide fixed-point intermediate.", span)]
    return replace(result, storage_bits=bits, fractional_bits=frac), {"operation": operation, "intermediate_bits": intermediate,
            "fractional_bits": frac, "rounding": result.rounding or numeric_policy.get("rounding", "nearest_even"),
            "overflow": result.overflow or numeric_policy.get("overflow", "reject")}, []

def validate_type_boundaries(document: ProjectDocument, environment: TypeEnvironment) -> list[LanguageDiagnostic]:
    """Reject incomplete or contradictory explicit record boundary contracts."""
    errors: list[LanguageDiagnostic] = []
    for record in document.records:
        boundary = record.boundary
        if boundary is None: continue
        names = [field.name for field in record.fields]
        if boundary.version <= 0: errors.append(source_error("invalid_boundary_version", "Boundary record versions must be positive.", boundary.span or record.span))
        if boundary.kind == "shared":
            if boundary.alignment is not None and (boundary.alignment <= 0 or boundary.alignment & (boundary.alignment - 1)):
                errors.append(source_error("invalid_shared_alignment", "Shared-record alignment must be a positive power of two.", boundary.span or record.span))
            for name, _ in boundary.field_offsets:
                if name not in names: errors.append(source_error("unknown_boundary_field", f"Boundary metadata refers to unknown field {name!r}.", boundary.span or record.span))
        elif boundary.kind == "persistent":
            if not boundary.schema_id: errors.append(source_error("persistent_schema_id_required", "Persistent records require a stable schema_id.", boundary.span or record.span))
            if not boundary.integrity: errors.append(source_error("persistent_integrity_required", "Persistent records must declare an integrity policy.", boundary.span or record.span))
            if not boundary.migration_policy: errors.append(source_error("persistent_migration_required", "Persistent records must declare a migration policy.", boundary.span or record.span))
    return errors

def _resolved_shape(type_spec: TypeSpec, environment: TypeEnvironment, errors: list[LanguageDiagnostic]) -> tuple[int, ...]:
    shape: list[int] = []
    constants = {}
    for binding in getattr(environment, "project_parameters", ()):
        if binding.value is not None and isinstance(getattr(binding.value, "value", None), int): constants[binding.name] = binding.value.value
    for dim in type_spec.shape:
        value = dim if isinstance(dim, int) else constants.get(dim)
        if value is None or value <= 0:
            errors.append(source_error("unresolved_array_dimension", f"Array dimension {dim!r} is not a positive compile-time integer.", type_spec.span)); continue
        shape.append(value)
    return tuple(shape)


def _resolve_record_member(base: ResolvedType, members: tuple[str, ...], environment: TypeEnvironment) -> tuple[ResolvedType | None, list[LanguageDiagnostic]]:
    """Resolve a bounded dotted path through a declared record type."""
    diagnostics: list[LanguageDiagnostic] = []
    current = base
    for member in members:
        if current.identity.kind == "generic" and current.identity.canonical_name == "Received":
            if member == "value" and len(current.arguments) == 1:
                current = current.arguments[0]
                continue
            if member == "fresh":
                current = environment.language_types["bool"]
                continue
            if member == "sequence":
                current = environment.language_types["u32"]
                continue
            if member == "received_at_us":
                current, more = resolve_type_spec(TypeSpec("u64", unit="us"), environment)
                diagnostics.extend(more)
                if current is None:
                    return None, diagnostics
                continue
            # `Received<T>` metadata is intentionally limited to the fields
            # declared by its UEF contract. Do not guess member names/types.
            return None, diagnostics
        if current.identity.kind == "vector":
            dimension = current.shape[0] if current.shape else 0
            components = ("x", "y", "z", "w")[:dimension]
            if member in components:
                scalar_name = str(environment.numeric_policy.get("scalar", "f32"))
                scalar = environment.language_types.get(scalar_name)
                if scalar is None:
                    return None, diagnostics
                current = replace(
                    scalar,
                    unit=current.unit,
                    unit_dimensions=current.unit_dimensions,
                    affine=current.affine,
                    unit_scale=current.unit_scale,
                    unit_offset=current.unit_offset,
                    boundary=current.boundary,
                )
                continue
            return None, diagnostics
        definition = environment.project_definitions.get(current.identity.canonical_name) or environment.imported_definitions.get(current.identity.canonical_name)
        if current.identity.kind != "record" or definition is None:
            if current.identity.owner == "uetd" and current.identity.kind == "opaque":
                diagnostics.append(source_error(
                    "uetd_opaque_member_access",
                    f"UETD type {current.identity.source_name!r} is registered as opaque; its C fields are not part of the UEF-PL contract.",
                ))
            return None, diagnostics
        field_type = None
        for field in getattr(definition, "fields", ()):
            name, type_spec = _record_field_parts(field)
            if name == member:
                field_type = type_spec
                break
        if field_type is None:
            return None, diagnostics
        current, more = resolve_type_spec(field_type, environment)
        diagnostics.extend(more)
        if current is None:
            return None, diagnostics
    return current, diagnostics


def _record_field_parts(field: Any) -> tuple[str | None, TypeSpec | None]:
    """Normalize project-record and imported-system-record field entries."""
    if isinstance(field, tuple):
        if len(field) >= 2:
            return field[0], field[1]
        return None, None
    return getattr(field, "name", None), getattr(field, "type_spec", None)


def _constant_value(value: Any) -> Any:
    """Convert a simple language constant to its literal registry value."""
    if isinstance(value, ValueLiteral):
        return value.value
    if isinstance(value, Identifier):
        return value.value
    if isinstance(value, Reference) and len(value.path) == 1:
        return value.path[0]
    return None

def _unit_facts(unit: str | None, catalogue: dict[str, Any]) -> tuple[tuple[tuple[str, int], ...], bool, str | None]:
    dimensions, affine, _scale, _offset, error = _unit_mapping_facts(unit, catalogue)
    return dimensions, affine, error

def _unit_mapping_facts(unit: str | None, catalogue: dict[str, Any]) -> tuple[
    tuple[tuple[str, int], ...], bool, tuple[int, int, int], tuple[int, int], str | None
]:
    """Resolve exact rational-times-pi scale and offset with UETD dimensions."""
    if unit in (None, "", "1"): return (), False, (1, 1, 0), (0, 1), None
    dimensions: dict[str, int] = {}
    scale = Fraction(1)
    pi_power = 0
    offset = Fraction(0)
    pieces = re.findall(r"([*/]?)\s*([A-Za-z_][A-Za-z0-9_]*|1)(?:\^([+-]?\d+))?", unit)
    if not pieces or "".join((op + atom + (f"^{power}" if power else "")) for op, atom, power in pieces).replace("*", "") != unit.replace("*", ""):
        return (), False, (1, 1, 0), (0, 1), f"Unit expression {unit!r} is malformed."
    affine = False
    for op, atom, exponent_text in pieces:
        if atom == "1": continue
        data = catalogue.get(atom)
        if not isinstance(data, dict): return (), False, (1, 1, 0), (0, 1), f"Unit {atom!r} is not in UETD's unit registry."
        power = int(exponent_text or "1") * (-1 if op == "/" else 1)
        for dim, value in data.get("dimension", {}).items(): dimensions[dim] = dimensions.get(dim, 0) + value * power
        atom_scale = data.get("exact_scale")
        if isinstance(atom_scale, dict):
            atom_ratio = Fraction(int(atom_scale.get("numerator", 1)), int(atom_scale.get("denominator", 1)))
            atom_pi = int(atom_scale.get("pi_power", 0))
        else:
            try: atom_ratio = Fraction(str(data.get("scale_to_si", "1")))
            except (ValueError, ZeroDivisionError): return (), False, (1, 1, 0), (0, 1), f"UETD unit {atom!r} has an invalid scale."
            atom_pi = 0
        scale *= atom_ratio ** power
        pi_power += atom_pi * power
        is_affine = data.get("kind") == "affine" or bool(data.get("offset_to_si", data.get("offset")))
        if is_affine:
            if power != 1 or len(pieces) != 1:
                return (), False, (1, 1, 0), (0, 1), f"Affine unit {atom!r} must be used as a standalone first-power unit."
            affine = True
            try: offset = Fraction(str(data.get("offset_to_si", data.get("offset", "0"))))
            except (ValueError, ZeroDivisionError): return (), False, (1, 1, 0), (0, 1), f"UETD unit {atom!r} has an invalid affine offset."
    dims = tuple(sorted((dim, value) for dim, value in dimensions.items() if value))
    return dims, affine, (scale.numerator, scale.denominator, pi_power), (offset.numerator, offset.denominator), None

def _combine_unit_scales(left: tuple[int, int, int], right: tuple[int, int, int], sign: int) -> tuple[int, int, int]:
    left_ratio = Fraction(left[0], left[1])
    right_ratio = Fraction(right[0], right[1]) ** sign
    value = left_ratio * right_ratio
    return value.numerator, value.denominator, left[2] + sign * right[2]

def _power_unit_scale(value: tuple[int, int, int], exponent: int) -> tuple[int, int, int]:
    ratio = Fraction(value[0], value[1]) ** exponent
    return ratio.numerator, ratio.denominator, value[2] * exponent

def _combine_dimensions(left: tuple[tuple[str, int], ...], right: tuple[tuple[str, int], ...], sign: int) -> tuple[tuple[str, int], ...]:
    result = dict(left)
    for dim, exponent in right: result[dim] = result.get(dim, 0) + sign * exponent
    return tuple(sorted((dim, exponent) for dim, exponent in result.items() if exponent))

def _unit_display(dimensions: tuple[tuple[str, int], ...]) -> str:
    if not dimensions: return "1"
    return "*".join(dim if power == 1 else f"{dim}^{power}" for dim, power in dimensions)


def _unit_expression_atoms(expression: str | None) -> tuple[list[str], dict[str, int]]:
    """Parse a UETD unit expression into stable atom powers."""
    text = expression or "1"
    pieces = re.findall(r"([*/]?)\s*([A-Za-z_][A-Za-z0-9_]*|1)(?:\^([+-]?\d+))?", text)
    if not pieces or "".join((op + atom + (f"^{power}" if power else "")) for op, atom, power in pieces).replace("*", "") != text.replace("*", ""):
        return [], {}
    order: list[str] = []
    powers: dict[str, int] = {}
    for operator, atom, exponent_text in pieces:
        if atom == "1":
            continue
        if atom not in powers:
            order.append(atom)
            powers[atom] = 0
        exponent = int(exponent_text or "1") * (-1 if operator == "/" else 1)
        powers[atom] += exponent
    return order, powers


def _render_unit_expression(order: list[str], powers: dict[str, int]) -> str:
    """Render a normalized product/quotient that remains valid in the DSL."""
    numerator: list[str] = []
    denominator: list[str] = []
    for atom in order:
        exponent = powers.get(atom, 0)
        if exponent == 0:
            continue
        destination = numerator if exponent > 0 else denominator
        magnitude = abs(exponent)
        destination.append(atom if magnitude == 1 else f"{atom}^{magnitude}")
    top = "*".join(numerator) if numerator else "1"
    if not denominator:
        return top
    return top + "/" + "*".join(denominator)


def _combine_unit_expression(left: str | None, right: str | None, sign: int) -> str:
    """Compose two unit expressions without replacing symbols by dimension names."""
    left_order, powers = _unit_expression_atoms(left)
    right_order, right_powers = _unit_expression_atoms(right)
    if not left_order and left not in (None, "", "1") or not right_order and right not in (None, "", "1"):
        return _unit_display(())
    for atom in right_order:
        if atom not in powers:
            left_order.append(atom)
            powers[atom] = 0
        powers[atom] += sign * right_powers[atom]
    return _render_unit_expression(left_order, powers)


def _power_unit_expression(unit: str | None, exponent: int) -> str:
    """Raise every registered unit atom to a bounded integer power."""
    order, powers = _unit_expression_atoms(unit)
    if not order and unit not in (None, "", "1"):
        return _unit_display(())
    return _render_unit_expression(order, {atom: power * exponent for atom, power in powers.items()})

def derive_time_rate_type(value_type: ResolvedType, environment: TypeEnvironment) -> ResolvedType | None:
    """Return the type of a state derivative, whose unit is state-unit/second.

    ``derivative position = velocity`` is a model equation, not an ordinary
    assignment. Preserve the target's scalar representation and shape while
    dividing its physical dimensions and exact unit scale by seconds.
    """
    if value_type.identity.kind not in {"integer", "float", "fixed", "vector"}:
        return None
    time_dimensions, _time_affine, time_scale, _time_offset, error = _unit_mapping_facts("s", environment.unit_catalogue)
    if error:
        return None
    dimensions = _combine_dimensions(value_type.unit_dimensions, time_dimensions, -1)
    scale = _combine_unit_scales(value_type.unit_scale, time_scale, -1)
    return replace(
        value_type,
        unit=_combine_unit_expression(value_type.unit, "s", -1),
        unit_dimensions=dimensions,
        affine=False,
        unit_scale=scale,
        unit_offset=(0, 1),
    )

def _expression_cost(expression: Any) -> int:
    if isinstance(expression, BinaryExpression): return 1 + _expression_cost(expression.left) + _expression_cost(expression.right)
    if isinstance(expression, UnaryExpression): return 1 + _expression_cost(expression.operand)
    if isinstance(expression, MathCallExpression): return 1 + sum(_expression_cost(value) for value in expression.arguments)
    if isinstance(expression, ListValue): return sum(_expression_cost(value) for value in expression.items)
    if isinstance(expression, MatrixLiteral): return sum(_expression_cost(value) for row in expression.rows for value in row)
    if isinstance(expression, SelectValue): return 1 + _expression_cost(expression.condition) + max(_expression_cost(expression.if_true), _expression_cost(expression.if_false))
    return 1

def _find_property(document: ProjectDocument, name: str) -> Any:
    for declaration in document.declarations:
        for prop in declaration.properties:
            if prop.name == name or (declaration.kind == "target" and prop.name == "value" and name == "target"): return prop.value
    return None

def _find_nested_property(document: ProjectDocument, declaration_kind: str, name: str) -> Any:
    for declaration in document.declarations:
        if declaration.kind == declaration_kind:
            for prop in declaration.properties:
                if prop.name == name: return prop.value
    return None

def _value_name(value: Any) -> str | None:
    if isinstance(value, Reference) and len(value.path) == 1: return value.path[0]
    if isinstance(value, Identifier): return value.value
    if isinstance(value, ValueLiteral) and isinstance(value.value, str): return value.value
    return None
