"""Build non-UCON project scaffolding from resolved hardware/project data."""

from __future__ import annotations

import json
import re

from uef_gen.resolver.models import ResolvedProject
from uef_gen.templates.models import GeneratedFile


def _identifier(value: str, fallback: str = "generated") -> str:
    """Return a stable C identifier derived from user-visible configuration."""
    result = re.sub(r"[^A-Za-z0-9_]", "_", value)
    if not result or result[0].isdigit():
        result = f"{fallback}_{result}"
    return result


def _comment_text(value: str) -> str:
    """Keep arbitrary project names from ending a generated C comment."""
    clean = "".join(character if ord(character) >= 32 and ord(character) != 127 else " "
                     for character in value)
    return clean.replace("*/", "* /")


def _protocol_entries(project: ResolvedProject) -> list[tuple[str, str]]:
    raw = project.user_configuration.get("protocols", {})
    entries: list[tuple[str, str]] = []
    if isinstance(raw, dict):
        for key, value in raw.items():
            protocol_type = value.get("type", key) if isinstance(value, dict) else key
            entries.append((str(key), str(protocol_type)))
    elif isinstance(raw, list):
        for index, value in enumerate(raw):
            if isinstance(value, dict):
                entries.append((str(value.get("name", f"protocol_{index}")),
                                str(value.get("type", value.get("name", "unknown")))))
            else:
                entries.append((str(value), str(value)))
    return entries


def render_project_scaffold(
    project: ResolvedProject,
    source_fingerprint: str = "",
    selected_modules: tuple[str, ...] | None = None,
) -> tuple[GeneratedFile, ...]:
    """Render organized project files without embedding UEF runtime or UCON code.

    Hardware-dependent source files are explicit placeholders. They do not
    imply that the reference chip data is sufficient to boot a physical board.
    """
    name = " ".join(project.name.split())
    chip = project.chip
    modules = selected_modules or project.modules
    module_macros = "".join(
        f"#define UEF_MODULE_{_identifier(module).upper()} 1\n"
        for module in modules
    )
    config_header = f"""/* Generated project options for {_comment_text(name)}. */
#ifndef UEF_GENERATED_CONFIG_H
#define UEF_GENERATED_CONFIG_H
#include <stdint.h>

#define UEF_TARGET_CHIP {json.dumps(chip.name)}
#define UEF_TARGET_ARITHMETIC {json.dumps(project.arithmetic)}
#define UEF_TARGET_RTOS {json.dumps(project.rtos)}
#define UEF_SOURCE_FINGERPRINT {json.dumps(source_fingerprint)}
{module_macros}{''.join(f'#define {define}\n' for define in project.defines)}
#endif /* UEF_GENERATED_CONFIG_H */
"""

    caps = project.capabilities
    arch = chip.family.arch
    capability_values = {
        "UHAL_HAS_FPU": caps.has_fpu,
        "UHAL_HAS_DCACHE": caps.dcache,
        "UHAL_HAS_ICACHE": arch.icache,
        "UHAL_HAS_MPU": arch.mpu,
        "UHAL_HAS_DWT": arch.dwt,
        "UHAL_HAS_DSP_SIMD": arch.dsp_simd,
        "UHAL_HAS_HW_DIV": arch.hw_divide,
        "UHAL_HAS_CMSIS_DSP": arch.cmsis_dsp,
        "UHAL_HAS_HRTIM": caps.hrtim,
        "UHAL_HAS_FDCAN": caps.fdcan,
        "UHAL_HAS_CORDIC": caps.cordic,
        "UHAL_HAS_FMAC": caps.fmac,
        "UHAL_HAS_OPAMP": caps.opamp,
        "UHAL_HAS_COMP": caps.comp,
        "UHAL_HAS_USB_FS": caps.usb_fs,
        "UHAL_HAS_DMAMUX": caps.dma_mux,
    }
    target_header = f"""/* Target capability snapshot for {_comment_text(chip.name)}.
 * These values reflect the registered target record, not an independent claim
 * that its vendor data has been validated.
 */
#ifndef UEF_GENERATED_TARGET_H
#define UEF_GENERATED_TARGET_H

#define UEF_TARGET_ARCH_NAME {json.dumps(arch.name)}
#define UEF_TARGET_WORD_BITS {arch.word_bits}U
#define UEF_TARGET_FLASH_BYTES {chip.flash_bytes}ULL
#define UEF_TARGET_SRAM_BYTES {chip.sram_bytes}ULL
#define UEF_TARGET_DATA_VERIFIED {1 if chip.verified else 0}
""" + "".join(
        f"#define {macro} {1 if supported else 0}\n"
        for macro, supported in capability_values.items()
    ) + "#endif /* UEF_GENERATED_TARGET_H */\n"

    board_instance_lines = "".join(
        f"#define UEF_BOARD_PERIPHERAL_{_identifier(p.key or p.instance).upper()} {json.dumps(p.instance)}\n"
        for p in project.peripherals
    )
    board_config = f"""/* Stable project aliases for the resolved board resources. */
#ifndef UEF_GENERATED_BOARD_CONFIG_H
#define UEF_GENERATED_BOARD_CONFIG_H
{board_instance_lines}#endif /* UEF_GENERATED_BOARD_CONFIG_H */
"""
    board_header = """/* Board-level initialization boundary; implement for a verified board. */
#ifndef GENERATED_BOARD_H
#define GENERATED_BOARD_H
#include <uef/ucore/uef_status.h>

uef_status_t board_initialize(void);
void board_deinitialize(void);

#endif /* GENERATED_BOARD_H */
"""
    board_source = f"""/* Board initialization scaffold for {_comment_text(chip.name)}.
 * The target record is {'verified' if chip.verified else 'unverified'}; this placeholder
 * refuses to report successful hardware initialization until pin, clock,
 * DMA, and peripheral setup has been implemented for the selected board.
 */
#include "board/board.h"

uef_status_t board_initialize(void)
{{
    /* TODO(board-initialize): Resolve the verified clock/pin plan, configure clocks before
     * dependent peripherals, then initialize selected UPAL devices in dependency order. Check
     * every status, unwind already-started devices on failure, and return success only when the
     * complete board profile is ready without modifying unowned pins. */
    return UEF_NOT_SUPPORTED;
}}

void board_deinitialize(void)
{{
    /* TODO(board-deinitialize): Stop board-owned services and DMA in reverse dependency order,
     * disable peripheral/interrupt sources, then place configured pins in their documented safe
     * state. Make repeated shutdown harmless and do not alter pins outside this board profile. */
}}
"""

    system_init = """/* Target startup hook. This is deliberately not added to the source list.
 * Implement clock-tree, flash-wait-state, MPU and cache setup from verified
 * vendor data before connecting this function to the reset/startup sequence.
 */
void uef_system_initialize(void)
{
    /* TODO(system-clock-startup): Apply the verified oscillator/PLL and bus-divider sequence;
     * configure flash wait states before raising frequency; enable required memory protection
     * and caches; wait with bounded timeouts for readiness; then verify and publish the actual
     * clock configuration before any timing-dependent peripheral is initialized. */
}
"""
    startup_assembly = """/* GENERATED PLACEHOLDER — replace with the selected vendor startup file.
 * The reset vector, stack top, exception vectors and data/BSS copy ranges are
 * target-specific. This file is not included in project_sources.txt.
 */
"""
    linker_script = """/* GENERATED PLACEHOLDER — replace with a verified target linker script.
 * Define FLASH/RAM origin and length, vector placement, stack/heap limits,
 * load addresses, and all required section-retention rules before linking.
 */
"""
    target_readme = f"""# Target integration for {chip.name}

The generator created named placeholders because this target record is
{'marked verified' if chip.verified else 'not verified'} and does not currently provide a complete startup/linker profile.

- `startup.s` is not a reset handler and is intentionally omitted from the build list.
- `link.ld` contains no memory map and must be replaced before linking.
- `system_init.c` is not called automatically; connect verified clock and memory setup in the board startup flow.
- `board/board.c` returns `UEF_NOT_SUPPORTED` until the physical board setup is implemented.

Use the vendor reference manual, datasheet, errata, CMSIS device package,
board schematic and selected toolchain to fill these files. Do not infer
addresses or clock values from the generic target name.
"""

    task_files = _render_task_scaffold(project)
    protocol_files = _render_protocol_scaffolds(project)
    freertos_files = _render_freertos_config(project)
    application = f"""/* Application lifecycle scaffold for {_comment_text(name)}. */
#include <uef/uapp/uapp_lifecycle.h>
#include <uef/uos/uos.h>
#include "board/board.h"
#include "config/uef_config.h"

static uef_status_t application_initialize(void *context)
{{
    (void)context;
    return board_initialize();
}}

static uef_status_t application_start(void *context)
{{
    (void)context;
    /* TODO(application-start): Start selected services only after their declared dependencies
     * report ready; on the first error, stop already-started services in reverse order and return
     * that failure instead of allowing the scheduler to run a partial application. */
    return UEF_NOT_SUPPORTED;
}}

static void application_stop(void *context)
{{
    (void)context;
    /* TODO(application-stop): Stop task producers first, then stop services in reverse dependency
     * order, drain owned DMA/transfers, flush durable state with bounded waits, and finally call
     * board_deinitialize; make the path safe after partial startup and repeated stop requests. */
}}

int main(void)
{{
    uapp_component_t application = {{
        .state = UAPP_STATE_CREATED,
        .name = {json.dumps(name)},
        .init = application_initialize,
        .start = application_start,
        .stop = application_stop,
        .on_fault = NULL,
        .ctx = NULL,
    }};

    if (uapp_component_init(&application) != UEF_OK) return 1;
    if (uapp_component_start(&application) != UEF_OK) return 2;
    /* TODO(application-main-loop): Use the selected UOS scheduler/lifecycle policy, propagate
     * startup failure without starting tasks, and arrange a defined shutdown path when the
     * scheduler returns or a system fault requests termination. */
    uos_scheduler_start();
    uapp_component_stop(&application);
    return 0;
}}
"""

    project_readme = f"""# {name}

Generated C project skeleton for **{chip.name}**.

## Project layout

- `uef/` contains the selected headers and source modules copied from the UEF checkout resolved for this generation.
- `application/` is the application lifecycle entry point.
- `board/` is the board initialization boundary and deliberately fails closed until implemented.
- `target/` contains startup/linker placeholders that must be replaced with verified target files.
- `config/`, `uos/`, and `protocols/` contain generated configuration surfaces and implementation notes.
- `manifest/` records target resolution, selected modules, UEF identity and source provenance.

## Before building for hardware

1. Select a concrete verified MCU and board record; confirm package pins, clock domains, DMA routes, IRQs and memory regions against vendor documentation.
2. Replace `target/startup.s` and `target/link.ld` with the correct startup/vector and linker files; connect `uef_system_initialize()` in the reset flow.
3. Implement `board_initialize()` and every selected UPAL operation; its scaffold returns `UEF_NOT_SUPPORTED` intentionally.
4. Supply external dependencies listed in `manifest/modules.json`, including the configured RTOS kernel, CMSIS/device headers or FatFS where required.
5. Complete any UCON template contract in the UEF checkout before generating algorithm code from NEXUS ControlIR.

The generic reference target is useful for configuration/assembly only. It is not board data. Python and NEXUS are generation-time tools and are not firmware runtime dependencies.

Canonical source, include, and link lists are `project_sources.txt`, `project_includes.txt`,
`project_defines.txt`, `project_cflags.txt`, and `project_libraries.txt`. The CMake and Make fragments under
`config/` consume those lists; they do not provide missing startup or dependency files.
"""

    return tuple([
        GeneratedFile("application/application.c", application),
        GeneratedFile("config/uef_config.h", config_header),
        GeneratedFile("config/uef_target.h", target_header),
        GeneratedFile("config/uef_board.h", board_config),
        GeneratedFile("board/board.h", board_header),
        GeneratedFile("board/board.c", board_source),
        GeneratedFile("target/system_init.c", system_init),
        GeneratedFile("target/startup.s", startup_assembly),
        GeneratedFile("target/link.ld", linker_script),
        GeneratedFile("target/README.md", target_readme),
        GeneratedFile("README.md", project_readme),
        *task_files,
        *protocol_files,
        *freertos_files,
    ])


def _render_task_scaffold(project: ResolvedProject) -> tuple[GeneratedFile, ...]:
    tasks = [task for task in project.tasks if task.execution_context == "task"]
    if not tasks:
        return ()
    declarations = "".join(
        f"void {_identifier(task.name, 'task')}(void *context);\n" for task in tasks
    )
    entries = "\n".join(
        f"    /* TODO(task-{_identifier(task.name, 'task')}): Allocate or bind the configured "
        f"{task.stack_bytes}-byte stack and task control storage, create the task at "
        f"{task.period_us} us with priority {task.priority} through the selected UOS backend, "
        f"check its creation status, and define how missed periods and startup failure are reported. */"
        for task in tasks
    )
    header = f"""/* Generated task declarations; implement task bodies in the application. */
#ifndef GENERATED_TASKS_H
#define GENERATED_TASKS_H

{declarations}void uef_project_tasks_create(void);

#endif /* GENERATED_TASKS_H */
"""
    source = f"""#include "uos/tasks.h"

void uef_project_tasks_create(void)
{{
{entries}
}}
"""
    return (GeneratedFile("uos/tasks.h", header), GeneratedFile("uos/tasks.c", source))


def _render_protocol_scaffolds(project: ResolvedProject) -> tuple[GeneratedFile, ...]:
    files: list[GeneratedFile] = []
    for name, protocol_type in _protocol_entries(project):
        stem = _identifier(name).lower()
        macro = _identifier(f"{name}_{protocol_type}").upper()
        header = f"""/* Protocol configuration placeholder for {protocol_type} ({name}). */
#ifndef GENERATED_PROTOCOL_{macro}_H
#define GENERATED_PROTOCOL_{macro}_H

#define UEF_PROTOCOL_{macro}_TYPE {json.dumps(protocol_type)}
/* TODO(protocol-{macro.lower()}): Resolve baud/bit timing, framing, polarity, and pin assignment
 * from the selected protocol profile and verified target clock; validate the values against the
 * transport limits and publish named constants only when every required setting is known. */

#endif /* GENERATED_PROTOCOL_{macro}_H */
"""
        files.append(GeneratedFile(f"protocols/{stem}_config.h", header))
    return tuple(files)


def _render_freertos_config(project: ResolvedProject) -> tuple[GeneratedFile, ...]:
    if project.rtos.casefold() != "freertos":
        return ()
    target = project.user_configuration.get("target", {})
    settings = target.get("freertos", {}) if isinstance(target, dict) else {}
    if not isinstance(settings, dict):
        settings = {}
    tick_hz = int(settings.get("tick_hz", 1000))
    highest_priority = max((task.priority for task in project.tasks if task.execution_context == "task"), default=0)
    heap_bytes = int(settings.get("heap_bytes", 0))
    static_allocation = bool(settings.get("static_allocation", True))
    header = f"""/* FreeRTOS application configuration scaffold.
 * The kernel sources/port remain an external dependency of the firmware project.
 * Confirm every option against the selected FreeRTOS release and port.
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#ifndef UEF_CPU_CLOCK_HZ
#error Define UEF_CPU_CLOCK_HZ from the verified board clock configuration
#endif
#define configCPU_CLOCK_HZ UEF_CPU_CLOCK_HZ
#define configTICK_RATE_HZ {tick_hz}UL
#define configMAX_PRIORITIES {highest_priority + 1}U
#define configTOTAL_HEAP_SIZE {heap_bytes}U
#define configSUPPORT_STATIC_ALLOCATION {1 if static_allocation else 0}
#define configSUPPORT_DYNAMIC_ALLOCATION {1 if heap_bytes > 0 else 0}

/* TODO(freertos-port-config): Resolve interrupt priorities and timer hooks from the chosen port;
 * select its stack-depth type, assertions, FPU/MPU settings, allocation hooks, and ISR-yield macro;
 * then cross-check heap/allocation and priority settings against the generated task profile.
 */

#endif /* FREERTOS_CONFIG_H */
"""
    return (GeneratedFile("uos/FreeRTOSConfig.h", header),)
