# UPAL first-pass peripheral contract extensions

**State:** Phase 0 design contracts; implementation and target qualification remain blocked.

The public outline is `capability_contracts.h`. Target records own exact instances, pins, DMA, interrupts, clocks and errata. First-pass placement is separate from generation readiness.

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

## Wireless coprocessor transport

`radio_transport.h` now defines bounded command/response and asynchronous
byte-stream operations over an already configured SPI, I2C, or UART interface,
with optional poll, reset and ready callbacks. The selected module provider
owns bus-specific transfer details, AT/HCI/vendor framing, wake timing,
unsolicited event parsing and target resource binding. The transport
intentionally does not claim a radio profile. Integrated radios bypass this
host link and bind through a target SDK provider in UPROTO.

Promotion still requires exact module/firmware identity, physical layer
configuration, interrupt/ready timing, buffer/queue bounds, bus conflict
checks, security policy and board qualification.

## Packet network interface

`netif.h` now provides a fixed-buffer packet link boundary shared by Ethernet,
WiFi, 802.15.4 and other packet providers. It reports media, MTU, frame limits,
hardware address and declared DMA/checksum/timestamp capabilities. Providers
implement start/stop/poll/link/transmit/receive. An IP-offload-only coprocessor
uses a UPROTO service adapter and must not advertise raw packet access.

## Ethernet MAC boundary (active portable contract)

`include/upal/network/ethernet.h` layers PHY status and MAC lifecycle over the active
packet netif contract in `include/upal/network/netif.h`. Ethernet remains optional; IP,
sockets, and application protocols remain separate services. A target driver
still needs RX/TX descriptor ownership, cache-line
alignment and maintenance, frame-size/MTU limits, interrupt/deferred-work
split, link/PHY state, checksum and timestamp capabilities, buffer exhaustion
policy, reset recovery, PHY wiring/management interface and DMA facts. A driver
may advertise only capabilities supported by that exact part and board.


## Implementation and generation gate

Each extension requires its typed API, implementation, module/resource closure, bounds, supported target matrix and validation before generation is enabled.

## Source status and USB ownership

The main `upal/capability_contracts` record now owns unsupported source
definitions for all ten declarations in `capability_contracts.h`. These stubs
never invoke callbacks or change caller storage and remain generation-blocked.
The USB profile functions are compatibility development outlines. The new
`uproto/usb_device` class adapter owns CDC/HID class policy above the qualified
UPAL controller provider; implementation must select one class-stack route
rather than building independent duplicate class implementations.
