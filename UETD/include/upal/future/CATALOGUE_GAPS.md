# UPAL peripheral contract extensions

**State:** design backlog. These contracts are not selectable modules and do
not imply that a target has a corresponding peripheral driver.

This folder records operation-level holes that the existing UPAL family list
does not express. Target records continue to own exact instances, pins, DMA,
interrupts, clocks and errata. A future contract is enabled only when a target
adapter implements its capability and the profile says that capability is
available.

## UART framing and half-duplex direction

Add an advanced configuration layered over the existing UART resource:

- data bits, parity, stop bits, hardware flow-control mode, inversion where
  supported, oversampling/baud tolerance, and explicit unsupported-feature
  rejection;
- full/half duplex mode and direction-change policy;
- optional RS-485 DE pin ownership, active polarity, assert/deassert delays,
  and whether DE is released at DMA completion or UART transmission-complete;
- echo policy for half-duplex receive, idle/turnaround timeout, and collision or
  framing-error reporting;
- transfer completion includes exact byte count and one terminal result. DMA
  completion alone must not release half-duplex DE before the last stop bit.

The existing simple `upal_uart_init` remains a compatibility path with its
documented default framing. Advanced modes must be opt-in and fail closed when
the selected UART instance cannot implement them. DShot-UART mappings must
state line encoding, turnaround timing, and receive-window ownership rather
than treating a generic UART byte stream as equivalent to a timer waveform.

## Timer capture publication

The capture contract must give applications a result path, not just an IRQ
initializer. A capture event contains channel, edge, raw counter, overflow
epoch, extended tick count, optional receiver-local monotonic timestamp, and
overcapture/ambiguity flags. Configuration selects edge, digital filter,
counter clock, wrap policy and delivery mode (bounded event queue, callback, or
DMA block). It defines queue overflow behavior and maximum ISR work.

For period/frequency measurement, specify the number of edges and counter-wrap
ambiguity limit. For DMA capture, declare buffer ownership, cache alignment,
half/full transfer publication and stop behavior. Captures spanning timer
overflow must either be extended unambiguously or marked invalid; silently
wrapping a `u16` delta is not acceptable.

## Synchronized acquisition and output update

Add an optional target-resolved synchronization group connecting timer or
HRTIM master events to ADC trigger cohorts, DMA capture, and output preload
commit. The group declares:

- master clock/timer and event edge;
- each ADC instance/channel rank, trigger source, sample phase/offset and
  oversampling cohort;
- DMA buffer ownership, frame length, timestamp source and overrun policy;
- PWM/HRTIM preload commit point, dead-time/interlock ownership and safe output
  state if any participant fails to arm;
- start ordering, atomic start boundary, stop ordering and partial-start
  rollback.

Generation must reject unsupported trigger routes, incompatible timer clocks,
or cohorts with no common deterministic boundary. This is a board capability,
not a portable assumption that all ADCs and timers can synchronize.

## USB profiles

Keep the hardware/endpoint driver separate from USB class profiles. The first
profiles to define are CDC ACM for wired diagnostics and HID for bounded
control/input reports. Contracts include descriptor ownership, endpoint and
packet budgets, polling/transfer interval, DMA/cache ownership, control-request
handling, suspend/resume, disconnect/reconnect, and bounded TX backpressure.
The host-visible VID/PID and strings come from project/product metadata; they
must not be invented by a generic peripheral template.

## Ethernet MAC boundary

Ethernet belongs behind an optional UPAL MAC/PHY boundary; IP, sockets, and
application protocols remain separate services. The MAC contract needs RX/TX
descriptor ownership, cache-line alignment and maintenance, frame-size/MTU
limits, interrupt/deferred-work split, link/PHY state, checksum offload
capabilities, timestamping capability, buffer exhaustion policy and reset
recovery. The target profile supplies PHY wiring/management interface and DMA
facts. A driver may advertise only capabilities actually supported by that
part and board.

## Promotion gate

Each extension needs a typed public API, manifest, target capability mapping,
resource-conflict diagnostics, compile-time bounds, a backend support matrix,
and host plus target evidence before it leaves `future`. The initial promotion
set should be chosen by project demand; USB, Ethernet and synchronized ADC/PWM
must remain optional.
