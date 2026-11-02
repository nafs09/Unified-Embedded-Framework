# UMID middleware and device-facing services

UMID contains reusable services that sit between application logic and peripheral-level APIs.

## Contents

- <code>umid.h</code> is the module umbrella; the root headers expose health monitoring, transport-neutral logging, ring buffers, and FatFs integration. <code>logging/log_uart.h</code> is the optional UPAL UART sink adapter.
- <code>drivers/</code> groups sensor and measurement-device interfaces such as current, encoder, FPGA, GNSS, IMU, magnetometer, pressure, range, temperature, and voltage.
- `data/` groups storage, settings, calibration and process-image contracts; `ui/` groups graphics and input. Their first-pass C entry points remain fail closed. These services are transport-neutral; applications adapt their records to UPROTO protocols without making UMID depend on WiFi, Bluetooth, or IP.

## Files by scope

Each row describes one header. The file link opens its source; the overview summarizes its purpose, and key declarations list the principal API symbols.

Type names may be defined in the linked header or supplied by an included dependency. Status guide: **Public contract** describes an API surface but does not guarantee every target has an implementation; **Future / planned** is a design or backlog header; **Fail-closed scaffold** declares development operations that are not implemented; **Target-family scaffold** is not a part or board profile; **Internal helper** is not an application-facing include; **Build configuration** supplies compile-time selection and capability macros.

| Scope | File | Overview | Key declarations and details | Status |
|---|---|---|---|---|
| drivers | [current.h](drivers/current.h) | Initialize the current sensor adapter. | Functions: <code>umid_current_calibrate_zero</code>, <code>umid_current_init</code>, <code>umid_current_read_a</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uetd_u8_t</code>, <code>umid_current_t</code>, <code>upal_adc_t</code> | Public contract |
| drivers | [encoder.h](drivers/encoder.h) | Initialize the encoder measurement adapter. | Functions: <code>umid_encoder_count</code>, <code>umid_encoder_init</code>, <code>umid_encoder_position_rad</code>, <code>umid_encoder_reset</code>, <code>umid_encoder_velocity_rps</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_i32_t</code>, <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>umid_encoder_t</code>, <code>umid_encoder_type_t</code>, <code>upal_timer_t</code> | Public contract |
| drivers | [fpga.h](drivers/fpga.h) | MCU-side endpoint state for a project-declared FPGA protocol. | Functions: <code>umid_fpga_has_valid_data</code>, <code>umid_fpga_init</code>, <code>umid_fpga_invalidate_data</code>, <code>umid_fpga_is_connected</code>, <code>umid_fpga_is_data_fresh</code>, <code>umid_fpga_last_valid_message_time</code>, <code>umid_fpga_note_valid_message</code>, <code>umid_fpga_set_link_qualified</code>; Types used or declared: <code>uetd_dur_us_t</code>, <code>uetd_status_t</code>, <code>uetd_time_us_t</code>, <code>umid_fpga_t</code> | Public contract |
| drivers | [gnss.h](drivers/gnss.h) | Initialize the GNSS receiver. | Functions: <code>umid_gnss_has_fix</code>, <code>umid_gnss_init</code>, <code>umid_gnss_process</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_f64_t</code>, <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>umid_gnss_data_t</code>, <code>umid_gnss_t</code>, <code>upal_uart_t</code> | Public contract |
| drivers | [imu.h](drivers/imu.h) | Initialize the IMU. | Functions: <code>umid_imu_init</code>, <code>umid_imu_read_blocking</code>, <code>umid_imu_start</code>, <code>umid_imu_stop</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>uhal_gpio_pin_t</code>, <code>umid_imu_bus_t</code>, <code>umid_imu_data_t</code>, <code>umid_imu_t</code>, <code>upal_i2c_t</code>, <code>upal_spi_t</code> | Public contract |
| drivers | [magnetometer.h](drivers/magnetometer.h) | Initialize the magnetometer sensor adapter. | Functions: <code>umid_magnetometer_init</code>, <code>umid_magnetometer_read</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>umid_mag_data_t</code>, <code>umid_magnetometer_t</code>, <code>upal_i2c_t</code> | Public contract |
| drivers | [pressure.h](drivers/pressure.h) | Initialize the pressure sensor adapter. | Functions: <code>umid_pressure_init</code>, <code>umid_pressure_read</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uhal_gpio_pin_t</code>, <code>umid_pressure_data_t</code>, <code>umid_pressure_t</code>, <code>upal_spi_t</code> | Public contract |
| drivers | [range.h](drivers/range.h) | Initialize the range sensor adapter. | Functions: <code>umid_range_init</code>, <code>umid_range_read</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uetd_u64_t</code>, <code>umid_range_t</code>, <code>umid_range_type_t</code> | Public contract |
| drivers | [temperature.h](drivers/temperature.h) | Initialize the temperature sensor adapter. | Functions: <code>umid_temperature_init</code>, <code>umid_temperature_read_degC</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uhal_gpio_pin_t</code>, <code>umid_temp_type_t</code>, <code>umid_temperature_t</code>, <code>upal_spi_t</code> | Public contract |
| drivers | [voltage.h](drivers/voltage.h) | Initialize the voltage sensor adapter. | Functions: <code>umid_voltage_init</code>, <code>umid_voltage_read_v</code>; Types used or declared: <code>uetd_f32_t</code>, <code>uetd_status_t</code>, <code>uetd_u8_t</code>, <code>umid_voltage_t</code>, <code>upal_adc_t</code> | Public contract |
| module root | [fatfs.h](fatfs.h) | Declares the optional FatFS diskio binding for a UPAL SDMMC instance. | Functions: <code>umid_fatfs_register_sdmmc</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u8_t</code>, <code>upal_sdmmc_t</code> | Public contract |
| logging | [blackbox.h](logging/blackbox.h) | Deferred UMID binary acquisition logging contract. | Functions: <code>umid_blackbox_append</code>, <code>umid_blackbox_flush</code>, <code>umid_blackbox_get_stats</code>, <code>umid_blackbox_start</code>, <code>umid_blackbox_stop</code>, <code>umid_blackbox_validate</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>umid_blackbox_backend_t</code>, <code>umid_blackbox_channel_t</code>, <code>umid_blackbox_config_t</code>, <code>umid_blackbox_overflow_policy_t</code>, <code>umid_blackbox_record_t</code>, <code>umid_blackbox_stats_t</code>, <code>umid_blackbox_t</code> | First-pass contract (generation blocked) |
| data | [calibration.h](data/calibration.h) | Deferred UMID calibration record contract. | Functions: <code>umid_calibration_store_load</code>, <code>umid_calibration_store_save</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>umid_calibration_record_t</code>, <code>umid_store_t</code>; Defines: <code>UMID_CAL_BOARD_ID_CAPACITY</code>, <code>UMID_CAL_DRIVER_ID_CAPACITY</code>, <code>UMID_CAL_METHOD_ID_CAPACITY</code>, <code>UMID_CAL_SENSOR_ID_CAPACITY</code>, <code>UMID_CAL_UNITS_ID_CAPACITY</code> | First-pass contract (generation blocked) |
| data | [data_services.h](data/data_services.h) | Compatibility umbrella for the deferred UMID data-service contracts. | Includes: <code>umid/data/storage.h</code>, <code>umid/data/calibration.h</code>, <code>umid/logging/blackbox.h</code>, <code>umid/data/process_image.h</code> | First-pass contract (generation blocked) |
| data | [process_image.h](data/process_image.h) | Deferred UMID typed process-image contract. | Functions: <code>umid_process_image_begin_scan</code>, <code>umid_process_image_commit_scan</code>, <code>umid_process_image_init</code>, <code>umid_process_image_snapshot</code>, <code>umid_process_image_stage_point</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>umid_process_authority_t</code>, <code>umid_process_command_semantics_t</code>, <code>umid_process_direction_t</code>, <code>umid_process_image_config_t</code>, <code>umid_process_image_t</code>, <code>umid_process_point_schema_t</code>, <code>umid_process_point_t</code>, <code>umid_process_quality_t</code>, <code>umid_process_scan_t</code> | First-pass contract (generation blocked) |
| data | [storage.h](data/storage.h) | Deferred UMID transactional storage contract. | Functions: <code>umid_store_erase</code>, <code>umid_store_init</code>, <code>umid_store_read</code>, <code>umid_store_write_atomic</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>umid_store_backend_t</code>, <code>umid_store_key_t</code>, <code>umid_store_record_kind_t</code>, <code>umid_store_record_t</code>, <code>umid_store_t</code>; Defines: <code>UMID_STORE_DEVICE_ID_CAPACITY</code>, <code>UMID_STORE_ENCODING_ID_CAPACITY</code>, <code>UMID_STORE_KEY_CAPACITY</code>, <code>UMID_STORE_OWNER_ID_CAPACITY</code>, <code>UMID_STORE_SCHEMA_ID_CAPACITY</code>, <code>UMID_STORE_TARGET_ID_CAPACITY</code> | First-pass contract (generation blocked) |
| module root | [health.h](health.h) | Register a subsystem for health monitoring. | Functions: <code>umid_health_count</code>, <code>umid_health_entry</code>, <code>umid_health_entry_copy</code>, <code>umid_health_entry_is_fresh</code>, <code>umid_health_register</code>, <code>umid_health_report</code>, <code>umid_health_system_status</code>; Types used or declared: <code>uetd_u32_t</code>, <code>uetd_u64_t</code>, <code>uetd_u8_t</code>, <code>umid_health_entry_t</code>, <code>umid_health_status_t</code>; Defines: <code>UMID_HEALTH_INVALID_ID</code>, <code>UMID_HEALTH_MAX_ENTRIES</code> | Public contract |
| logging | [log.h](logging/log.h) | Initialize the bounded logging queue. | Functions: <code>umid_log</code>, <code>umid_log_drain</code>, <code>umid_log_dropped</code>, <code>umid_log_hex</code>, <code>umid_log_init</code>, <code>umid_log_queued</code>; Types used or declared: <code>uetd_status_t</code>, <code>uetd_u16_t</code>, <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>umid_log_level_t</code>, <code>umid_log_sink_t</code>; Defines: <code>UMID_LOGD</code>, <code>UMID_LOGE</code>, <code>UMID_LOGI</code>, <code>UMID_LOGW</code>, <code>UMID_LOG_QUEUE_CAPACITY</code>, <code>UMID_LOG_RECORD_BYTES</code> | Public contract |
| logging | [log_uart.h](logging/log_uart.h) | Optional UART output adapter for the transport-neutral UMID logger. | Functions: <code>umid_log_uart_init</code>; Types used or declared: <code>umid_log_level_t</code>, <code>upal_uart_t</code> | Public contract |
| messaging | [ringbuf.h](messaging/ringbuf.h) | Fixed-capacity, power-of-two byte ring buffer with an SPSC contract. | Functions: <code>umid_ringbuf_available</code>, <code>umid_ringbuf_flush</code>, <code>umid_ringbuf_free</code>, <code>umid_ringbuf_init</code>, <code>umid_ringbuf_peek</code>, <code>umid_ringbuf_read</code>, <code>umid_ringbuf_write</code>; Types used or declared: <code>uetd_u32_t</code>, <code>uetd_u8_t</code>, <code>umid_ringbuf_t</code> | Public contract |
| module root | [umid.h](umid.h) | Umbrella for the UMID health, logging, buffering, storage, and device-facing service APIs. | Includes: <code>umid/drivers/current.h</code>, <code>umid/drivers/encoder.h</code>, <code>umid/drivers/fpga.h</code>, <code>umid/drivers/gnss.h</code>, <code>umid/drivers/imu.h</code>, <code>umid/drivers/magnetometer.h</code>, <code>umid/drivers/pressure.h</code>, <code>umid/drivers/range.h</code>, <code>umid/drivers/temperature.h</code>, <code>umid/drivers/voltage.h</code>, <code>umid/health.h</code>, <code>umid/logging/log.h</code>, <code>umid/messaging/ringbuf.h</code>, <code>umid/fatfs.h</code> | Public contract |

## First-pass source scaffolds

All rows below are draft interfaces with source definitions returning
`UETD_NOT_SUPPORTED`. Generation remains blocked. Include them explicitly;
the established module umbrellas do not make these stubs usable.

| Header | Contract | Operations |
|---|---|---:|
| [messaging/buffer_pool.h](messaging/buffer_pool.h) | Fixed aligned blocks with generation-tagged ownership | 4 |
| [messaging/mailbox.h](messaging/mailbox.h) | Coherent copied latest-value publication | 3 |
| [messaging/event_bus.h](messaging/event_bus.h) | Bounded typed topic and subscriber delivery | 4 |
| [sensor.h](sensor.h) | Shared sample quality, clock and calibration envelopes | 4 |
| [data/settings.h](data/settings.h) | Generation-checked staged configuration transactions | 5 |
| [ui/input.h](ui/input.h) | Bounded touch, button and encoder events | 5 |
| [ui/graphics.h](ui/graphics.h) | GUI integration, assets and display flush ownership | 5 |

## Header file structure

~~~text
umid/
├── data/
│   ├── calibration.h
│   ├── data_services.h
│   ├── process_image.h
│   ├── settings.h
│   └── storage.h
├── drivers/
│   ├── current.h
│   ├── encoder.h
│   ├── fpga.h
│   ├── gnss.h
│   ├── imu.h
│   ├── magnetometer.h
│   ├── pressure.h
│   ├── range.h
│   ├── temperature.h
│   └── voltage.h
├── logging/
│   ├── blackbox.h
│   ├── log.h
│   └── log_uart.h
├── messaging/
│   ├── buffer_pool.h
│   ├── event_bus.h
│   ├── mailbox.h
│   └── ringbuf.h
├── ui/
│   ├── graphics.h
│   └── input.h
├── DATA_SERVICES.md
├── fatfs.h
├── health.h
├── README.md
├── sensor.h
└── umid.h
~~~
