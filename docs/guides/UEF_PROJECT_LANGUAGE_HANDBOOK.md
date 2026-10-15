# UEF-PL user handbook

This handbook is for people who have downloaded the Unified Embedded
Framework repository and want to understand how a standalone UETD project is
written. It introduces the `.uef` and `.uefs` files, explains the role of
`uef-gen`, and points to the current UETD catalogues.

## What you are downloading

The repository contains two related projects:

- **UETD** is the C11 embedded framework. Applications can use its public C
  APIs directly. Start with the [UETD README](../../UETD/README.md) for the
  library structure and current build requirements.
- **UEF Project Language (UEF-PL)** is the standalone authoring language.
  A `.uef` file describes one firmware target; a `.uefs` file describes a
  group of targets and their communication links. It is designed to use UETD
  components and target data without requiring NEXUS.

Python and NEXUS do not run on the embedded target. The intended result of
generation is a native embedded project built from UETD and the selected target
toolchain.

## Current status

UEF-PL's lexer, parser, registry discovery, semantic checker, and lowering are
implemented. `uef-gen check` validates a `.uef` project and `.uefs` system
contract. The standalone firmware emitter is not implemented yet, so
`generate`, `build`, and `flash` cannot currently produce or deploy firmware.
Examples are design references; validation does not certify a physical board
configuration.

You can build the portable UETD C library as described in the
[UETD README](../../UETD/README.md). To install the current Python tool and use
its implemented generator commands, follow the
[uef-gen installation instructions](../../uef-gen/README.md#requirements-and-installation).
The standalone `.uef` commands remain a planned workflow until the front end
and generation backend are implemented.

## First look at a `.uef` project

The source is ordered so the project identity and component declarations are
easy to scan, the operating-system plan appears next, and detailed bindings
follow in the implementation block. This abbreviated example omits component
settings and the subsystem body; the linked examples show complete project
shapes.

```uetd
project BareMetalMotorLoop {
  target = "<registered-board-id>";
  numeric {
    scalar = f32; accumulator = f64;
    rounding = nearest_even; overflow = reject;
    non_finite = reject; fused_multiply_add = disabled;
    matrix_layout = row_major;
  }

  sensor wheel : encoder;
  algorithm speed_pid : pid;
  actuator motor : h_bridge;
  subsystem motor_speed_loop;

  operatingsystem MotorLoop {
    runtime = BareMetal;
    state_machine control_mode : ControlMode {
      state disabled;
      state active;
      initial disabled;
      owner = control_loop;
      consumers = [];
      transition disabled -> active when enable_command;
      transition active -> disabled when not wheel.valid;
    }
    loop control_loop {
      run = motor_speed_loop();
      rate = 200 Hz;
      deadline = 5 ms;
      on_miss = disable_motor;
    }
  }

  implementation {
    // Example UETD-backed body: algorithm speed_pid { ... } pid;
    // A subsystem body is language-level composition and has no suffix.
  }
}
```

The `<registered-board-id>` is a placeholder. A real project uses an exact
board or target identifier from UETD's target catalogue. `encoder`, `pid`, and
`h_bridge` are module descriptors that resolve through UETD's module registry.

In a state machine, `any` matches every state, including the destination. A
same-state transition runs exit and entry actions again, so guard a latched
destination against repeat entry. Fault transitions should precede other
global transitions. Every machine has one scheduled `owner` and an explicit
`consumers` list. The owner alone evaluates and changes the machine; consumers
receive independent read-only latest-value state snapshots at their own
scheduled activation. Listing a consumer does not wake or reprioritize it; use
a channel trigger or registered notification when a task must run on a state
change. A BareMetal machine is owned by the single loop; an RTOS machine is
owned by one task with a declared rate/period or trigger. Every operating
system declares one `component_failure_policy` for task, loop, and FSM
callback failures. The firmware emitter and runtime binding are not available,
so these are validated backend contracts rather than executable state-machine
implementations.
The subsystem is declared before it is configured; its inputs, outputs,
component bindings, and ordered stages belong in `implementation`.

Every UETD-backed implementation repeats its descriptor after the closing
brace, such as `} pid;`. This suffix must match the constructor declaration
above (`algorithm speed_pid : pid;`). It makes the implementation's UETD
module apparent while reading the body and is checked as part of the source.
Project-defined `custom algorithm`, `custom protocol`, `model`, `hybrid
algorithm`, and `subsystem` definitions do not take this suffix.

### Reading the main declarations

| Declaration | What it describes |
|---|---|
| `project` | One independently built firmware target. |
| `target` | A registered chip or development-board identity. |
| `numeric` | Scalar/accumulator types, rounding, overflow, non-finite, FMA, and matrix-layout policy. |
| `record`, `enum` | Project-owned fixed-layout data types. A state machine can synthesize its own enum type from inline `state` declarations. |
| `project routine` | Project-owned, zero-argument callback contract. FSM hooks use `returns status`; routines can be global or attached to a component. |
| `sensor`, `actuator`, `algorithm`, `protocol`, `peripheral`, `bus` | Typed component roles whose descriptors are checked against UETD's catalogues. |
| `subsystem` | A reusable, typed composition of other declared components. Its stages describe execution order. |
| `hybrid algorithm` | A project-level algorithm composition with an explicit interface and selection/fallback contract. |
| `operatingsystem` | Runtime, state machines, loops or tasks, channels, resource ownership, timing, and failure policy. |
| `implementation` | Detailed component settings, resource bindings, models, custom bodies, and subsystem/hybrid definitions. |

For syntax and language rules, continue with the
[technical language guide](UEF_PROJECT_LANGUAGE_GUIDE.md). For the complete
determinism, type, expression, subsystem, hybrid, and OS contract, see the
[language design specification](../design/PROJECT_LANGUAGE_DESIGN.md).

Project routines declare callback ownership; they do not contain the routine's
source implementation. A state action can invoke a declared project routine
only when it returns status, so UETD can fault the machine on failure. UETD
operations remain discoverable from the selected module's authoring contract.
The default callback context is `task`. A project routine used by an interrupt
must declare `context = isr_signal_only`, and the interrupt must declare the
same context. An interrupt can also name a task or loop as deferred work using
`context = isr_signal_only`.

## How a standalone project is intended to flow

When implemented, the command-line flow is:

```powershell
uef-gen check .\project.uef
uef-gen generate .\project.uef
uef-gen build .\project.uef
uef-gen flash .\project.uef
```

`check` should report syntax, type, unit, target, resource, timing, and
component-contract problems before any source is generated. `generate` should
resolve UETD manifests and templates into a buildable target project. `build`
should use the selected target's registered toolchain. `flash` should select
the board's registered deployment route and show the command/output. These
standalone operations are intended to be usable without NEXUS; their current
implementation status is documented in the
[uef-gen README](../../uef-gen/README.md#project-language-frontend-scaffold).

## Examples to explore

Start with the smallest example matching the runtime you want to understand:

- [Bare-metal loop controller](../../uef-gen/examples/basics/bare_metal_loop_controller.uef)
  shows a cooperative `loop`, state machine, and one motor-control subsystem.
- [Generic robot controller](../../uef-gen/examples/basics/generic_robot_controller.uef)
  adds two encoder/speed-control paths and typed subsystem data.
- [ESP32 differential-drive projects](../../uef-gen/examples/platforms/)
  show target and peripheral binding examples.
- [STM32F722 drone controller](../../uef-gen/examples/showcases/stm32f722_drone_controller.uef)
  is the closest language comparison to a larger hand-written flight
  controller.
- [Abyssal ROV](../../uef-gen/examples/showcases/abyssal_rov.uef) and the
  [ECHO acoustic observatory](../../uef-gen/examples/showcases/echo_acoustic_observatory.uef)
  show more specialized multi-sensor and signal-processing designs.
- [Tesla-coil multi-target system](../../uef-gen/examples/showcases/tesla_coil/tesla_coil_system.uefs)
  combines the H745 control node, G474 executor node, and MCU-side FPGA
  communications contract.

These files are design examples, not board-verified or flashable outputs.
Browse the [example index](../../uef-gen/examples/README.md) for the full list.

## Where the JSON files fit

UETD uses JSON for machine-readable catalogues and contracts. They are not
project configuration files that most `.uef` authors need to write.

```text
UETD/api.json
  └── catalogue index paths
      ├── registry/modules/registry.json
      │     └── module ID -> DSL_descriptor + manifest path
      │                                  └── authoring and generation contract
      ├── registry/types/registry.json
      ├── registry/units/registry.json
      ├── registry/umath/registry.json
      └── registry/targets/**/registry.json

uef-gen/language_registry/registry.json
  └── .uef grammar and language-owned syntax

uef-gen/system_language/registry.json
  └── .uefs grammar and system-only syntax
```

- **`UETD/api.json`** is the public map of catalogue locations. It tells tools
  where the indexes and some schemas live; it does not contain every record.
- **An index `registry.json`** lists records in that catalogue. The module
  index maps a canonical module ID to the name users write in the language
  (`DSL_descriptor`) and a path to its detailed manifest.
- **A module manifest** records source files and dependencies. Its optional
  `authoring` object describes the fields, ports, operations, constraints,
  readiness, and generation route that the language can use. A name without
  a complete contract can be discoverable while remaining unavailable for
  generation.
- **The type index** records UETD public C declarations, their source header,
  readiness, visibility, and the explicit DSL spelling for types UETD exposes
  directly. The built-in numeric spellings are `f32`, `f64`, `i8`/`i16`/
  `i32`/`i64`, `u8`/`u16`/`u32`/`u64`, and `q15`/`q16`/`q31`/`q63`, plus
  `bool`. Other C types are discoverable in the
  [type catalogue](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md#public-uetd-c-types-and-enums),
  but only a compatible UETD authoring contract can make one a project value.
  A `future_header` entry is visible to tooling but is not generation-ready.
- **The UMATH index** lists public declarations with UEF-PL call names,
  signatures, binding metadata, and readiness. Header presence alone does not
  make a type or math function usable in source; its language contract and
  readiness still apply.
- **The unit index** is the source of truth for unit dimensions, exact scales,
  and affine offsets.
- **The `uef-gen` language registries** define grammar and language-only
  constructs. They do not copy UETD's component catalogue.

The [registries and discovery guide](UEF_PL_REGISTRIES.md) gives the full
file-by-file explanation, loader search order, examples for adding a module,
and the rules for regenerating header-derived indexes. The
[generated catalogue reference](../reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md)
is useful when looking up registered names. The
[unit reference](../reference/UEF_PL_UNITS_REFERENCE.md) lists units by
dimension using mathematical notation.

### Locating a UETD checkout

`uef-gen` first checks `UETD_PATH`, then `uetd.path` in project configuration,
then documented relative locations next to the project. Set `UETD_PATH` when
you keep the framework and project in separate directories:

```powershell
$env:UETD_PATH = (Resolve-Path .\UETD).Path
```

The CLI follows the catalogue paths declared by `api.json`; it does not guess
module manifests from names or duplicate UETD's module list in Python. More
details are in [UEF-PL registries and discovery](UEF_PL_REGISTRIES.md).

## Editor support

The repository contains a VS Code extension and a native
[Visual Studio VSIX](../../uef-gen/editors/visual-studio/README.md).
Both associate `.uef` and `.uefs` with UEF-PL, use the shared TextMate grammar,
and can connect to the same `uef-gen-language-server` process. The language
server is implemented in Python with `pygls`; LSP is a general editor protocol,
so the server parses UEF-PL and does not provide Python-language features.

The extensions provide syntax coloring and connect to the language server for
registry-derived completion, hover, diagnostics, signature help, local and
UETD definition navigation, references, rename, document/workspace symbols,
formatting, folding, semantic tokens, and quick fixes. The `.uef`/`.uefs`
parser and semantic verifier are implemented; the standalone firmware emitter,
build, and flash path are not.

Install the optional language-server dependency from the `uef-gen` checkout:

```powershell
python -m pip install -e ".\uef-gen[language-server]"
```

In VS Code, enable `uef-project-language.languageServer.enabled` and configure
the server executable if it is not on `PATH`. The Visual Studio VSIX expects
`uef-gen-language-server` on `PATH` or an explicit executable in
`UEF_PL_LANGUAGE_SERVER`. Both clients discover UETD using the project-relative
layout or `UETD_PATH`; VS Code also accepts the `uefRoot` setting. Open the
`.uef` file directly or use **File > Open > Folder** in Visual Studio. The
VSIX targets the Visual Studio 17.x extension API surface for Visual Studio
2022 and 2026, but the package still needs to be built and checked in both
versions. See Microsoft's [Visual Studio LSP extension guide](https://learn.microsoft.com/en-us/visualstudio/extensibility/adding-an-lsp-extension?view=visualstudio)
and [extension compatibility guidance](https://learn.microsoft.com/en-us/visualstudio/extensibility/migration/extension-compatibility?view=visualstudio).

Registering `.uef` as a source-file type is provided by the VSIX.
Treating a `.uef` file itself as a full project in Solution
Explorer would require a project container/system; that can be added later if
Open Folder and a language service are not enough. Microsoft's guidance
recommends reusing its Visual Studio Project System for a custom project type
because implementing one from scratch is substantial work. The current
language is already usable as a standalone source document design without
NEXUS; a Visual Studio project system is not a requirement for the compiler.

## Next steps

1. Read the [technical guide](UEF_PROJECT_LANGUAGE_GUIDE.md) for types,
   units, expressions, custom code boundaries, composition, and runtime rules.
2. Read [registries and discovery](UEF_PL_REGISTRIES.md) before changing a
   JSON file.
3. Explore a simple `.uef` example and then one showcase.
4. Check the current [uef-gen implementation status](../../uef-gen/README.md)
   before relying on `.uef` validation, generation, build, or flash.
