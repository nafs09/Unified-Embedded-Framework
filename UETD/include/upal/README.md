# UPAL peripheral abstraction

UPAL declares portable peripheral access APIs so application and middleware code can use hardware capabilities through a consistent boundary.

## Contents

- <code>upal.h</code> is the main module umbrella. Related APIs share `analog/`, `can/`, `compute/`, `display/`, `network/`, `serial/`, `storage/`, `timing/` and `watchdog/` folders. `can/` contains classic CAN and CAN FD; the other groups collect their related peripheral capabilities.
- First-pass capability contracts live in the module root. The later IEEE 802.15.4 outline is colocated with network transports and remains generation-blocked. Radio bus transport, packet netif, Ethernet MAC/PHY and display are active portable provider contracts; their hardware drivers and board profiles remain target-specific.
- Concrete implementations and target availability are maintained separately under <code>src/</code> and the target/profile configuration.

The [MIPI display-link design](../../docs/design/complete-system/MIPI_DISPLAY_LINK.md)
defines first-pass DSI/DSI-2 host and DCS/panel integration over compatible
D-PHY or C-PHY hardware. The generic display facade does not implement a PHY.

## Files by scope

Each row describes one header. The file link opens its source; the overview summarizes its purpose, and key declarations list the principal API symbols.

Type names may be defined in the linked header or supplied by an included dependency. Status guide: **Public contract** describes an API surface but does not guarantee every target has an implementation; **Future / planned** is a design or backlog header; **Fail-closed scaffold** declares development operations that are not implemented; **Target-family scaffold** is not a part or board profile; **Internal helper** is not an application-facing include; **Build configuration** supplies compile-time selection and capability macros.

| Scope | File | Overview | Key declarations and details | Status |
|---|---|---|---|---|
| analog | [adc.h](analog/adc.h) | Initialize the ADC. | Functions: <code>upal_adc_calibrate</code>, <code>upal_adc_dma_handler</code>, <code>upal_adc_init</code>, <code>upal_adc_irq_handler</code>, <code>upal_adc_read_blocking</code>, <code>upal_adc_start</code>, <code>upal_adc_stop</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_adc_channel_cfg_t</code>, <code>upal_adc_hw_t</code>, <code>upal_adc_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code> | Public contract |
| can | [can.h](can/can.h) | Initialize the CAN interface. | Functions: <code>upal_can_add_filter</code>, <code>upal_can_err_irq_handler</code>, <code>upal_can_init</code>, <code>upal_can_rx_irq_handler</code>, <code>upal_can_transmit</code>, <code>upal_can_transmit_async</code>, <code>upal_can_tx_irq_handler</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_can_cfg_t</code>, <code>upal_can_frame_t</code>, <code>upal_can_hw_t</code>, <code>upal_can_rx_cb_t</code>, <code>upal_can_t</code> | Public contract |
| analog | [comp.h](analog/comp.h) | Initialize the hardware comparator. | Functions: <code>upal_comp_disable</code>, <code>upal_comp_enable</code>, <code>upal_comp_init</code>, <code>upal_comp_irq_handler</code>, <code>upal_comp_read</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_comp_cb_t</code>, <code>upal_comp_hw_t</code>, <code>upal_comp_inm_t</code>, <code>upal_comp_t</code> | Public contract |
| compute | [cordic.h](compute/cordic.h) | Initialize the CORDIC. | Functions: <code>upal_cordic_atan2_q31</code>, <code>upal_cordic_batch_sincos_q31</code>, <code>upal_cordic_init</code>, <code>upal_cordic_modulus_q31</code>, <code>upal_cordic_sincos_q31</code>, <code>upal_cordic_sqrt_q31</code>; Types used or declared: <code>uetd_q31_t</code>, <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>upal_dma_callback_t</code> | Public contract |
| analog | [dac.h](analog/dac.h) | Initialize the DAC. | Functions: <code>upal_dac_dma_start</code>, <code>upal_dac_dma_stop</code>, <code>upal_dac_init</code>, <code>upal_dac_write</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_dac_hw_t</code>, <code>upal_dac_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code> | Public contract |
| display | [display.h](display/display.h) | Generic first-pass display/panel provider for command, RGB/MIPI-DSI scanout, and coprocessor routes. | Validated regions/stride/format/alignment, one bounded pixel loan, asynchronous completion/cancel, brightness and sleep. Exact panel/provider qualification remains required. | Provider interface |
| module root | [dma.h](dma.h) | DMA is a first-class resource allocated by this abstraction, not hidden in peripheral drivers. | Functions: <code>upal_dma_abort</code>, <code>upal_dma_init</code>, <code>upal_dma_irq_handler</code>, <code>upal_dma_remaining</code>, <code>upal_dma_start</code>, <code>upal_dma_wait</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_dir_t</code>, <code>upal_dma_event_t</code>, <code>upal_dma_hw_t</code>, <code>upal_dma_t</code>, <code>upal_dma_width_t</code>, <code>upal_dma_xfer_t</code> | Public contract |
| network | [ethernet.h](network/ethernet.h) | Ethernet MAC/PHY profile layered over the generic UPAL netif. | Functions: <code>upal_ethernet_bind</code>, <code>upal_ethernet_get_status</code>, <code>upal_ethernet_poll</code>, <code>upal_ethernet_start</code>, <code>upal_ethernet_stop</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_ethernet_ops_t</code>, <code>upal_ethernet_speed_t</code>, <code>upal_ethernet_status_t</code>, <code>upal_ethernet_t</code>, <code>upal_netif_t</code> | Public contract |
| can | [fdcan_fd.h](can/fdcan_fd.h) | Phase 1 CAN-FD frame contract, separate from the classic CAN API. | Functions: <code>upal_fdcan_fd_deinit</code>, <code>upal_fdcan_fd_init</code>, <code>upal_fdcan_fd_receive</code>, <code>upal_fdcan_fd_transmit</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_fdcan_fd_config_t</code>, <code>upal_fdcan_fd_frame_t</code>, <code>upal_fdcan_fd_t</code>; Defines: <code>UPAL_FDCAN_FD_MAX_DATA_BYTES</code> | Public contract |
| storage | [flash.h](storage/flash.h) | Enable write and erase access to internal flash. | Functions: <code>upal_flash_erase_page</code>, <code>upal_flash_lock</code>, <code>upal_flash_unlock</code>, <code>upal_flash_verify</code>, <code>upal_flash_write</code>, <code>upal_flash_write_dword</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code> | Public contract |
| compute | [fmac.h](compute/fmac.h) | Initialize the FMAC filter with the supplied coefficients and buffer sizes. | Functions: <code>upal_fmac_init</code>, <code>upal_fmac_irq_handler</code>, <code>upal_fmac_read</code>, <code>upal_fmac_write</code>; Types used or declared: <code>uetd_q15_t</code>, <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>upal_fmac_cfg_t</code>, <code>upal_fmac_func_t</code> | Public contract |
| module root | [capability_contracts.h](capability_contracts.h) | Design-only UART modes, timer capture, synchronized I/O groups, and USB CDC/HID contracts. | Uses `uetd_i32_t` for signed phase offsets. Ethernet is owned by the active `network/ethernet.h` and `network/netif.h` provider contracts. | First-pass contract (generation blocked) |
| network | [ieee802154.h](network/ieee802154.h) | Fixed-frame IEEE 802.15.4 MAC/radio provider contract. | Functions: <code>upal_ieee802154_get_status</code>, <code>upal_ieee802154_init</code>, <code>upal_ieee802154_poll</code>, <code>upal_ieee802154_receive</code>, <code>upal_ieee802154_set_channel</code>, <code>upal_ieee802154_start</code>, <code>upal_ieee802154_stop</code>, <code>upal_ieee802154_transmit</code>; Types used or declared: <code>uetd_i8_t</code>, <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>upal_ieee802154_address_mode_t</code>, <code>upal_ieee802154_address_t</code>, <code>upal_ieee802154_config_t</code>, <code>upal_ieee802154_frame_t</code>, <code>upal_ieee802154_ops_t</code>, <code>upal_ieee802154_status_t</code>, <code>upal_ieee802154_t</code>; Defines: <code>UPAL_IEEE802154_EXTENDED_ADDRESS_BYTES</code>, <code>UPAL_IEEE802154_FRAME_MAX</code>, <code>UPAL_IEEE802154_PSDU_MAX</code> | Future / planned |
| timing | [hrtim.h](timing/hrtim.h) | Calibrate the HRTIM using current hardware conditions. | Functions: <code>upal_hrtim_calibrate</code>, <code>upal_hrtim_fault_init</code>, <code>upal_hrtim_fault_irq_handler</code>, <code>upal_hrtim_master_irq_handler</code>, <code>upal_hrtim_preload_enable</code>, <code>upal_hrtim_pwm_init</code>, <code>upal_hrtim_set_duty</code>, <code>upal_hrtim_timer_irq_handler</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_t</code>, <code>upal_hrtim_hw_t</code>, <code>upal_hrtim_pwm_cfg_t</code>, <code>upal_hrtim_t</code>, <code>upal_hrtim_timer_t</code> | Public contract |
| serial | [i2c.h](serial/i2c.h) | Initialize the I2C interface. | Functions: <code>upal_i2c_er_irq_handler</code>, <code>upal_i2c_ev_irq_handler</code>, <code>upal_i2c_init</code>, <code>upal_i2c_read_blocking</code>, <code>upal_i2c_read_dma</code>, <code>upal_i2c_write_blocking</code>, <code>upal_i2c_write_dma</code>, <code>upal_i2c_write_read_dma</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code>, <code>upal_i2c_hw_t</code>, <code>upal_i2c_speed_t</code>, <code>upal_i2c_t</code> | Public contract |
| watchdog | [iwdg.h](watchdog/iwdg.h) | Initialize the IWDG. | Functions: <code>upal_iwdg_init</code>, <code>upal_iwdg_refresh</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code> | Public contract |
| network | [netif.h](network/netif.h) | Fixed-buffer packet interface between a link provider and a network stack. | Functions: <code>upal_netif_get_link</code>, <code>upal_netif_init</code>, <code>upal_netif_poll</code>, <code>upal_netif_receive</code>, <code>upal_netif_start</code>, <code>upal_netif_stop</code>, <code>upal_netif_transmit</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_netif_info_t</code>, <code>upal_netif_link_t</code>, <code>upal_netif_media_t</code>, <code>upal_netif_ops_t</code>, <code>upal_netif_t</code>; Defines: <code>UPAL_NETIF_ADDRESS_MAX</code> | Public contract |
| analog | [opamp.h](analog/opamp.h) | Initialize the operational amplifier with its hardware configuration. | Functions: <code>upal_opamp_calibrate</code>, <code>upal_opamp_disable</code>, <code>upal_opamp_enable</code>, <code>upal_opamp_init</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_opamp_hw_t</code>, <code>upal_opamp_mode_t</code>, <code>upal_opamp_t</code> | Public contract |
| storage | [qspi.h](storage/qspi.h) | Initialize the QSPI interface. | Functions: <code>upal_qspi_enter_memory_mapped</code>, <code>upal_qspi_erase_sector</code>, <code>upal_qspi_exit_memory_mapped</code>, <code>upal_qspi_init</code>, <code>upal_qspi_read</code>, <code>upal_qspi_write_page</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_t</code>, <code>upal_qspi_hw_t</code>, <code>upal_qspi_t</code> | Public contract |
| network | [radio_transport.h](network/radio_transport.h) | Bounded host transport for external radio/network coprocessors. | Functions: <code>upal_radio_transport_exchange</code>, <code>upal_radio_transport_init</code>, <code>upal_radio_transport_is_ready</code>, <code>upal_radio_transport_poll</code>, <code>upal_radio_transport_receive</code>, <code>upal_radio_transport_send</code>, <code>upal_radio_transport_set_reset</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_radio_bus_t</code>, <code>upal_radio_transport_config_t</code>, <code>upal_radio_transport_t</code> | Public contract |
| timing | [rtc.h](timing/rtc.h) | Initialize the real-time clock. | Functions: <code>upal_rtc_get_date</code>, <code>upal_rtc_get_time</code>, <code>upal_rtc_get_unix</code>, <code>upal_rtc_init</code>, <code>upal_rtc_set_date</code>, <code>upal_rtc_set_time</code>, <code>upal_rtc_set_unix</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>upal_rtc_date_t</code>, <code>upal_rtc_time_t</code> | Public contract |
| storage | [sdmmc.h](storage/sdmmc.h) | Declares the Phase 1 SD/MMC block-transfer boundary. | Functions: <code>upal_sdmmc_card_info</code>, <code>upal_sdmmc_card_present</code>, <code>upal_sdmmc_dma_handler</code>, <code>upal_sdmmc_erase</code>, <code>upal_sdmmc_init</code>, <code>upal_sdmmc_irq_handler</code>, <code>upal_sdmmc_read_blocking</code>, <code>upal_sdmmc_read_blocks</code>, <code>upal_sdmmc_write_blocking</code>, <code>upal_sdmmc_write_blocks</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code>, <code>upal_sdmmc_card_info_t</code>, <code>upal_sdmmc_card_type_t</code>, <code>upal_sdmmc_hw_t</code>, <code>upal_sdmmc_t</code>, <code>upal_sdmmc_width_t</code>; Defines: <code>UPAL_SDMMC_BLOCK_BYTES</code> | Public contract |
| serial | [spi.h](serial/spi.h) | Initialize the SPI interface. | Functions: <code>upal_spi_dma_handler</code>, <code>upal_spi_init</code>, <code>upal_spi_transfer_blocking</code>, <code>upal_spi_transfer_dma</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code>, <code>upal_spi_hw_t</code>, <code>upal_spi_mode_t</code>, <code>upal_spi_t</code> | Public contract |
| timing | [timer.h](timing/timer.h) | Initialize the timer timebase with the requested update period. | Functions: <code>upal_timer_capture_init</code>, <code>upal_timer_cc_irq_handler</code>, <code>upal_timer_dma_output_start</code>, <code>upal_timer_init_timebase</code>, <code>upal_timer_pwm_init</code>, <code>upal_timer_pwm_set_duty</code>, <code>upal_timer_start</code>, <code>upal_timer_stop</code>, <code>upal_timer_up_irq_handler</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code>, <code>upal_timer_hw_t</code>, <code>upal_timer_t</code> | Public contract |
| serial | [uart.h](serial/uart.h) | Initialize the UART interface. | Functions: <code>upal_uart_dma_rx_handler</code>, <code>upal_uart_dma_tx_handler</code>, <code>upal_uart_init</code>, <code>upal_uart_irq_handler</code>, <code>upal_uart_rx_available</code>, <code>upal_uart_rx_read</code>, <code>upal_uart_tx_blocking</code>, <code>upal_uart_tx_dma</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>upal_dma_callback_t</code>, <code>upal_dma_t</code>, <code>upal_uart_hw_t</code>, <code>upal_uart_t</code> | Public contract |
| module root | [upal.h](upal.h) | Umbrella for the public UPAL peripheral-access APIs. | Includes: <code>upal/analog/adc.h</code>, <code>upal/can/can.h</code>, <code>upal/analog/comp.h</code>, <code>upal/compute/cordic.h</code>, <code>upal/analog/dac.h</code>, <code>upal/dma.h</code>, <code>upal/display/display.h</code>, <code>upal/can/fdcan_fd.h</code>, <code>upal/storage/flash.h</code>, <code>upal/compute/fmac.h</code>, <code>upal/timing/hrtim.h</code>, <code>upal/serial/i2c.h</code>, <code>upal/watchdog/iwdg.h</code>, <code>upal/network/ethernet.h</code>, <code>upal/network/netif.h</code>, <code>upal/analog/opamp.h</code>, <code>upal/storage/qspi.h</code>, <code>upal/network/radio_transport.h</code>, <code>upal/timing/rtc.h</code>, <code>upal/storage/sdmmc.h</code>, <code>upal/serial/spi.h</code>, <code>upal/timing/timer.h</code>, <code>upal/serial/uart.h</code>, <code>upal/serial/usb.h</code>, <code>upal/watchdog/wwdg.h</code> | Public contract |
| serial | [usb.h](serial/usb.h) | Reserved public boundary for the planned USB OTG transport module. | Includes: <code>ucore/status.h</code>, <code>ucore/types.h</code>, <code>uhal/target.h</code> | Public contract |
| watchdog | [wwdg.h](watchdog/wwdg.h) | Initialize the WWDG. | Functions: <code>upal_wwdg_early_warning_hook</code>, <code>upal_wwdg_init</code>, <code>upal_wwdg_irq_handler</code>, <code>upal_wwdg_refresh</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u32_t</code> | Public contract |

## First-pass source scaffolds

All rows below are draft interfaces with source definitions returning
`UETD_NOT_SUPPORTED`. Generation remains blocked. Include them explicitly;
the established module umbrellas do not make these stubs usable.

| Header | Contract | Operations |
|---|---|---:|
| [transaction.h](transaction.h) | Common asynchronous I/O token and ownership contract | 4 |
| [display/dsi_host.h](display/dsi_host.h) | MIPI DSI host with explicit D-PHY/C-PHY profile | 7 |
| [display/display_panel.h](display/display_panel.h) | Bounded panel profile and initialization sequencing | 6 |
| [display/touch.h](display/touch.h) | Coherent bounded touchscreen contact snapshots | 4 |
| [power.h](power.h) | Power-state admission, wake and battery facts | 5 |
| [storage/block_device.h](storage/block_device.h) | Generation-tagged block storage and durability | 5 |
| [capability_contracts.h](capability_contracts.h) | Advanced peripheral compatibility contracts | 10 |

## Header file structure

~~~text
upal/
├── analog/
│   ├── adc.h
│   ├── comp.h
│   ├── dac.h
│   └── opamp.h
├── can/
│   ├── can.h
│   └── fdcan_fd.h
├── compute/
│   ├── cordic.h
│   └── fmac.h
├── display/
│   ├── display.h
│   ├── display_panel.h
│   ├── dsi_host.h
│   └── touch.h
├── network/
│   ├── ethernet.h
│   ├── ieee802154.h
│   ├── netif.h
│   └── radio_transport.h
├── serial/
│   ├── i2c.h
│   ├── spi.h
│   ├── uart.h
│   └── usb.h
├── storage/
│   ├── block_device.h
│   ├── flash.h
│   ├── qspi.h
│   └── sdmmc.h
├── timing/
│   ├── hrtim.h
│   ├── rtc.h
│   └── timer.h
├── watchdog/
│   ├── iwdg.h
│   └── wwdg.h
├── capability_contracts.h
├── CAPABILITY_CONTRACTS.md
├── CATALOGUE_GAPS.md
├── dma.h
├── power.h
├── README.md
├── transaction.h
└── upal.h
~~~
