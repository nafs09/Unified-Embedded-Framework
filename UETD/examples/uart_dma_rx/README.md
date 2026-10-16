# UART DMA receive scaffold

This source scaffold outlines a UART receive path using DMA and an IDLE event.
The intended flow keeps byte consumption outside the hard interrupt and
detects ring-buffer overrun and framing errors.

Target-specific UART settings, DMA/cache coherency, IRQ priority, buffer
ownership, and task handoff are not implemented. The example is not built by
default and is not ready to run on hardware.
