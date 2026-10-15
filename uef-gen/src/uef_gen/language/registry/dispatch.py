"""Fixed allowlist of standalone-language frontend dispatch IDs.

Registry JSON remains declarative: it cannot name Python modules, functions,
or imports. A listed lowering ID means AST/application-model lowering, not
that a target backend can emit C for the feature.
"""

from __future__ import annotations
from uef_gen.language.registry.loader import LanguageRegistry, LanguageRegistryError

KNOWN_PARSE_RULE_IDS = frozenset(
    {
        "project",
        "implementation_block",
        "property",
        "property_block",
        "named_registry_declaration",
        "typed_binding",
        "received_type_spec",
        "registered_component",
        "dsl_map_expression",
        "dsl_select_expression",
        "sensor_declaration",
        "actuator_declaration",
        "algorithm_instance",
        "custom_algorithm_definition",
        "model_definition",
        "hybrid_algorithm_definition",
        "subsystem_definition",
        "choose_output",
        "enum_decl",
        "variant_decl",
        "flags_decl",
        "fixed_type_decl",
        "bounded_buffer_type",
        "match_expression",
        "variant_constructor",
        "flags_operation",
        "boundary_record_decl",
        "state_machine_definition",
        "protocol_instance",
        "custom_protocol_definition",
        "operatingsystem",
        "loop_block",
        "taskset_block",
        "project_parameter_decl",
        "project_routine_signature",
        "stage_call",
        "state_machine_decl",
        "task_run_decl",
        "equation",
        "bounded_loop_statement",
        "conditional_statement",
        "map_expression",
        "select_expression",
        "unit_conversion_expression",
    }
)

KNOWN_LOWERING_RULE_IDS = frozenset(
    {
        "project_metadata",
        "implementation_collection",
        "hardware_target",
        "numeric_profile",
        "peripheral_binding",
        "bus_binding",
        "signal_declaration",
        "received_envelope_type",
        "uetd_component_instance",
        "dsl_value_map",
        "dsl_conditional_value",
        "checked_unit_conversion",
        "typed_record_construction",
        "lazy_typed_conditional",
        "sensor_binding",
        "actuator_binding",
        "uetd_algorithm_instance",
        "standalone_custom_algorithm",
        "hybrid_algorithm_instance",
        "subsystem_wrapper",
        "ordered_component_call",
        "ordered_lazy_output_selection",
        "state_machine_component",
        "fixed_width_project_enum",
        "standalone_model",
        "uetd_protocol_instance",
        "standalone_custom_protocol",
        "operatingsystem_root",
        "baremetal_main_loop",
        "runtime_schedule",
        "task_interrupt",
        "task_channel",
        "custom_equation",
        "bounded_loop",
        "conditional_branch",
        "required_project_input",
        "project_callback_contract",
        "scheduled_component_invocation",
        "tagged_variant",
        "option_result",
        "exhaustive_match",
        "nominal_flags",
        "fixed_point_format",
        "bounded_byte_buffer",
        "shared_memory_record",
        "persistent_record",
        "umath_abi_binding",
        "type_interchange_manifest",
    }
)

def validate_dispatch_ids(registry: LanguageRegistry) -> None:
    """Reject registry IDs not listed in this fixed dispatch allowlist."""
    unknown_parse = {item.parse_rule for item in registry.constructs.values() if item.parse_rule not in KNOWN_PARSE_RULE_IDS}
    unknown_parse.update(item.parse_rule for item in registry.builtins.values() if item.parse_rule not in KNOWN_PARSE_RULE_IDS)
    unknown_lowering = {item.lowering_rule for item in registry.constructs.values() if item.lowering_rule not in KNOWN_LOWERING_RULE_IDS}
    unknown_lowering.update(item.lowering_rule for item in registry.builtins.values() if item.lowering_rule not in KNOWN_LOWERING_RULE_IDS)
    if unknown_parse or unknown_lowering:
        details = []
        if unknown_parse:
            details.append(f"unknown parse rules: {', '.join(sorted(unknown_parse))}")
        if unknown_lowering:
            details.append(f"unknown lowering rules: {', '.join(sorted(unknown_lowering))}")
        raise LanguageRegistryError("Invalid language registry dispatch IDs (" + "; ".join(details) + ")")
