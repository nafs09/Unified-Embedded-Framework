from __future__ import annotations

import json
from typing import Any

from uef_gen.resolver.models import ResolvedProject
from uef_gen.templates.registry import GeneratedFile


def render_project_scaffold(project: ResolvedProject, snapshot_hash: str = "") -> tuple[GeneratedFile, ...]:
    """Render only the generic application files that do not own UCON behavior.

    UCON's reusable scalar types and algorithm implementation belong to UEF.
    Until that UEF contract is present, this scaffold must not emit a second
    generated copy of UCON types.
    """
    name = " ".join(project.name.split())
    chip = " ".join(project.chip.name.split())
    comment_name = "".join(character if ord(character) >= 32 and ord(character) != 127 else " "
                           for character in name).replace("*/", "* /")
    c_string = lambda value: json.dumps(value, ensure_ascii=True)
    config = f"""/* Generated target configuration for {comment_name}. */
#ifndef GENERATED_CONFIG_H
#define GENERATED_CONFIG_H
#define UEF_TARGET_CHIP {c_string(chip)}
#define UEF_TARGET_ARITHMETIC {c_string(project.arithmetic)}
#define UEF_TARGET_RTOS {c_string(project.rtos)}
#define UEF_SNAPSHOT_HASH {c_string(snapshot_hash)}
""" + "".join(f"#define {define}\n" for define in project.defines) + "#endif\n"
    app = f"""/* Generated application entry point. Add verified board startup here. */
#include <uef/uapp/uapp_lifecycle.h>
#include <uef/uos/uos.h>
#include <generated/config.h>

static uef_status_t application_initialize(void *context) {{
    (void)context;
    /* Initialize board-owned drivers and application components here. */
    return UEF_OK;
}}

static uef_status_t application_start(void *context) {{
    (void)context;
    /* Start only after every required component has initialized successfully. */
    return UEF_OK;
}}

static void application_stop(void *context) {{
    (void)context;
    /* Stop components in the order required by the application. */
}}

int main(void) {{
    uapp_component_t application = {{
        .state = UAPP_STATE_CREATED,
        .name = {c_string(name)},
        .init = application_initialize,
        .start = application_start,
        .stop = application_stop,
        .on_fault = NULL,
        .ctx = NULL,
    }};

    if (uapp_component_init(&application) != UEF_OK) return 1;
    if (uapp_component_start(&application) != UEF_OK) return 2;
    /* FreeRTOS owns control after this call; bare-metal apps own their loop. */
    uos_scheduler_start();
    uapp_component_stop(&application);
    return 0;
}}
"""
    readme = f"""# {name}

Generated embedded project skeleton for **{chip}**.

The canonical build description is in `project_sources.txt`, `project_includes.txt`,
`project_defines.txt`, and `project_cflags.txt`. Add a target startup/backend,
verified board drivers, and registered algorithm/template extensions before
using this output on hardware. NEXUS and uef-gen are not firmware runtime dependencies.
"""
    return (GeneratedFile("src/main.c", app), GeneratedFile("include/generated/config.h", config),
            GeneratedFile("README.md", readme))


def render_user_algorithms(algorithms: tuple[Any, ...], context: dict[str, Any]) -> tuple[GeneratedFile, ...]:
    """Call registered legacy algorithm renderers until UEF owns this boundary.

    This adapter is migration-only. New algorithm bodies and reusable UCON
    type definitions belong in the verified UEF snapshot, not these extensions.
    """
    from uef_gen.templates.registry import render_algorithm
    result: list[GeneratedFile] = []
    for algorithm in algorithms:
        result.extend(render_algorithm(algorithm, context))
    return tuple(result)
