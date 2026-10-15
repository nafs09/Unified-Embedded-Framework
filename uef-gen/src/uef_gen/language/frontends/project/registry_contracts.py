"""UETD registry access for standalone project compilation and validation."""

from __future__ import annotations
import json
import re
from dataclasses import dataclass
from jsonschema import Draft202012Validator
from pathlib import Path
from typing import Any
from uef_gen.uetd_adapter.registry import UETDRegistryError, load_module_manifest, load_registry_document

@dataclass(frozen=True)
class UETDModuleContract:
    """Validated settings/signals/capabilities for one UETD-owned module."""

    registry_id: str
    kind: str
    display_name: str
    properties: tuple[dict[str, Any], ...]
    inputs: tuple[dict[str, Any], ...]
    outputs: tuple[dict[str, Any], ...]
    numerics: dict[str, Any]
    scheduling: dict[str, Any]
    capabilities: dict[str, Any]
    operations: tuple[dict[str, Any], ...]
    generation: dict[str, Any]
    source_path: str
    dsl_descriptor: str | None = None
    protocol_endpoint_binding: dict[str, Any] | None = None
    description: str = ""

@dataclass(frozen=True)
class DSLDescriptorBinding:
    """Main-registry DSL spelling and its UETD implementation, if authorable."""

    descriptor: str
    module_id: str
    contract: UETDModuleContract | None
    source_path: str

@dataclass(frozen=True)
class UETDTypeDefinition:
    """Public C type declaration and its optional UEF-PL representation contract."""

    c_name: str
    dsl_name: str | None
    kind: str
    header: str
    declaration: str
    enum_members: tuple[str, ...]
    language_visibility: str
    source_status: str
    language_contract: str | None = None
    vector_dimension: int | None = None

@dataclass(frozen=True)
class UETDMathFunctionContract:
    """UEF-PL callable name plus its hidden UMATH C binding and readiness."""

    dsl_name: str
    dsl_aliases: tuple[str, ...]
    dsl_signature: str | None
    operators: tuple[str, ...]
    c_name: str
    module_id: str | None
    header: str
    c_signature: str
    parameters: str
    dsl_binding_contract: dict[str, Any] | None
    declaration_kind: str
    implementation_status: str
    dsl_binding_status: str
    available_for_generation: bool
    callable_contexts: tuple[str, ...]

@dataclass(frozen=True)
class AuthoringCatalogue:
    """Project compilation view indexed by IDs from UETD's registries.

    `modules` is indexed by UETD's canonical module IDs. Role-specific maps are
    convenience views over those same records, not separate catalogues or
    parser allowlists. The source language has no generic `component` form:
    its identifiers come from the main registry's `DSL_descriptor` fields.
    """

    modules: dict[str, UETDModuleContract]
    algorithms: dict[str, UETDModuleContract]
    protocols: dict[str, UETDModuleContract]
    sensors: dict[str, UETDModuleContract]
    actuators: dict[str, UETDModuleContract]
    peripherals: dict[str, UETDModuleContract]
    hybrid_strategies: dict[str, UETDModuleContract]
    math_functions: dict[str, tuple[UETDMathFunctionContract, ...]]
    math_operators: dict[str, tuple[UETDMathFunctionContract, ...]]
    math_functions_by_c_name: dict[str, UETDMathFunctionContract]
    types: dict[str, tuple[UETDTypeDefinition, ...]]
    units: dict[str, dict[str, Any]]
    unit_dimensions: tuple[str, ...]
    runtimes: dict[str, UETDModuleContract]
    aliases: dict[str, str | list[str]]
    dsl_descriptors: dict[str, DSLDescriptorBinding]

def load_authoring_catalogue(uetd_root: Path) -> AuthoringCatalogue:
    """Load all source-language contracts through UETD registry indexes only.

    Discover every spelling from UETD's main module index. Missing authoring
    contracts remain visible as descriptor bindings with ``contract=None``;
    callers can then issue a readiness warning without incorrectly reporting
    an unknown identifier. Authoring details are validated against the schema
    referenced by the same UETD index. Registry identifiers are never inferred
    from filenames or copied into a uef-gen catalogue.
    """
    root = uetd_root.expanduser().resolve()
    api = load_registry_document(root, "api.json")
    module_registry_path = api.get("module_registry")
    if not isinstance(module_registry_path, str) or not module_registry_path.strip():
        raise UETDRegistryError("UETD API manifest does not declare module_registry")
    index = load_registry_document(root, module_registry_path)
    schema_path = index.get("registry_schema", "registry/modules/registry.schema.json")
    authoring_schema_path = index.get("authoring_contract_schema", "registry/modules/authoring_contract.schema.json")
    registry_schema = _read_schema(root, schema_path)
    authoring_schema = _read_schema(root, authoring_schema_path)
    _validate_schema(index, registry_schema, "UETD module registry")
    # The loader expands only the manifest references from this UETD-owned index,
    # checking path containment and duplicate DSL descriptors along the way.
    expanded = load_module_manifest(root)
    entries = index.get("modules")
    if not isinstance(entries, dict):
        raise UETDRegistryError("UETD module registry 'modules' must be an object")
    modules: dict[str, UETDModuleContract] = {}
    roles: dict[str, dict[str, UETDModuleContract]] = {
        "algorithm": {},
        "protocol": {},
        "sensor": {},
        "actuator": {},
        "peripheral": {},
        "hybrid_strategy": {},
        "runtime": {},
    }
    descriptors: dict[str, DSLDescriptorBinding] = {}
    for module_id, source_entry in entries.items():
        if not isinstance(source_entry, dict):
            raise UETDRegistryError(f"UETD module entry {module_id!r} must be an object")
        descriptor = source_entry.get("DSL_descriptor")
        manifest_reference = source_entry.get("manifest")
        module = expanded.get("modules", {}).get(module_id)
        if not isinstance(descriptor, str) or not isinstance(module, dict):
            raise UETDRegistryError(f"UETD module {module_id!r} has an invalid descriptor or manifest")
        if not isinstance(manifest_reference, str):
            raise UETDRegistryError(f"UETD module {module_id!r} has no manifest path")
        authoring = module.get("authoring")
        contract: UETDModuleContract | None = None
        if authoring is not None:
            if not isinstance(authoring, dict):
                raise UETDRegistryError(f"UETD module {module_id!r} authoring contract must be an object")
            _validate_schema(authoring, authoring_schema, f"UETD authoring contract for {module_id}")
            kind = authoring.get("kind")
            if not isinstance(kind, str):
                raise UETDRegistryError(f"UETD module {module_id!r} contract has no kind")
            contract = UETDModuleContract(
                registry_id=module_id,
                kind=kind,
                display_name=str(authoring.get("display_name", module_id)),
                properties=tuple(_normalize_parameter(item) for item in _mapping_list(authoring.get("parameters"))),
                inputs=tuple(
                    _normalize_port(item)
                    for item in _mapping_list(authoring.get("ports"))
                    if item.get("direction") == "input"
                ),
                outputs=tuple(
                    _normalize_port(item)
                    for item in _mapping_list(authoring.get("ports"))
                    if item.get("direction") == "output"
                ),
                numerics=_mapping(authoring.get("numerics")),
                scheduling=_mapping(authoring.get("scheduling")),
                capabilities=_module_capabilities(authoring),
                operations=tuple(_mapping_list(authoring.get("operations"))),
                generation=_mapping(authoring.get("generation")),
                source_path=manifest_reference,
                dsl_descriptor=descriptor,
                protocol_endpoint_binding=(
                    _mapping(authoring.get("protocol_endpoint_binding")) if authoring.get("protocol_endpoint_binding") is not None else None
                ),
                description=str(authoring.get("description", "")),
            )
            modules[module_id] = contract
            if kind in roles:
                roles[kind][descriptor] = contract
        descriptors[descriptor] = DSLDescriptorBinding(descriptor=descriptor, module_id=module_id, contract=contract, source_path=manifest_reference)
    type_catalogue = _load_source_catalogue(root, api.get("type_registry"), api.get("type_registry_schema"), "UETD type registry")
    umath_api = api.get("umath_api")
    if not isinstance(umath_api, dict):
        raise UETDRegistryError("UETD API manifest does not declare umath_api")
    function_catalogue = _load_source_catalogue(
        root, umath_api.get("function_registry"), umath_api.get("function_registry_schema"), "UETD UMATH function registry"
    )
    unit_catalogue = _load_source_catalogue(root, api.get("unit_registry"), api.get("unit_registry_schema"), "UETD unit registry")
    type_definitions: dict[str, list[UETDTypeDefinition]] = {}
    for item in _mapping_list(type_catalogue.get("types")):
        if not all(isinstance(item.get(key), str) for key in ("c_name", "kind", "header", "declaration")):
            raise UETDRegistryError("UETD type registry contains an incomplete type entry")
        definition = UETDTypeDefinition(
            c_name=item["c_name"],
            dsl_name=item.get("dsl_name") if isinstance(item.get("dsl_name"), str) else None,
            kind=item["kind"],
            header=item["header"],
            declaration=item["declaration"],
            enum_members=tuple(value for value in item.get("enum_members", []) if isinstance(value, str)),
            language_visibility=str(item.get("language_visibility", "UETD contract type")),
            source_status=str(item.get("source_status", "public_header")),
            language_contract=item.get("language_contract") if isinstance(item.get("language_contract"), str) else None,
            vector_dimension=item.get("vector_dimension") if isinstance(item.get("vector_dimension"), int) and not isinstance(item.get("vector_dimension"), bool) else None,
        )
        type_definitions.setdefault(definition.c_name, []).append(definition)
    math_function_entries: list[UETDMathFunctionContract] = []
    math_functions_by_c_name: dict[str, UETDMathFunctionContract] = {}
    for item in _mapping_list(function_catalogue.get("functions")):
        if not all(
            isinstance(item.get(key), str)
            for key in ("dsl_name", "c_name", "header", "c_signature", "parameters", "implementation_status", "dsl_binding_status")
        ):
            raise UETDRegistryError("UETD UMATH registry contains an incomplete function entry")
        contract = UETDMathFunctionContract(
            dsl_name=item["dsl_name"],
            dsl_aliases=tuple(dict.fromkeys(
                value for value in item.get("dsl_aliases", [])
                if isinstance(value, str) and value != item["dsl_name"]
            )),
            dsl_signature=item.get("dsl_signature") if isinstance(item.get("dsl_signature"), str) else None,
            operators=tuple(dict.fromkeys(
                value for value in (item.get("dsl_binding_contract") or {}).get("operators", [])
                if isinstance(value, str)
            )),
            c_name=item["c_name"],
            module_id=item.get("module_id") if isinstance(item.get("module_id"), str) else None,
            header=item["header"],
            c_signature=item["c_signature"],
            parameters=item["parameters"],
            dsl_binding_contract=(item.get("dsl_binding_contract") if isinstance(item.get("dsl_binding_contract"), dict) else None),
            declaration_kind=str(item.get("declaration_kind", "prototype")),
            implementation_status=item["implementation_status"],
            dsl_binding_status=item["dsl_binding_status"],
            available_for_generation=item.get("available_for_generation") is True,
            callable_contexts=tuple(value for value in item.get("callable_contexts", []) if isinstance(value, str)),
        )
        previous_c = math_functions_by_c_name.get(contract.c_name)
        if previous_c is not None and previous_c.dsl_name != contract.dsl_name:
            raise UETDRegistryError(f"UMATH C function {contract.c_name!r} has conflicting DSL names")
        if previous_c is not None:
            raise UETDRegistryError(f"UMATH C function {contract.c_name!r} is registered more than once")
        math_function_entries.append(contract)
        math_functions_by_c_name[contract.c_name] = contract
    math_function_names: dict[str, list[UETDMathFunctionContract]] = {}
    for contract in math_function_entries:
        for name in (contract.dsl_name, *contract.dsl_aliases):
            math_function_names.setdefault(name, []).append(contract)
    math_functions = {
        name: tuple(sorted(entries, key=lambda item: (item.c_name, item.header)))
        for name, entries in math_function_names.items()
    }
    math_operator_names: dict[str, list[UETDMathFunctionContract]] = {}
    for contract in math_function_entries:
        for operator in contract.operators:
            math_operator_names.setdefault(operator, []).append(contract)
    math_operators = {
        operator: tuple(sorted(entries, key=lambda item: (item.c_name, item.header)))
        for operator, entries in math_operator_names.items()
    }
    return AuthoringCatalogue(
        modules=modules,
        algorithms=roles["algorithm"],
        protocols=roles["protocol"],
        sensors=roles["sensor"],
        actuators=roles["actuator"],
        peripherals=roles["peripheral"],
        hybrid_strategies=roles["hybrid_strategy"],
        math_functions=math_functions,
        math_operators=math_operators,
        math_functions_by_c_name=math_functions_by_c_name,
        types={name: tuple(items) for name, items in type_definitions.items()},
        units=_mapping(unit_catalogue.get("units")),
        unit_dimensions=tuple(value for value in unit_catalogue.get("base_dimensions", []) if isinstance(value, str)),
        runtimes=roles["runtime"],
        aliases={},
        dsl_descriptors=descriptors,
    )

def resolve_dsl_descriptor(catalogue: AuthoringCatalogue, descriptor: str) -> DSLDescriptorBinding | None:
    """Look up a registered declaration identifier from UETD's existing main module index.

    Return None only for an unknown name. A known binding with `contract=None`
    is not an unknown registered name: validation must report
    `uetd_descriptor_contract_missing` against its UETD manifest. Never guess an
    ID from the spelling.
    """
    return catalogue.dsl_descriptors.get(descriptor)

def resolve_module(catalogue: AuthoringCatalogue, registry_id: str, *, role: str) -> UETDModuleContract:
    """Resolve one UETD module ID for a specific language role.

    Resolve through the descriptor recorded in the UETD module index. A missing
    contract is left to semantic validation so the source can receive a
    registered-but-unimplemented warning with its UETD manifest path.
    """
    binding = catalogue.dsl_descriptors.get(registry_id)
    if binding is None:
        # A canonical module ID is accepted only when it is an exact UETD ID.
        binding = next((item for item in catalogue.dsl_descriptors.values() if item.module_id == registry_id), None)
    if binding is None:
        raise KeyError(f"UETD module or DSL descriptor is not registered: {registry_id}")
    if binding.contract is None:
        raise ValueError(f"UETD module {binding.module_id!r} has no authoring contract (manifest: {binding.source_path})")
    if role and role != "module" and binding.contract.kind != role:
        raise ValueError(f"UETD descriptor {binding.descriptor!r} has kind {binding.contract.kind!r}, not requested role {role!r}")
    return binding.contract

def resolve_math_function(catalogue: AuthoringCatalogue, function_name: str, *, argument_types: tuple[Any, ...], context: str,
                          require_ready: bool = True, numeric_scalar_name: str | None = None,
                          shape_bindings: dict[str, int] | None = None) -> UETDMathFunctionContract:
    """Resolve a typed formula call through UETD's UMATH/function registry.

    Resolve only through the UETD header-derived UMATH index's DSL names.
    Never resolve a user-written C symbol here. Calls are legal in custom
    algorithm and model equation bodies, never in runtime guards, module
    settings, or protocol declarations. Argument type checking consumes the
    reviewed DSL signature/binding contract in the shared semantic front end.
    """
    contracts = catalogue.math_functions.get(function_name, ())
    if not contracts:
        if function_name in catalogue.math_functions_by_c_name:
            dsl_name = catalogue.math_functions_by_c_name[function_name].dsl_name
            raise KeyError(f"UETD C symbol {function_name!r} is hidden in UEF-PL; use DSL name {dsl_name!r}")
        raise KeyError(f"UEF-PL math function is not registered: {function_name}")
    return resolve_math_overload(function_name, contracts, argument_types=argument_types, context=context,
                                 require_ready=require_ready, numeric_scalar_name=numeric_scalar_name,
                                 shape_bindings=shape_bindings)

def resolve_math_overload(function_name: str, contracts: tuple[UETDMathFunctionContract, ...], *,
                          argument_types: tuple[Any, ...], context: str, require_ready: bool = True,
                          shape_bindings: dict[str, int] | None = None,
                          numeric_scalar_name: str | None = None) -> UETDMathFunctionContract:
    """Select one registered overload by its typed logical parameters."""
    if not isinstance(argument_types, tuple):
        raise TypeError("UMATH argument types must be supplied as a resolved tuple")
    normalized_context = {
        "model_equations": "model_equation",
    }.get(context, context)
    allowed = [item for item in contracts if normalized_context in item.callable_contexts]
    if not allowed:
        raise ValueError(f"UETD math function {function_name!r} is not permitted in {context!r}")
    typed = [item for item in allowed if math_contract_parameter_types(item) is not None]
    untyped = [item for item in allowed if math_contract_parameter_types(item) is None]
    matches: list[tuple[UETDMathFunctionContract, dict[str, int]]] = []
    for item in typed:
        dimensions = _math_overload_shape_matches(item, argument_types, numeric_scalar_name=numeric_scalar_name)
        if dimensions is not None:
            matches.append((item, dimensions))
    # A candidate with an unsupported C ABI can only be ignored when its
    # declared logical arity differs from the call. Otherwise the registry
    # needs an authored type contract before overload selection is deterministic.
    potentially_matching_untyped = [item for item in untyped
                                    if _c_signature_logical_arity(item) in {None, len(argument_types)}]
    if potentially_matching_untyped:
        signatures = "; ".join(item.dsl_signature or item.c_signature for item in potentially_matching_untyped)
        raise ValueError(f"UEF-PL overload set {function_name!r} has candidates without usable typed DSL metadata ({signatures}); no candidate can be selected safely")
    if not matches:
        signatures = "; ".join(item.dsl_signature or item.c_signature for item in allowed)
        raise TypeError(f"No overload of {function_name!r} accepts ({', '.join(_display_argument_type(item) for item in argument_types)}); candidates: {signatures}")
    if len(matches) != 1:
        signatures = "; ".join(item.dsl_signature or item.c_signature for item, _ in matches)
        raise TypeError(f"Call to {function_name!r} is ambiguous for the supplied argument types; matching overloads: {signatures}")
    contract, dimensions = matches[0]
    if shape_bindings is not None:
        shape_bindings.update(dimensions)
    if require_ready and contract.dsl_binding_status != "ready":
        raise ValueError(f"UETD math overload {contract.dsl_signature or contract.dsl_name!r} has {contract.dsl_binding_status!r} DSL binding metadata")
    if require_ready and not contract.available_for_generation:
        raise ValueError(f"UETD math overload {contract.dsl_signature or contract.dsl_name!r} is {contract.implementation_status!r} and unavailable for generation")
    if require_ready and (not contract.dsl_signature or contract.dsl_binding_contract is None):
        raise ValueError(f"UEF-PL math overload {function_name!r} has no reviewed DSL binding contract")
    return contract

def math_contract_parameter_types(contract: UETDMathFunctionContract) -> tuple[Any, ...] | None:
    """Return DSL parameter types, or safe type-checking-only data from C.

    C-signature inference never marks a binding ready for generation. It only
    prevents declarations with simple by-value arguments from looking like
    unknown overloads while their authoring contract is still unfinished.
    """
    binding = contract.dsl_binding_contract
    if isinstance(binding, dict) and isinstance(binding.get("parameters"), list):
        result: list[Any] = []
        for parameter in binding["parameters"]:
            if not isinstance(parameter, dict) or not isinstance(parameter.get("type"), (str, dict)):
                return None
            type_descriptor = parameter["type"]
            expected = dict(type_descriptor) if isinstance(type_descriptor, dict) else {"name": type_descriptor}
            for field_name in ("shape", "unit", "unit_from_parameter"):
                if field_name in parameter:
                    expected[field_name] = parameter[field_name]
            result.append(expected)
        return tuple(result)
    return _c_signature_parameter_types(contract)

def math_contract_result_type(contract: UETDMathFunctionContract, argument_types: tuple[Any, ...] = ()) -> Any | None:
    """Return a logical result descriptor from a DSL contract or simple C ABI."""
    binding = contract.dsl_binding_contract
    result = binding.get("result") if isinstance(binding, dict) else None
    if isinstance(result, dict):
        type_descriptor = result.get("type")
        if isinstance(type_descriptor, str):
            type_descriptor = {"name": type_descriptor}
        elif isinstance(type_descriptor, dict):
            type_descriptor = dict(type_descriptor)
        else:
            return None
        for field_name in ("shape", "unit"):
            if field_name in result:
                type_descriptor[field_name] = result[field_name]
        for field_name in ("unit_product_parameters", "unit_sqrt_parameter"):
            if field_name in result:
                type_descriptor[field_name] = result[field_name]
        unit_from = result.get("unit_from_parameter", type_descriptor.get("unit_from_parameter"))
        if isinstance(unit_from, int) and 0 <= unit_from < len(argument_types):
            unit = getattr(argument_types[unit_from], "unit", None)
            if unit is not None:
                type_descriptor["unit"] = unit
        return type_descriptor
    return _c_signature_result_type(contract)

def _math_overload_shape_matches(contract: UETDMathFunctionContract, actual_types: tuple[Any, ...], *,
                                 numeric_scalar_name: str | None) -> dict[str, int] | None:
    expected_types = math_contract_parameter_types(contract)
    if expected_types is None or len(expected_types) != len(actual_types):
        return None
    shape_bindings: dict[str, int] = {}
    if not all(_math_type_matches(actual, expected, shape_bindings, numeric_scalar_name=numeric_scalar_name)
               for actual, expected in zip(actual_types, expected_types)):
        return None
    for index, expected in enumerate(expected_types):
        unit_from = expected.get("unit_from_parameter") if isinstance(expected, dict) else None
        if unit_from is None:
            continue
        if not isinstance(unit_from, int) or not 0 <= unit_from < len(actual_types):
            return None
        actual, reference = actual_types[index], actual_types[unit_from]
        if (getattr(actual, "unit_dimensions", None) != getattr(reference, "unit_dimensions", None)
                or getattr(actual, "affine", None) != getattr(reference, "affine", None)
                or getattr(actual, "unit_scale", None) != getattr(reference, "unit_scale", None)
                or getattr(actual, "unit_offset", None) != getattr(reference, "unit_offset", None)):
            return None
    return shape_bindings


def _c_signature_parts(contract: UETDMathFunctionContract) -> tuple[str, list[str]] | None:
    """Split a simple public function declaration into return and parameters."""
    signature = contract.c_signature.strip().removesuffix(";").strip()
    if "{" in signature:
        signature = signature.split("{", 1)[0].strip()
    match = re.match(
        r"^(?:(?:static|inline|extern)\s+)*(?P<ret>[^()]+?)\s+" + re.escape(contract.c_name) + r"\s*\((?P<args>.*)\)$",
        signature,
    )
    if not match:
        return None
    arguments = match.group("args").strip()
    if not arguments or arguments == "void":
        return match.group("ret").strip(), []
    return match.group("ret").strip(), [item.strip() for item in arguments.split(",")]


def _c_type_descriptor(declaration: str) -> dict[str, Any] | None:
    """Map only scalar and by-value UETD types into safe DSL descriptors."""
    value = re.sub(r"\b(const|volatile|restrict)\b", "", declaration).strip()
    match = re.match(r"(?P<type>[A-Za-z_]\w*)\s*(?P<stars>\*+)?\s+(?P<name>[A-Za-z_]\w*)(?:\[(?P<dimension>\d+)\])?\s*$", value)
    if not match or match.group("stars"):
        return None
    c_type = match.group("type")
    if c_type == "umath_scalar_t":
        descriptor: dict[str, Any] = {"name": "numeric_scalar"}
        if match.group("dimension"):
            descriptor["shape"] = [int(match.group("dimension"))]
        return descriptor
    if c_type == "bool":
        return {"name": "bool"}
    if c_type.startswith(("umath_", "uetd_")) and c_type.endswith("_t"):
        return {"c_name": c_type}
    return None


def _c_signature_parameter_types(contract: UETDMathFunctionContract) -> tuple[Any, ...] | None:
    """Infer types for a simple value-only signature, never for pointer buffers."""
    parts = _c_signature_parts(contract)
    if parts is None:
        return None
    return_type, declarations = parts
    if return_type == "umath_status_t":
        if not declarations or "*" not in declarations[-1]:
            return None
        declarations = declarations[:-1]
    expected = [_c_type_descriptor(item) for item in declarations]
    return tuple(expected) if all(item is not None for item in expected) else None


def _c_signature_logical_arity(contract: UETDMathFunctionContract) -> int | None:
    """Return a C signature's value-argument count when parseable."""
    parts = _c_signature_parts(contract)
    if parts is None:
        return None
    return_type, declarations = parts
    if return_type == "umath_status_t" and declarations and "*" in declarations[-1]:
        declarations = declarations[:-1]
    return len(declarations)


def _c_signature_result_type(contract: UETDMathFunctionContract) -> Any | None:
    """Derive the result type of a direct-value or final output-pointer API."""
    parts = _c_signature_parts(contract)
    if parts is None:
        return None
    return_type, declarations = parts
    if return_type == "umath_status_t":
        if not declarations or "*" not in declarations[-1]:
            return None
        declaration = re.sub(r"\*+", "", declarations[-1]).strip()
        declaration = re.sub(r"\s+[A-Za-z_]\w*\s*$", " result", declaration)
        return _c_type_descriptor(declaration)
    return _c_type_descriptor(f"{return_type} result")

def _math_type_matches(actual: Any, expected: Any, shape_bindings: dict[str, int], *,
                       numeric_scalar_name: str | None) -> bool:
    identity = getattr(actual, "identity", None)
    if identity is None:
        return False
    kind = getattr(identity, "kind", None)
    category_kinds = {
        "real": {"integer", "float", "fixed"},
        "numeric": {"integer", "float", "fixed", "vector"},
        "vector": {"vector"},
        "integer": {"integer"},
        "float": {"float"},
        "fixed": {"fixed"},
        "boolean": {"boolean"},
        "enum": {"enum"},
        "record": {"record"},
        "variant": {"variant"},
        "flags": {"flags"},
        "opaque": {"opaque"},
    }
    expected_shape = None
    expected_unit = None
    expected_arguments = None
    expected_kind = None
    expected_c_kind = None
    expected_signed = None
    expected_storage_bits = None
    if isinstance(expected, str):
        names = {getattr(identity, "canonical_name", None), getattr(identity, "source_name", None), getattr(identity, "c_name", None)}
        if expected in {"scalar", "numeric_scalar", "array", "tensor", "matrix"} or expected in category_kinds:
            if not _math_category_matches(expected, actual, kind, category_kinds, numeric_scalar_name):
                return False
        elif expected not in names:
            return False
    elif isinstance(expected, dict):
        name = expected.get("name", expected.get("c_name", expected.get("type")))
        names = {getattr(identity, "canonical_name", None), getattr(identity, "source_name", None), getattr(identity, "c_name", None)}
        if isinstance(name, str):
            if name in {"scalar", "numeric_scalar", "array", "tensor", "matrix"} or name in category_kinds:
                if not _math_category_matches(name, actual, kind, category_kinds, numeric_scalar_name):
                    return False
            elif name not in names:
                return False
        expected_kind = expected.get("kind")
        if isinstance(expected_kind, str):
            if expected_kind in {"scalar", "numeric_scalar", "array", "tensor", "matrix"} or expected_kind in category_kinds:
                if not _math_category_matches(expected_kind, actual, kind, category_kinds, numeric_scalar_name):
                    return False
            elif expected_kind not in {kind, getattr(identity, "c_kind", None)}:
                return False
        expected_c_kind = expected.get("c_kind")
        if isinstance(expected_c_kind, str) and expected_c_kind != getattr(identity, "c_kind", None):
            return False
        expected_shape = expected.get("shape")
        expected_unit = expected.get("unit")
        expected_arguments = expected.get("arguments")
        expected_signed = expected.get("signed")
        expected_storage_bits = expected.get("storage_bits")
    else:
        return False
    if expected_signed is not None and getattr(actual, "signed", None) is not expected_signed:
        return False
    if expected_storage_bits is not None and getattr(actual, "storage_bits", None) != expected_storage_bits:
        return False
    if expected_shape is not None:
        actual_shape = tuple(getattr(actual, "shape", ()))
        if not isinstance(expected_shape, (list, tuple)):
            return False
        if len(actual_shape) != len(expected_shape):
            return False
        for expected_dimension, actual_dimension in zip(expected_shape, actual_shape):
            if isinstance(expected_dimension, int):
                if actual_dimension != expected_dimension:
                    return False
            elif isinstance(expected_dimension, str) and expected_dimension != "*":
                previous = shape_bindings.setdefault(expected_dimension, actual_dimension)
                if previous != actual_dimension:
                    return False
            elif expected_dimension != "*":
                return False
    if expected_arguments is not None:
        actual_arguments = tuple(getattr(actual, "arguments", ()))
        if not isinstance(expected_arguments, (list, tuple)) or len(actual_arguments) != len(expected_arguments):
            return False
        if not all(_math_type_matches(actual_argument, expected_argument, shape_bindings,
                                      numeric_scalar_name=numeric_scalar_name)
                   for actual_argument, expected_argument in zip(actual_arguments, expected_arguments)):
            return False
    return True

def _math_category_matches(category: str, actual: Any, kind: Any,
                           category_kinds: dict[str, set[str]], numeric_scalar_name: str | None) -> bool:
    """Match a registry category against scalar, vector, matrix, or array values."""
    shape = tuple(getattr(actual, "shape", ()))
    numeric_kinds = {"integer", "float", "fixed", "vector"}
    if category in {"scalar", "numeric_scalar"}:
        if shape:
            return False
        if category == "numeric_scalar":
            identity = actual.identity
            names = {getattr(identity, "canonical_name", None), getattr(identity, "source_name", None), getattr(identity, "c_name", None)}
            return numeric_scalar_name in names
        return kind in {"integer", "float", "fixed"}
    if category in {"array", "tensor"}:
        return bool(shape)
    if category == "matrix":
        return len(shape) == 2 and kind in numeric_kinds
    if category == "vector":
        return kind == "vector" or (len(shape) == 1 and kind in {"integer", "float", "fixed"})
    if category == "numeric":
        return kind in numeric_kinds or (bool(shape) and kind in {"integer", "float", "fixed"})
    if category == "real":
        return not shape and kind in category_kinds[category]
    accepted = category_kinds.get(category)
    return accepted is not None and kind in accepted and not shape

def _display_argument_type(value: Any) -> str:
    identity = getattr(value, "identity", None)
    if identity is None:
        return str(value)
    return str(getattr(identity, "source_name", None) or getattr(identity, "canonical_name", "?"))

def validate_module_contract(document: dict[str, Any], *, manifest_path: str, schema: dict[str, Any] | None = None) -> list[str]:
    """Return schema/consistency issues for one UETD project-facing module contract.

    When the UETD authoring-contract schema is supplied, apply the canonical
    schema. Otherwise perform structural checks useful to callers that have
    only loaded one manifest. Readiness and implementation checks remain
    separate so incomplete entries can still be discovered and warned about.
    """
    issues: list[str] = []
    if not isinstance(document, dict):
        return [f"{manifest_path}: authoring contract must be an object"]
    if schema is not None:
        try:
            _validate_schema(document, schema, f"UETD authoring contract at {manifest_path}")
        except UETDRegistryError as exc:
            issues.append(str(exc))
    required = ("contract_version", "status", "kind", "display_name", "parameters", "ports", "numerics", "composition")
    for field_name in required:
        if field_name not in document:
            issues.append(f"{manifest_path}: authoring.{field_name} is required")
    for collection in ("parameters", "ports", "operations"):
        value = document.get(collection, [])
        if not isinstance(value, list) or any(not isinstance(item, dict) for item in value):
            issues.append(f"{manifest_path}: authoring.{collection} must be an array of objects")
    generation = document.get("generation")
    if generation is not None and (
        not isinstance(generation, dict) or not isinstance(generation.get("status"), str) or not isinstance(generation.get("route_id"), str)
    ):
        issues.append(f"{manifest_path}: authoring.generation must declare status and route_id")
    return issues

def _read_schema(root: Path, relative_path: Any) -> dict[str, Any]:
    """Read a JSON schema only from within the specified UETD checkout."""
    if not isinstance(relative_path, str) or not relative_path.strip():
        raise UETDRegistryError("UETD registry schema path must be a non-empty relative path")
    root = root.expanduser().resolve()
    candidate = Path(relative_path.replace("\\", "/"))
    if candidate.is_absolute() or ".." in candidate.parts:
        raise UETDRegistryError("UETD registry schema path must remain inside the checkout")
    path = (root / candidate).resolve()
    if not path.is_relative_to(root):
        raise UETDRegistryError("UETD registry schema path resolves outside the checkout")
    try:
        schema = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise UETDRegistryError(f"Cannot read UETD registry schema {path}: {exc}") from exc
    if not isinstance(schema, dict):
        raise UETDRegistryError(f"UETD registry schema must be an object: {path}")
    return schema

def _load_source_catalogue(root: Path, registry_path: Any, schema_path: Any, description: str) -> dict[str, Any]:
    """Load a UETD API-linked registry and validate it against its UETD schema."""
    if not isinstance(registry_path, str) or not registry_path.strip():
        raise UETDRegistryError(f"UETD API manifest does not declare {description}")
    document = load_registry_document(root, registry_path)
    schema = _read_schema(root, schema_path)
    _validate_schema(document, schema, description)
    return document

def _validate_schema(instance: Any, schema: dict[str, Any], description: str) -> None:
    errors = sorted(Draft202012Validator(schema).iter_errors(instance), key=lambda err: tuple(map(str, err.path)))
    if errors:
        location = ".".join(str(part) for part in errors[0].path) or "<root>"
        raise UETDRegistryError(f"{description} invalid at {location}: {errors[0].message}")

def _mapping(value: Any) -> dict[str, Any]:
    return value if isinstance(value, dict) else {}

def _mapping_list(value: Any) -> list[dict[str, Any]]:
    if not isinstance(value, list):
        return []
    return [item for item in value if isinstance(item, dict)]

def _normalize_parameter(item: dict[str, Any]) -> dict[str, Any]:
    """Expose UETD's canonical ``id`` parameter as the DSL property name."""
    result = dict(item)
    if isinstance(item.get("id"), str):
        result["name"] = item["id"]
    result["type"] = item.get("value_type")
    return result

def _normalize_port(item: dict[str, Any]) -> dict[str, Any]:
    """Expose a UETD port with stable DSL name, type, requiredness, and unit."""
    result = dict(item)
    if isinstance(item.get("id"), str):
        result["name"] = item["id"]
    result["type"] = item.get("value_type")
    result["required"] = not bool(item.get("optional", False))
    units = item.get("units")
    if isinstance(units, str) and units not in {"signal-defined", "output-defined"}:
        result["unit"] = units
    return result

def _module_capabilities(authoring: dict[str, Any]) -> dict[str, Any]:
    return {key: value for key, value in authoring.items() if key.endswith("_capabilities") and isinstance(value, dict)}
