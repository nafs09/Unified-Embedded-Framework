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
    clean = "".join(character if ord(character) >= 32 and ord(character) != 127 else " " for character in value)
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
                entries.append((str(value.get("name", f"protocol_{index}")), str(value.get("type", value.get("name", "unknown")))))
            else:
                entries.append((str(value), str(value)))
    return entries

def _render_system_init_scaffold(project: ResolvedProject) -> str:
    """Render the target initialization hook and its clock/memory TODO.

    TODO(target-system-init-renderer): use the resolved board build profile to
    select the initialization owner. Cortex-M may call a UETD hook from startup;
    AVR CRT and ESP-IDF own different reset paths. Never generate guessed
    clock-register writes or an initialization call the selected runtime will
    not make.
    """
    return f"""/* Target startup hook for {_comment_text(project.chip.name)}.
 * This generic scaffold is not added to the source list or called automatically.
 * Choose the owning startup/runtime from the verified board build profile.
 */
void uetd_system_initialize(void)
{{
    /* TODO(system-clock-startup): For a profile that uses this hook, apply its verified
     * oscillator/PLL and bus-divider sequence; configure flash wait states before raising
     * frequency; configure required MPU/cache regions before DMA use; wait with bounded
     * timeouts; then verify and publish actual clocks before timing-dependent drivers start.
     * Do not emit this hook for profiles whose SDK/CRT owns startup unless an explicit call
     * point is part of that profile. */
}}
"""

def _render_startup_scaffold() -> str:
    """Render a non-buildable note where a target reset/vector file belongs."""
    return """/* GENERATED PLACEHOLDER — not a reset handler and not in project_sources.txt.
 * TODO(target-startup): select the verified architecture/core startup source;
 * define reset entry, stack, vectors, data/BSS initialization, FPU state,
 * interrupt vector ownership, and the call point for board/system init. Prefer
 * the target SDK/CRT startup where that runtime owns reset handling.
 */
"""

def _render_linker_scaffold() -> str:
    """Render a non-linkable note until verified memory geometry is available."""
    return """/* GENERATED PLACEHOLDER — has no memory map and must not be linked.
 * TODO(target-linker): derive FLASH/RAM origins and lengths from the selected
 * exact chip record; reserve bootloader/secure/dual-core/DMA areas; define
 * vector, text, data, BSS, stack, heap, load, and retained sections; then check
 * the map-file ranges against the board's verified layout.
 */
"""

def _render_target_readme(project: ResolvedProject) -> str:
    """Explain the selected target placeholders and architecture-specific owner."""
    chip = project.chip
    board = project.development_board or {}
    build = board.get("build", {}) if isinstance(board, dict) else {}
    backend = build.get("backend", "unspecified") if isinstance(build, dict) else "unspecified"
    backend_key = backend.casefold() if isinstance(backend, str) else ""
    if backend_key in {"esp-idf", "esp_idf"}:
        file_policy = """ESP-IDF owns reset/startup, linker fragments, partition layout, and image offsets.
No generic startup or linker placeholder was emitted. Implement the IDF project
and component adapter described in `UETD/templates/targets/esp_idf/README.md`.
"""
    elif backend_key in {"arduino-cli", "arduino"}:
        file_policy = """The selected Arduino core/CRT owns reset/startup and the AVR link recipe.
No generic startup or linker placeholder was emitted. Implement the Arduino CLI
adapter described in `UETD/templates/targets/arduino/README.md`.
"""
    else:
        file_policy = """The generic `startup.s` and `link.ld` files are named placeholders with no
reset behavior or memory map. They are omitted from the canonical source list
and must be replaced by verified files before linking. `system_init.c` is not
called automatically.
"""
    return f"""# Target integration for {chip.name}
Target data status: {"marked verified" if chip.verified else "not verified"}.
Selected build backend: `{backend}`.
{file_policy}
- When present, `startup.s` is not a reset handler. Cortex-M needs its selected core/vector
  startup; AVR normally uses the compiler CRT; ESP-IDF owns bootloader and
  runtime startup.
- When present, `link.ld` contains no memory map. Cortex-M may use a reviewed linker script;
  AVR linker/CRT options come from the exact MCU profile; ESP-IDF owns linker
  fragments and partition layout.
- `system_init.c` is not called automatically. Connect it only when the
  selected backend has an explicit, verified call point.
- `board/board.c` returns `UETD_NOT_SUPPORTED` until physical board setup has
  been implemented and verified.
TODO(target-profile): add sourced startup/linker/runtime paths, exact part and
core selection, memory reservations, toolchain executable profile, output
format, flash geometry, and board initialization order to the UETD records.
Use the vendor reference manual, datasheet, errata, board schematic, and
selected toolchain. Do not infer register addresses or clocks from a family
name.
"""

def _render_target_files(project: ResolvedProject) -> tuple[GeneratedFile, ...]:
    """Emit only placeholders compatible with the selected board build adapter.

    TODO(target-file-selection): use a typed adapter ID from the resolved UETD
    board profile and render its UETD-owned template set. ESP-IDF and Arduino
    own startup/CRT and linker behavior, so do not emit Cortex-style startup or
    linker files for those backends. Unknown profiles get a documentation-only
    manual action and are never treated as ready to build for hardware.
    """
    board = project.development_board or {}
    build = board.get("build", {}) if isinstance(board, dict) else {}
    backend = build.get("backend", "") if isinstance(build, dict) else ""
    backend_key = backend.casefold() if isinstance(backend, str) else ""
    if backend_key in {"esp-idf", "esp_idf", "arduino-cli", "arduino"}:
        return (GeneratedFile("target/README.md", _render_target_readme(project)),)
    return (
        GeneratedFile("target/system_init.c", _render_system_init_scaffold(project)),
        GeneratedFile("target/startup.s", _render_startup_scaffold()),
        GeneratedFile("target/link.ld", _render_linker_scaffold()),
        GeneratedFile("target/README.md", _render_target_readme(project)),
    )

def render_project_scaffold(
    project: ResolvedProject, source_fingerprint: str = "", selected_modules: tuple[str, ...] | None = None
) -> tuple[GeneratedFile, ...]:
    """Render organized project files without embedding UETD runtime or UCON code.

    Hardware-dependent source files are explicit placeholders. They do not
    imply that the reference chip data is sufficient to boot a physical board.
    """
    name = " ".join(project.name.split())
    chip = project.chip
    board_id = project.development_board["id"] if project.development_board else ""
    board_description = (
        f"Selected board: **{project.development_board['name']}** (`{board_id}`)."
        if project.development_board
        else "No development-board record was selected; board setup remains a project integration step."
    )
    modules = selected_modules or project.modules
    module_macros = "".join(f"#define UETD_MODULE_{_identifier(module).upper()} 1\n" for module in modules)
    define_macros = "".join(f"#define {define}\n" for define in project.defines)
    config_header = f"""/* Generated project options for {_comment_text(name)}. */
#ifndef UETD_GENERATED_CONFIG_H
#define UETD_GENERATED_CONFIG_H
#include <stdint.h>
#define UETD_TARGET_CHIP {json.dumps(chip.name)}
#define UETD_TARGET_CHIP_ID {json.dumps(chip.registry_id or chip.name)}
#define UETD_TARGET_DEVELOPMENT_BOARD {json.dumps(board_id)}
#define UETD_TARGET_ARITHMETIC {json.dumps(project.arithmetic)}
#define UETD_TARGET_RTOS {json.dumps(project.rtos)}
#define UETD_SOURCE_FINGERPRINT {json.dumps(source_fingerprint)}
{module_macros}{define_macros}
#endif /* UETD_GENERATED_CONFIG_H */
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
    target_header = (
        f"""/* Target capability snapshot for {_comment_text(chip.name)}.
 * These values reflect the registered target record, not an independent claim
 * that its vendor data has been validated.
 */
#ifndef UETD_GENERATED_TARGET_H
#define UETD_GENERATED_TARGET_H
#define UETD_TARGET_ARCH_NAME {json.dumps(arch.name)}
#define UETD_TARGET_WORD_BITS {arch.word_bits}U
#define UETD_TARGET_FLASH_BYTES {project.flash_bytes if project.flash_bytes is not None else chip.flash_bytes}ULL
#define UETD_TARGET_SRAM_BYTES {chip.sram_bytes}ULL
#define UETD_TARGET_DATA_VERIFIED {1 if chip.verified else 0}
"""
        + "".join(f"#define {macro} {1 if supported else 0}\n" for macro, supported in capability_values.items())
        + "#endif /* UETD_GENERATED_TARGET_H */\n"
    )
    board_instance_lines = "".join(
        f"#define UETD_BOARD_PERIPHERAL_{_identifier(p.key or p.instance).upper()} {json.dumps(p.instance)}\n" for p in project.peripherals
    )
    board_config = f"""/* Stable project aliases for the resolved board resources. */
#ifndef UETD_GENERATED_BOARD_CONFIG_H
#define UETD_GENERATED_BOARD_CONFIG_H
{board_instance_lines}#endif /* UETD_GENERATED_BOARD_CONFIG_H */
"""
    board_header = """/* Board-level initialization boundary; implement for a verified board. */
#ifndef GENERATED_BOARD_H
#define GENERATED_BOARD_H
#include <ucore/status.h>

uetd_status_t board_initialize(void);
void board_deinitialize(void);

#endif /* GENERATED_BOARD_H */
"""
    board_source = f"""/* Board initialization scaffold for {_comment_text(chip.name)}.
 * The target record is {"verified" if chip.verified else "unverified"}; this placeholder
 * refuses to report successful hardware initialization until pin, clock,
 * DMA, and peripheral setup has been implemented for the selected board.
 */
#include "board/board.h"
uetd_status_t board_initialize(void)
{{
    /* TODO(board-initialize): Resolve the verified clock/pin plan, configure clocks before
     * dependent peripherals, then initialize selected UPAL devices in dependency order. Check
     * every status, unwind already-started devices on failure, and return success only when the
     * complete board profile is ready without modifying unowned pins. */
    return UETD_NOT_SUPPORTED;
}}
void board_deinitialize(void)
{{
    /* TODO(board-deinitialize): Stop board-owned services and DMA in reverse dependency order,
     * disable peripheral/interrupt sources, then place configured pins in their documented safe
     * state. Make repeated shutdown harmless and do not alter pins outside this board profile. */
}}
"""
    target_files = _render_target_files(project)
    task_files = _render_task_scaffold(project)
    protocol_files = _render_protocol_scaffolds(project)
    freertos_files = _render_freertos_config(project)
    application = f"""/* Application lifecycle scaffold for {_comment_text(name)}. */
#include <uapp/lifecycle.h>
#include <uos/uos.h>
#include "board/board.h"
#include "config/uetd_config.h"
static uetd_status_t application_initialize(void *context)
{{
    (void)context;
    return board_initialize();
}}
static uetd_status_t application_start(void *context)
{{
    (void)context;
    /* TODO(application-start): Start selected services only after their declared dependencies
     * report ready; on the first error, stop already-started services in reverse order and return
     * that failure instead of allowing the scheduler to run a partial application. */
    return UETD_NOT_SUPPORTED;
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
    if (uapp_component_init(&application) != UETD_OK) return 1;
    if (uapp_component_start(&application) != UETD_OK) return 2;
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
{board_description}
## Project layout
- `uetd/` contains the selected headers and source modules copied from the UETD checkout resolved for this generation.
- `application/` is the application lifecycle entry point.
- `board/` is the board initialization boundary and deliberately fails closed until implemented.
- `target/` describes the selected build/startup strategy; consult its README for architecture-specific file ownership.
- `config/`, `uos/`, and `protocols/` contain generated configuration surfaces and implementation notes.
- `manifest/` records target resolution, selected modules, UETD identity and source provenance.
## Before building for hardware
1. Select a concrete verified MCU and board record; confirm package pins, clock domains, DMA routes, IRQs and memory regions against vendor documentation.
2. Follow `target/README.md` for the selected backend. Supply verified startup/link behavior only where that runtime requires it; connect `uetd_system_initialize()` only at an explicit call point.
3. Implement `board_initialize()` and every selected UPAL operation; its scaffold returns `UETD_NOT_SUPPORTED` intentionally.
4. Supply external dependencies listed in `manifest/modules.json`, including the configured RTOS kernel, CMSIS/device headers or FatFS where required.
5. Review the UCON algorithm contract in the UETD checkout before using an algorithm in the application.
The generic reference target is useful for configuration/assembly only. It is not board data. Python and NEXUS are generation-time tools and are not firmware runtime dependencies.
Canonical source, include, and link lists are `project_sources.txt`, `project_includes.txt`,
`project_defines.txt`, `project_cflags.txt`, and `project_libraries.txt`. The CMake and Make fragments under
`config/` consume those lists; they do not provide missing startup or dependency files.
"""
    return tuple(
        [
            GeneratedFile("application/application.c", application),
            GeneratedFile("config/uetd_config.h", config_header),
            GeneratedFile("config/uetd_target.h", target_header),
            GeneratedFile("config/uetd_board.h", board_config),
            GeneratedFile("board/board.h", board_header),
            GeneratedFile("board/board.c", board_source),
            *target_files,
            GeneratedFile("README.md", project_readme),
            *task_files,
            *protocol_files,
            *freertos_files,
        ]
    )

def _render_task_scaffold(project: ResolvedProject) -> tuple[GeneratedFile, ...]:
    tasks = [task for task in project.tasks if task.execution_context == "task"]
    if not tasks:
        return ()
    declarations = "".join(f"void {_identifier(task.name, 'task')}(void *context);\n" for task in tasks)
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
{declarations}void uetd_project_tasks_create(void);
#endif /* GENERATED_TASKS_H */
"""
    source = f"""#include "uos/tasks.h"
void uetd_project_tasks_create(void)
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
#define UETD_PROTOCOL_{macro}_TYPE {json.dumps(protocol_type)}
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
#ifndef UETD_CPU_CLOCK_HZ
#error Define UETD_CPU_CLOCK_HZ from the verified board clock configuration
#endif
#define configCPU_CLOCK_HZ UETD_CPU_CLOCK_HZ
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
