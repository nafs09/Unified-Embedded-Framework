# Standalone UETD language examples

These examples are design inputs for UEF Project Language (UEF-PL) in `.uef`
and `.uefs` files. They illustrate how a project can declare its target, typed
components, physical interfaces, timing, subsystem contracts, state machines,
and failure behavior. They are not currently buildable or flashable: the
standalone parser, validator, generator, build route, and flash route are still
scaffold work. Numeric plant values, exact board bindings, and timing budgets
must be reviewed against a real design before use.

## Folder guide

- [`basics/`](basics/) contains a single-loop bare-metal controller and a
  generic differential-drive controller.
- [`platforms/`](platforms/) binds a small controller to one ESP32 board
  profile, then shows a source-derived FreeRTOS sorting robot with a larger
  set of board, sensor, actuator, and task bindings.
- [`showcases/`](showcases/) contains the source-derived drone and Tesla coil
  designs alongside the underwater vehicle, precision isolation bench, and
  networked acoustic observatory.

## Basics and platform examples

- [`bare_metal_loop_controller.uef`](basics/bare_metal_loop_controller.uef)
  is a single-core motor-speed controller using `runtime = BareMetal` and
  one explicit `loop`. It has no RTOS task or Arduino `loop()` dependency;
  uef-gen lowers its recurring work into the UETD bare-metal application loop.
- [`generic_robot_controller.uef`](basics/generic_robot_controller.uef) shows
  two encoder feedback loops, two H-bridge outputs, and one differential-drive
  subsystem.
- [`esp32_differential_drive.uef`](platforms/esp32_differential_drive.uef)
  expands the same controller with an ESP32 board pin map and fixed-priority
  task configuration. Its pin assignments are illustrative.

## ESP32 sorting robot example

- [`esp32_mecanum_sorter_freertos.uef`](platforms/esp32_mecanum_sorter_freertos.uef)
  represents the supplied five-axis sorting robot as direct FreeRTOS work:
  four mecanum drive steppers, an arm stepper, IR sensing, mission state,
  payload release, and two-core task placement. The source's `stepper` module
  is not in the current UETD module registry, so that identifier is the one
  expected editor warning until the module is registered.
## Showcases

- [`stm32f722_drone_controller.uef`](showcases/stm32f722_drone_controller.uef)
  is the source-derived flight controller. It represents receiver and
  flight-mode state, sensor services, typed navigation, camera/FPV paths,
  cascaded position/velocity/attitude/rate control, motor mixing, DShot
  telemetry, safety, and logging.
- [`tesla_coil/`](showcases/tesla_coil/) contains the H745 controller, G474
  executor, and `tesla_coil_system.uefs`. The controller uses an ENMPC policy
  with online NMPC-SQP fallback, then a separate continuous-to-finite-set MPC
  hybrid. FPGA declarations describe only the MCU-side protocol endpoint;
  FPGA RTL remains external.

- [`abyssal_rov.uef`](showcases/abyssal_rov.uef) combines attitude and
  navigation estimation, current-disturbance rejection, constrained thruster
  allocation, tether supervision, leak-triggered controlled ascent, and
  multi-rate sensing. Without a registered DVL or another underwater position
  source, horizontal position remains inertial dead reckoning between surface
  GNSS fixes; the sonar inputs provide short-range observations, not a claim of
  long-term absolute XY accuracy.
- [`precision_optical_bench.uef`](showcases/precision_optical_bench.uef)
  represents a six-axis isolation stage with measured base-motion rejection,
  modal estimation, constrained control, force allocation, current/stroke
  health checks, and hard output inhibit.
- [`echo_acoustic_observatory.uef`](showcases/echo_acoustic_observatory.uef)
  sketches a network of synchronized eight-hydrophone nodes. One H745 core
  acquires fixed-size ADC/DMA blocks; the other runs spectral and tonal
  analysis, bounded TDOA bearing estimation, a typed peer protocol, geometric
  ray fusion, and a source-motion filter. Surveyed node positions, array
  geometry, clock quality, and acoustic propagation speed remain explicit
  project inputs.

## Shared design assumptions

The H745 and G474 Tesla targets are `STM32H745XIH6` and `STM32G474VET6`.
Their exact custom-board pin maps, clocks, DMA/IRQ ownership, memory regions,
linker/startup configuration, and measured deadlines still need to be
specified before generation. The other examples also require real pin maps,
calibration, plant identification, and solver/timing qualification.

The language examples use fixed-width enums and records, typed ports,
bounded loops, channel-bound component inputs, and explicit state-machine
transitions. A channel uses `consumers = [task_a, task_b]`; each subscriber gets
an independent mailbox copy. A channel supplies its declared component input
to the one matching consumer task, so that task's `run` call leaves the
argument out. Task-trigger channels wake the listed task and acknowledge one
queued event per activation. Every FSM names one scheduled owner and its state
consumers. The owner evaluates against one activation snapshot and commits at
most one transition; consumers receive read-only state copies. A BareMetal
owner is its single cooperative loop, while an RTOS owner is one task using
UOS-synchronized ingress. These frontend contracts do not imply that an FSM
controls the scheduler or can stop other tasks synchronously. The `.uefs` example owns
cross-target contracts; each `.uef` project owns its local runtime and safety
authority.

The intended commands are `uef-gen check`, `generate`, `build`, and
`flash` on a `.uef` file. Those routes are still scaffolding; the examples have
only received a source-level consistency review, not parser, build, or hardware
verification. Start with the
[UEF-PL user handbook](../../docs/guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md).
For detailed source rules, see the
[technical guide](../../docs/guides/UEF_PROJECT_LANGUAGE_GUIDE.md) and
[language design](../../docs/design/PROJECT_LANGUAGE_DESIGN.md). The
[registry and discovery guide](../../docs/guides/UEF_PL_REGISTRIES.md) explains
the JSON source of truth, and the [generated catalogue](../../docs/reference/UEF_PROJECT_LANGUAGE_CATALOGUE.md)
is the lookup page for registered names. See the
[multi-target plan](../../docs/roadmap/MULTI_TARGET_AND_MULTICORE_PLAN.md) for
the system-level architecture work.
