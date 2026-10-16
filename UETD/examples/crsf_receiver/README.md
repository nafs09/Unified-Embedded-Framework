# CRSF receiver scaffold

This source scaffold outlines serial reception and CRSF frame decoding. Only
CRC-valid receiver snapshots are intended for publication; stale-link and
failsafe state must be derived from sample freshness.

UART settings, DMA/ring-buffer bounds, decoder scheduling, CRC handling, and
the application failsafe policy are not implemented. The example is not built
by default and is not ready to run on hardware.
