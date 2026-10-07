# UPROTO project wrappers

UPROTO provides two ways to use its protocols:

1. Firmware can call the generic C APIs under `include/uef/uproto/` directly.
2. A project generator can render the thin per-instance wrappers here from a
  ControlIR node indexed by `../../registry/control_ir/registry.json`.

The templates call UEF's protocol APIs; they are not copies of protocol
implementations and do not belong in `uef-gen`. They expose a small instance
hook that the application connects to its board's UPAL transport and task or
deferred-interrupt flow. The current protocol code still contains scaffolded
and unsupported target behavior, so generated files alone do not mean that a
board is ready to use.

The shared manifest lists every remaining driver, configuration, external
dialect/stack, safety-policy, and hardware-validation action. Those actions
are returned to the caller as warnings and in the generated project's
`manifest/manual_actions.json`. A consumer must complete applicable actions
before using the generated code with physical hardware.

| Protocol | Reusable UEF API | Generated wrapper | External/project work |
|---|---|---|---|
| DSHOT | Bounded timer/UART waveform API | Instance initialization hook | Timer/DMA/UART configuration, signal levels, timing measurement |
| CRSF | UART protocol and RC/telemetry API | Process and RC-read hooks | UART/DMA configuration and deferred processing |
| SBUS | UART decoder API | Process and frame-read hooks | Inverted 100 kbaud 8E2 receive path and failsafe policy |
| PPM | Fixed-memory interval decoder | Capture-interval and frame-read hooks | Timer capture, rollover conversion, signal polarity, and ISR/task synchronization |
| MAVLink | Transport stream API | Send/receive hooks | Vehicle dialect/message headers, IDs, allow-list, stream binding |
| UAVCAN / DroneCAN | UPAL CAN transport API | Node initialization and periodic spin hooks | Protocol version, compatible stack/dialect, static memory and CAN callbacks |

Do not add an entry to the manifest until its templates match the public UEF
API. Keep upstream-generated MAVLink and CAN dialect files in the selected
firmware project's dependency tree, with their own version and license
provenance.
