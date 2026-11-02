# Later UPAL peripheral extensions

The first-pass UART, timer capture, synchronization, USB and active network contracts are documented in [CAPABILITY_CONTRACTS.md](CAPABILITY_CONTRACTS.md). This file owns genuinely later radio scope.

## IEEE 802.15.4 radio

`ieee802154.h` defines the common 2.4 GHz channel profile, PAN/local address,
bounded MAC frame transfer, receive RSSI/LQI/timestamp, and polling/status
operations. The fixed API buffer excludes the two-byte FCS; the radio provider
generates/checks it. Thread/Zigbee security, key lifecycle, association and
network state remain with their selected stack. Other regional PHY/channel
profiles require explicit extensions rather than widening this profile
silently.
