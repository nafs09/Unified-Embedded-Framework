# Cortex-M templates

`startup.s.tmpl` renders a GNU assembler reset handler and vector table.
`linker.ld.tmpl` renders a GNU ld memory map with data/BSS initialization,
heap and stack bounds, and constructor sections. `system_init.c.tmpl` renders
an ordered sequence of target initialization calls.

Render `startup.s.tmpl` with:

- `cpu`: GNU assembler core name, such as `cortex-m4` or `cortex-m7`.
- `vector_alignment_bytes`: device-required vector-table alignment in bytes; use the exact core/device profile value.
- `vectors`: ordered vector symbols after the initial stack and reset entries; use `"0"` for reserved slots.
- `weak_handlers`: vector handler symbols that should resolve to `Default_Handler` unless the application provides a strong definition.
- `system_init_symbol`: system initialization function called before C data initialization.
- `runtime_init_symbol`: C runtime initializer symbol, or an empty string when none is required.
- `main_symbol`: application entry symbol.

Render `linker.ld.tmpl` with:

- `entry_symbol`: linker entry function.
- `vector_alignment_bytes`: same verified vector-table alignment used by the startup template.
- `memory_regions`: ordered records with `name`, GNU ld `attributes`, `origin`, and `length`.
- `flash_region`, `ram_region`, and `stack_region`: names from `memory_regions`.
- `heap_bytes` and `stack_bytes`: reserved capacities expressed as linker integer literals.

Render `system_init.c.tmpl` with `headers` (include paths), `symbol` (the
exported function name), and `calls` (ordered, no-argument initialization
function symbols). The call sequence must come from the selected verified
target profile.

The `cmsis_dsp` architecture flag means that the core can use CMSIS-DSP
kernels; it does not mean the library has been installed or linked. The
separate `cmsis-dsp` package record must be pinned and selected when a project
uses those functions.

Render `toolchain.cmake.tmpl` with `processor`, `c_compiler_name`,
`asm_compiler_name`, `objcopy_name`, `size_name`, `c_flags`, `asm_flags`, and
`linker_flags`. Flags must come from a pinned compiler profile. The template
uses executable names resolved through the selected tool package; it does not
pick an FPU ABI or compiler version.

Render `CMakeLists.txt.tmpl` with `project_name`, `target_name`, `sources`,
`include_dirs`, `compile_definitions`, `compile_options`, `linker_script`,
`link_options`, `libraries`, `emit_ihex`, and `emit_binary`. It creates an ELF
and optionally Intel HEX/binary copies. Those outputs are build artifacts,
not an upload plan; the board's programmer profile owns address/range checks.

The Cortex-M compiler package is `arm-none-eabi`; CMake and Ninja are separate
host packages. STM32 device support is selected by family: `stm32cube-f4`,
`stm32cube-f7`, `stm32cube-g0`, `stm32cube-g4`, `stm32cube-h7`,
`stm32cube-h5`, `stm32cube-l4plus`, or `stm32cube-u5`. `cmsis-dsp` is
optional. These package records are scaffold-only and have no pinned,
installable payloads.

## Phase 0–2 core and memory constraints

| Family | Core model | Template-specific constraint |
| --- | --- | --- |
| STM32G0B1 | Cortex-M0+, Armv6-M, MPU, no FPU/DSP extension | Startup must use Thumb-1-compatible instructions; the generic startup now avoids post-index load/store forms. |
| STM32F4, STM32G4, STM32L4+ | Cortex-M4/F, DSP, hardware divide, MPU; FPU is part-specific | Use exact per-part FPU flags. Linker memory region input must describe each RAM bank/CCM separately; the simple startup initializes only `.data` and `.bss` in one selected RAM region. |
| STM32F7 and single-core STM32H7 | Cortex-M7 with caches, MPU, FPU and TCM/SRAM domains | Exact cache policy and DMA-visible RAM placement must be reviewed. The current generic linker does not initialize multiple RAM data banks or generate cache setup. |
| STM32H5 and STM32U5 | Cortex-M33F with TrustZone-M, caches, MPU and secure attribution | Use the dual-image TrustZone build template below and separately reviewed secure/non-secure linker scripts. |
| STM32H745/H755 | Cortex-M7 + Cortex-M4 | Do not use the single-core startup/link template as a whole-chip profile. Each core needs its own vectors, startup, image, linker map, and resource ownership. |
| nRF52840 | Cortex-M4F with radio subsystem | Use the Nordic/Zephyr template only when Zephyr/NCS is the selected runtime; Nordic SDK startup, memory layout, and radio ownership are not supplied by the generic adapter. |
| RP2040 | Dual Cortex-M0+ with board-supplied XIP flash | Use the Pico SDK project adapter and explicit core/PIO/DMA allocations. The SoC SIO divider is separate from the Cortex-M0+ instruction set. |
| i.MX RT1062 | Cortex-M7 with cache/TCM and external boot flash | The board's boot source, FlexSPI, IVT/FCB and external memory profile must supply the actual image layout. |

## TrustZone-M two-image CMake template

`trustzone/CMakeLists.txt.tmpl` creates secure and non-secure ELF targets and
connects the secure CMSE import library to the non-secure link. Render it with
`project_name`, `secure_target`, `nonsecure_target`, `secure_sources`,
`nonsecure_sources`, `include_dirs`, `secure_compile_options`,
`nonsecure_compile_options`, `secure_linker_script`, `nonsecure_linker_script`,
`secure_link_options`, `nonsecure_link_options`, `cmse_import_library`, and
`common_libraries`. The secure link enables CMSE import-library generation;
all flash boundaries, NSC sizing, startup/vector code, attribution, and signing
remain external, explicit profile inputs. This template does not provision
option bytes or create a combined signed boot image.

## Dual-core Cortex-M7/M4 CMake template

`dual_core/CMakeLists.txt.tmpl` creates independent M7 and M4 ELF targets for
parts such as STM32H745/H755. Render it with `project_name`, `cm7_target`,
`cm4_target`, `cm7_sources`, `cm4_sources`, and each core's
`*_include_dirs`, `*_compile_definitions`, `*_compile_options`,
`*_linker_script`, `*_link_options`, and `*_libraries`. Use separate startup
and vector sources and exact linker maps. The template does not encode core
release/reset sequencing, shared peripheral ownership, HSEM protocol,
shared-memory placement, cache maintenance, IPC, combined-image packaging, or
a dual-image flash procedure. Those remain target-profile requirements.
