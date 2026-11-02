# UEF-PL generated catalogue reference

This reference is generated from UETD's module, type, and UMATH registries and public headers. The source JSON records and C headers are authoritative; edit those sources and regenerate this page.

A registered name is discoverable, but generation also requires a reviewed authoring contract, an implemented UETD route, and a supported target. See the [language handbook](../guides/UEF_PROJECT_LANGUAGE_HANDBOOK.md), [technical language guide](../guides/UEF_PROJECT_LANGUAGE_GUIDE.md), and [registry/discovery guide](../guides/UEF_PL_REGISTRIES.md).

The [unit reference](UEF_PL_UNITS_REFERENCE.md) is generated separately and groups exact scales by dimension.

## Registered module names

This list is for discovery. A descriptor does not by itself mean its module is ready for project generation.

| Source name | UETD module ID | Authoring status | Generation status | Manifest |
|---|---|---|---|---|
| `BareMetal` | `uos/baremetal` | no authoring contract | contract not yet authored | [registry/modules/manifest/uos/baremetal.json](../../UETD/registry/modules/manifest/uos/baremetal.json) |
| `FreeRTOS` | `uos/freertos` | no authoring contract | contract not yet authored | [registry/modules/manifest/uos/freertos.json](../../UETD/registry/modules/manifest/uos/freertos.json) |
| `SafeRTOS` | `uos/safertos` | no authoring contract | contract not yet authored | [registry/modules/manifest/uos/safertos.json](../../UETD/registry/modules/manifest/uos/safertos.json) |
| `actuator_service` | `uactuator/actuator` | no authoring contract | contract not yet authored | [registry/modules/manifest/uactuator/actuator.json](../../UETD/registry/modules/manifest/uactuator/actuator.json) |
| `adaptive_bs` | `ucon/control/nonlinear/adaptive_bs` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/adaptive_bs.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/adaptive_bs.json) |
| `adaptive_kf` | `ucon/future/estimators/kalman/adaptive_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/adaptive_kf.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/adaptive_kf.json) |
| `adaptive_mpc` | `ucon/future/control/predictive/adaptive_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/predictive/adaptive_mpc.json](../../UETD/registry/modules/manifest/ucon/future/control/predictive/adaptive_mpc.json) |
| `adc` | `upal/adc` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/analog/adc.json](../../UETD/registry/modules/manifest/upal/analog/adc.json) |
| `adrc` | `ucon/control/disturbance_rejection/adrc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/adrc.json](../../UETD/registry/modules/manifest/ucon/control/adrc.json) |
| `alpha_beta` | `ucon/estimators/tracking/alpha_beta` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/tracking/alpha_beta.json](../../UETD/registry/modules/manifest/ucon/estimators/tracking/alpha_beta.json) |
| `alpha_beta_gamma` | `ucon/estimators/tracking/alpha_beta_gamma` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/tracking/alpha_beta_gamma.json](../../UETD/registry/modules/manifest/ucon/estimators/tracking/alpha_beta_gamma.json) |
| `anti_windup_backcalc` | `ucon/infrastructure/blocks/anti_windup_backcalc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/anti_windup_backcalc.json](../../UETD/registry/modules/manifest/ucon/infrastructure/anti_windup_backcalc.json) |
| `anti_windup_clamp` | `ucon/infrastructure/blocks/anti_windup_clamp` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/anti_windup_clamp.json](../../UETD/registry/modules/manifest/ucon/infrastructure/anti_windup_clamp.json) |
| `anti_windup_cond` | `ucon/infrastructure/blocks/anti_windup_cond` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/anti_windup_cond.json](../../UETD/registry/modules/manifest/ucon/infrastructure/anti_windup_cond.json) |
| `arm_cm` | `uhal/arm_cm` | no authoring contract | contract not yet authored | [registry/modules/manifest/uhal/arm_cm.json](../../UETD/registry/modules/manifest/uhal/arm_cm.json) |
| `average_current_mode_control` | `ucon/future/electrical/power_conversion/average_current_mode_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/power_conversion/average_current_mode_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/power_conversion/average_current_mode_control.json) |
| `avr` | `uhal/avr` | no authoring contract | contract not yet authored | [registry/modules/manifest/uhal/avr.json](../../UETD/registry/modules/manifest/uhal/avr.json) |
| `aw_observer` | `ucon/future/infrastructure/blocks/aw_observer` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/aw_observer.json](../../UETD/registry/modules/manifest/ucon/future/control/aw_observer.json) |
| `backstepping` | `ucon/control/nonlinear/backstepping` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/backstepping.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/backstepping.json) |
| `bdot_detumble` | `ucon/aerospace/attitude/bdot_detumble` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/attitude/bdot_detumble.json](../../UETD/registry/modules/manifest/ucon/aerospace/attitude/bdot_detumble.json) |
| `bessel` | `ucon/filtering/coefficient_design/bessel` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/coefficient_design/bessel.json](../../UETD/registry/modules/manifest/ucon/filtering/coefficient_design/bessel.json) |
| `bezier_traj` | `ucon/trajectory/profiles/bezier_traj` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/bezier_traj.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/bezier_traj.json) |
| `binary_codec` | `ucore/codec` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucore/codec.json](../../UETD/registry/modules/manifest/ucore/codec.json) |
| `biquad_cascade` | `ucon/filtering/runtime/biquad_cascade` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/runtime/biquad_cascade.json](../../UETD/registry/modules/manifest/ucon/filtering/runtime/biquad_cascade.json) |
| `blackbox` | `umid/blackbox` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/logging/blackbox.json](../../UETD/registry/modules/manifest/umid/logging/blackbox.json) |
| `block_device` | `upal/block_device` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/storage/block_device.json](../../UETD/registry/modules/manifest/upal/storage/block_device.json) |
| `bluetooth` | `uproto/bluetooth` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/bluetooth.json](../../UETD/registry/modules/manifest/uproto/network/bluetooth.json) |
| `bounded_framing` | `uproto/framing` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/framing.json](../../UETD/registry/modules/manifest/uproto/framing.json) |
| `bspline_traj` | `ucon/trajectory/profiles/bspline_traj` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/bspline_traj.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/bspline_traj.json) |
| `buffer_pool` | `umid/buffer_pool` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/messaging/buffer_pool.json](../../UETD/registry/modules/manifest/umid/messaging/buffer_pool.json) |
| `bumpless_transfer` | `ucon/infrastructure/blocks/bumpless_transfer` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/bumpless_transfer.json](../../UETD/registry/modules/manifest/ucon/infrastructure/bumpless_transfer.json) |
| `butterworth` | `ucon/filtering/coefficient_design/butterworth` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/coefficient_design/butterworth.json](../../UETD/registry/modules/manifest/ucon/filtering/coefficient_design/butterworth.json) |
| `calibration` | `umid/calibration` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/data/calibration.json](../../UETD/registry/modules/manifest/umid/data/calibration.json) |
| `can` | `upal/can` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/can/can.json](../../UETD/registry/modules/manifest/upal/can/can.json) |
| `canopen` | `uproto/canopen` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/can/canopen.json](../../UETD/registry/modules/manifest/uproto/can/canopen.json) |
| `canopen_fd` | `uproto/future/industrial/canopen_fd` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/can/future/canopen_fd.json](../../UETD/registry/modules/manifest/uproto/can/future/canopen_fd.json) |
| `cbf_qp` | `ucon/control/nonlinear/cbf_qp` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/cbf_qp.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/cbf_qp.json) |
| `chebyshev1` | `ucon/filtering/coefficient_design/chebyshev1` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/coefficient_design/chebyshev1.json](../../UETD/registry/modules/manifest/ucon/filtering/coefficient_design/chebyshev1.json) |
| `chebyshev2` | `ucon/filtering/coefficient_design/chebyshev2` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/coefficient_design/chebyshev2.json](../../UETD/registry/modules/manifest/ucon/filtering/coefficient_design/chebyshev2.json) |
| `checked_deadline` | `ucore/deadline` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucore/deadline.json](../../UETD/registry/modules/manifest/ucore/deadline.json) |
| `checked_span` | `ucore/span` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucore/span.json](../../UETD/registry/modules/manifest/ucore/span.json) |
| `cholesky` | `umath/matrix/cholesky` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/cholesky.json](../../UETD/registry/modules/manifest/umath/matrix/cholesky.json) |
| `cic_decimator` | `ucon/future/signal_processing/resampling/cic_decimator` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/resampling/cic_decimator.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/resampling/cic_decimator.json) |
| `ckf` | `ucon/estimators/kalman/ckf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/ckf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/ckf.json) |
| `clarke_park` | `ucon/electrical/transforms/clarke_park` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/clarke_park.json](../../UETD/registry/modules/manifest/ucon/electrical/clarke_park.json) |
| `clf_cbf` | `ucon/control/nonlinear/clf_cbf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/clf_cbf.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/clf_cbf.json) |
| `clf_qp` | `ucon/control/nonlinear/clf_qp` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/clf_qp.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/clf_qp.json) |
| `coap` | `uproto/future/connectivity/coap` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/future/coap.json](../../UETD/registry/modules/manifest/uproto/network/future/coap.json) |
| `comp` | `upal/comp` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/analog/comp.json](../../UETD/registry/modules/manifest/upal/analog/comp.json) |
| `complementary` | `ucon/estimators/attitude/complementary` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/attitude/complementary.json](../../UETD/registry/modules/manifest/ucon/estimators/attitude/complementary.json) |
| `complex` | `umath/complex` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/complex.json](../../UETD/registry/modules/manifest/umath/complex.json) |
| `computed_torque` | `ucon/control/nonlinear/computed_torque` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/computed_torque.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/computed_torque.json) |
| `connection_manager` | `uproto/connection_manager` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/connection_manager.json](../../UETD/registry/modules/manifest/uproto/network/connection_manager.json) |
| `control_allocator` | `ucon/control/allocation/control_allocator` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/control_allocator.json](../../UETD/registry/modules/manifest/ucon/control/control_allocator.json) |
| `cordic` | `upal/cordic` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/compute/cordic.json](../../UETD/registry/modules/manifest/upal/compute/cordic.json) |
| `crsf` | `uproto/crsf` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/rc/crsf.json](../../UETD/registry/modules/manifest/uproto/rc/crsf.json) |
| `current` | `umid/current` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/current.json](../../UETD/registry/modules/manifest/umid/current.json) |
| `cusum_change_detector` | `ucon/future/estimators/diagnostics/cusum_change_detector` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/diagnostics/cusum_change_detector.json](../../UETD/registry/modules/manifest/ucon/future/estimators/diagnostics/cusum_change_detector.json) |
| `dac` | `upal/dac` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/analog/dac.json](../../UETD/registry/modules/manifest/upal/analog/dac.json) |
| `dahlin` | `ucon/future/control/classical/dahlin` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/classical/dahlin.json](../../UETD/registry/modules/manifest/ucon/future/control/classical/dahlin.json) |
| `ddp` | `ucon/control/predictive/ddp` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/ddp.json](../../UETD/registry/modules/manifest/ucon/control/predictive/ddp.json) |
| `ddsrf_pll` | `ucon/future/electrical/grid/ddsrf_pll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/grid/ddsrf_pll.json](../../UETD/registry/modules/manifest/ucon/future/electrical/grid/ddsrf_pll.json) |
| `deadband` | `ucon/infrastructure/blocks/deadband` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/deadband.json](../../UETD/registry/modules/manifest/ucon/infrastructure/deadband.json) |
| `deadbeat` | `ucon/future/control/classical/deadbeat` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/classical/deadbeat.json](../../UETD/registry/modules/manifest/ucon/future/control/classical/deadbeat.json) |
| `deadbeat_mpc` | `ucon/control/predictive/deadbeat_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/deadbeat_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/deadbeat_mpc.json) |
| `decoupler` | `ucon/control/state_space/decoupler` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/state_space/decoupler.json](../../UETD/registry/modules/manifest/ucon/control/state_space/decoupler.json) |
| `deferred_work` | `uos/deferred` | no authoring contract | contract not yet authored | [registry/modules/manifest/uos/deferred.json](../../UETD/registry/modules/manifest/uos/deferred.json) |
| `diagnostics` | `uapp/diagnostics` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/diagnostics/diagnostics.json](../../UETD/registry/modules/manifest/uapp/diagnostics/diagnostics.json) |
| `display` | `upal/display` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/display/display.json](../../UETD/registry/modules/manifest/upal/display/display.json) |
| `display_panel` | `upal/display_panel` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/display/display_panel.json](../../UETD/registry/modules/manifest/upal/display/display_panel.json) |
| `distributions` | `umath/probability/distributions` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/probability/distributions.json](../../UETD/registry/modules/manifest/umath/probability/distributions.json) |
| `dma` | `upal/dma` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/dma.json](../../UETD/registry/modules/manifest/upal/dma.json) |
| `dob` | `ucon/estimators/observers/dob` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/observers/dob.json](../../UETD/registry/modules/manifest/ucon/estimators/observers/dob.json) |
| `droop_control` | `ucon/electrical/grid/droop_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/droop_control.json](../../UETD/registry/modules/manifest/ucon/electrical/droop_control.json) |
| `dshot` | `uproto/dshot` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/rc/dshot.json](../../UETD/registry/modules/manifest/uproto/rc/dshot.json) |
| `dsi_host` | `upal/dsi_host` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/display/dsi_host.json](../../UETD/registry/modules/manifest/upal/display/dsi_host.json) |
| `dsogi_pll` | `ucon/future/electrical/grid/dsogi_pll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/grid/dsogi_pll.json](../../UETD/registry/modules/manifest/ucon/future/electrical/grid/dsogi_pll.json) |
| `dtc_control` | `ucon/electrical/motor_control/dtc_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/dtc_control.json](../../UETD/registry/modules/manifest/ucon/electrical/dtc_control.json) |
| `dtc_flux_torque_estimator` | `ucon/electrical/motor_control/dtc_flux_torque_estimator` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/dtc_flux_torque_estimator.json](../../UETD/registry/modules/manifest/ucon/electrical/dtc_flux_torque_estimator.json) |
| `dual` | `umath/derivatives/dual` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/derivatives/dual.json](../../UETD/registry/modules/manifest/umath/derivatives/dual.json) |
| `dual_kf` | `ucon/future/estimators/kalman/dual_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/dual_kf.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/dual_kf.json) |
| `dynamic_surface_control` | `ucon/future/control/nonlinear/dynamic_surface_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/dynamic_surface_control.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/dynamic_surface_control.json) |
| `economic_mpc` | `ucon/control/predictive/economic_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/economic_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/economic_mpc.json) |
| `edob` | `ucon/estimators/observers/edob` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/observers/edob.json](../../UETD/registry/modules/manifest/ucon/estimators/observers/edob.json) |
| `ekf` | `ucon/estimators/kalman/ekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/ekf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/ekf.json) |
| `elliptic` | `ucon/filtering/coefficient_design/elliptic` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/coefficient_design/elliptic.json](../../UETD/registry/modules/manifest/ucon/filtering/coefficient_design/elliptic.json) |
| `elrs` | `uproto/future/elrs` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/rc/future/elrs.json](../../UETD/registry/modules/manifest/uproto/rc/future/elrs.json) |
| `ema` | `ucon/filtering/runtime/ema` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/runtime/ema.json](../../UETD/registry/modules/manifest/ucon/filtering/runtime/ema.json) |
| `encoder` | `umid/encoder` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/encoder.json](../../UETD/registry/modules/manifest/umid/encoder.json) |
| `enmpc` | `ucon/control/predictive/enmpc` | draft | draft | [registry/modules/manifest/ucon/control/predictive/enmpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/enmpc.json) |
| `error_state_ekf` | `ucon/estimators/kalman/error_state_ekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/error_state_ekf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/error_state_ekf.json) |
| `eso` | `ucon/estimators/observers/eso` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/observers/eso.json](../../UETD/registry/modules/manifest/ucon/estimators/observers/eso.json) |
| `esp_idf` | `uhal/esp_idf` | no authoring contract | contract not yet authored | [registry/modules/manifest/uhal/esp_idf.json](../../UETD/registry/modules/manifest/uhal/esp_idf.json) |
| `ethernet` | `upal/ethernet` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/network/ethernet.json](../../UETD/registry/modules/manifest/upal/network/ethernet.json) |
| `event_bus` | `umid/event_bus` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/messaging/event_bus.json](../../UETD/registry/modules/manifest/umid/messaging/event_bus.json) |
| `explicit_mpc` | `ucon/control/predictive/explicit_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/explicit_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/explicit_mpc.json) |
| `extremum_seek` | `ucon/control/adaptive/extremum_seek` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/adaptive/extremum_seek.json](../../UETD/registry/modules/manifest/ucon/control/adaptive/extremum_seek.json) |
| `fading_memory_kf` | `ucon/future/estimators/kalman/fading_memory_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/fading_memory_kf.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/fading_memory_kf.json) |
| `fatfs` | `umid/fatfs` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/fatfs.json](../../UETD/registry/modules/manifest/umid/fatfs.json) |
| `fault` | `uapp/fault` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/diagnostics/fault.json](../../UETD/registry/modules/manifest/uapp/diagnostics/fault.json) |
| `fault_history` | `uapp/fault_history` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/diagnostics/fault_history.json](../../UETD/registry/modules/manifest/uapp/diagnostics/fault_history.json) |
| `fdcan_fd` | `upal/fdcan_fd` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/can/fdcan_fd.json](../../UETD/registry/modules/manifest/upal/can/fdcan_fd.json) |
| `feedback_lin` | `ucon/control/nonlinear/feedback_lin` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/feedback_lin.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/feedback_lin.json) |
| `fft` | `umath/signal/fft` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/signal/fft.json](../../UETD/registry/modules/manifest/umath/signal/fft.json) |
| `fft_radix2` | `ucon/future/signal_processing/transforms/fft_radix2` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/transforms/fft_radix2.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/transforms/fft_radix2.json) |
| `fir` | `ucon/filtering/runtime/fir` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/runtime/fir.json](../../UETD/registry/modules/manifest/ucon/filtering/runtime/fir.json) |
| `fir_windowed` | `ucon/filtering/coefficient_design/fir_windowed` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/coefficient_design/fir_windowed.json](../../UETD/registry/modules/manifest/ucon/filtering/coefficient_design/fir_windowed.json) |
| `fixed` | `umath/fixed` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/fixed.json](../../UETD/registry/modules/manifest/umath/fixed.json) |
| `flash` | `upal/flash` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/storage/flash.json](../../UETD/registry/modules/manifest/upal/storage/flash.json) |
| `flatness` | `ucon/control/nonlinear/flatness` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/flatness.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/flatness.json) |
| `fll` | `ucon/estimators/frequency/fll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/frequency/fll.json](../../UETD/registry/modules/manifest/ucon/estimators/frequency/fll.json) |
| `flux_weakening_control` | `ucon/future/electrical/motor_control/flux_weakening_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/motor_control/flux_weakening_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/motor_control/flux_weakening_control.json) |
| `fmac` | `upal/fmac` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/compute/fmac.json](../../UETD/registry/modules/manifest/upal/compute/fmac.json) |
| `foc_current_control` | `ucon/electrical/motor_control/foc_current_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/foc_current_control.json](../../UETD/registry/modules/manifest/ucon/electrical/foc_current_control.json) |
| `fpga` | `umid/fpga` | reviewed | ready | [registry/modules/manifest/umid/fpga.json](../../UETD/registry/modules/manifest/umid/fpga.json) |
| `fractional_order_smc` | `ucon/future/control/nonlinear/fractional_order_smc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/fractional_order_smc.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/fractional_order_smc.json) |
| `fs_mpc` | `ucon/control/predictive/fs_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/fs_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/fs_mpc.json) |
| `functions` | `umath/special/functions` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/functions.json](../../UETD/registry/modules/manifest/umath/functions.json) |
| `fuzzy_controller` | `ucon/future/control/nonlinear/fuzzy_controller` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/fuzzy_controller.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/fuzzy_controller.json) |
| `gain_block` | `ucon/infrastructure/blocks/gain_block` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/gain_block.json](../../UETD/registry/modules/manifest/ucon/infrastructure/gain_block.json) |
| `gain_schedule` | `ucon/control/scheduled/gain_schedule` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/gain_schedule.json](../../UETD/registry/modules/manifest/ucon/control/gain_schedule.json) |
| `gain_scheduled_mpc` | `ucon/future/control/predictive/gain_scheduled_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/predictive/gain_scheduled_mpc.json](../../UETD/registry/modules/manifest/ucon/future/control/predictive/gain_scheduled_mpc.json) |
| `gaussian_sum_filter` | `ucon/future/estimators/kalman/gaussian_sum_filter` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/gaussian_sum_filter.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/gaussian_sum_filter.json) |
| `glucose_control_profile` | `ucon/medical/profiles/glucose_control_profile` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/medical/profiles/glucose_control_profile.json](../../UETD/registry/modules/manifest/ucon/medical/profiles/glucose_control_profile.json) |
| `gnss` | `umid/gnss` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/gnss.json](../../UETD/registry/modules/manifest/umid/gnss.json) |
| `gnss_carrier_fll` | `ucon/aerospace/navigation/gnss_carrier_fll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/navigation/gnss_carrier_fll.json](../../UETD/registry/modules/manifest/ucon/aerospace/navigation/gnss_carrier_fll.json) |
| `gnss_carrier_pll` | `ucon/aerospace/navigation/gnss_carrier_pll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/navigation/gnss_carrier_pll.json](../../UETD/registry/modules/manifest/ucon/aerospace/navigation/gnss_carrier_pll.json) |
| `gnss_code_dll` | `ucon/aerospace/navigation/gnss_code_dll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/navigation/gnss_code_dll.json](../../UETD/registry/modules/manifest/ucon/aerospace/navigation/gnss_code_dll.json) |
| `goertzel` | `ucon/future/signal_processing/transforms/goertzel` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/transforms/goertzel.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/transforms/goertzel.json) |
| `graphics_adapter` | `umid/graphics` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/ui/graphics.json](../../UETD/registry/modules/manifest/umid/ui/graphics.json) |
| `h2_state_feedback` | `ucon/future/control/state_space/h2_state_feedback` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/state_space/h2_state_feedback.json](../../UETD/registry/modules/manifest/ucon/future/control/state_space/h2_state_feedback.json) |
| `h_bridge` | `uactuator/h_bridge` | draft | draft | [registry/modules/manifest/uactuator/h_bridge.json](../../UETD/registry/modules/manifest/uactuator/h_bridge.json) |
| `h_inf_obs` | `ucon/future/estimators/observers/h_inf_obs` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/observers/h_inf_obs.json](../../UETD/registry/modules/manifest/ucon/future/estimators/observers/h_inf_obs.json) |
| `h_inf_of` | `ucon/future/control/state_space/h_inf_of` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/state_space/h_inf_of.json](../../UETD/registry/modules/manifest/ucon/future/control/state_space/h_inf_of.json) |
| `h_inf_sf` | `ucon/future/control/state_space/h_inf_sf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/state_space/h_inf_sf.json](../../UETD/registry/modules/manifest/ucon/future/control/state_space/h_inf_sf.json) |
| `half_cycle_predictive_current_control` | `ucon/future/electrical/power_conversion/half_cycle_predictive_current_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/power_conversion/half_cycle_predictive_current_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/power_conversion/half_cycle_predictive_current_control.json) |
| `health` | `umid/health` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/health.json](../../UETD/registry/modules/manifest/umid/health.json) |
| `high_gain_observer` | `ucon/future/estimators/observers/high_gain_observer` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/observers/high_gain_observer.json](../../UETD/registry/modules/manifest/ucon/future/estimators/observers/high_gain_observer.json) |
| `hrtim` | `upal/hrtim` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/timing/hrtim.json](../../UETD/registry/modules/manifest/upal/timing/hrtim.json) |
| `http` | `uproto/http` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/http.json](../../UETD/registry/modules/manifest/uproto/network/http.json) |
| `hysteresis` | `ucon/infrastructure/blocks/hysteresis` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/hysteresis.json](../../UETD/registry/modules/manifest/ucon/infrastructure/hysteresis.json) |
| `hysteretic_current_control` | `ucon/future/electrical/power_conversion/hysteretic_current_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/power_conversion/hysteretic_current_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/power_conversion/hysteretic_current_control.json) |
| `i2c` | `upal/i2c` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/serial/i2c.json](../../UETD/registry/modules/manifest/upal/serial/i2c.json) |
| `ida_pbc` | `ucon/future/control/nonlinear/ida_pbc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/ida_pbc.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/ida_pbc.json) |
| `iec60870` | `uproto/future/iec60870` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/industrial/iec60870.json](../../UETD/registry/modules/manifest/uproto/industrial/iec60870.json) |
| `ieee802154` | `upal/future/ieee802154` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/network/ieee802154.json](../../UETD/registry/modules/manifest/upal/network/ieee802154.json) |
| `ilc` | `ucon/future/control/adaptive/ilc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/adaptive/ilc.json](../../UETD/registry/modules/manifest/ucon/future/control/adaptive/ilc.json) |
| `ilqr` | `ucon/control/predictive/ilqr` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/ilqr.json](../../UETD/registry/modules/manifest/ucon/control/predictive/ilqr.json) |
| `imc` | `ucon/control/classical/imc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/classical/imc.json](../../UETD/registry/modules/manifest/ucon/control/classical/imc.json) |
| `imm_ekf` | `ucon/estimators/kalman/imm_ekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/imm_ekf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/imm_ekf.json) |
| `imu` | `umid/imu` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/imu.json](../../UETD/registry/modules/manifest/umid/imu.json) |
| `indi` | `ucon/control/nonlinear/indi` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/indi.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/indi.json) |
| `indi2` | `ucon/control/nonlinear/indi2` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/indi2.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/indi2.json) |
| `innovation_monitor` | `ucon/future/estimators/diagnostics/innovation_monitor` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/diagnostics/innovation_monitor.json](../../UETD/registry/modules/manifest/ucon/future/estimators/diagnostics/innovation_monitor.json) |
| `input_service` | `umid/input` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/ui/input.json](../../UETD/registry/modules/manifest/umid/ui/input.json) |
| `ins_gnss` | `ucon/aerospace/navigation/ins_gnss` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/navigation/ins_gnss.json](../../UETD/registry/modules/manifest/ucon/aerospace/navigation/ins_gnss.json) |
| `ins_gnss_tight` | `ucon/aerospace/navigation/ins_gnss_tight` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/navigation/ins_gnss_tight.json](../../UETD/registry/modules/manifest/ucon/aerospace/navigation/ins_gnss_tight.json) |
| `integral_smc` | `ucon/future/control/nonlinear/integral_smc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/integral_smc.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/integral_smc.json) |
| `integration` | `umath/numerical/integration` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/numerical/integration.json](../../UETD/registry/modules/manifest/umath/numerical/integration.json) |
| `integrator` | `ucon/infrastructure/blocks/integrator` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/integrator.json](../../UETD/registry/modules/manifest/ucon/infrastructure/integrator.json) |
| `invariant_ekf` | `ucon/estimators/kalman/invariant_ekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/invariant_ekf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/invariant_ekf.json) |
| `io_transaction` | `upal/transaction` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/transaction.json](../../UETD/registry/modules/manifest/upal/transaction.json) |
| `ip_stack` | `uproto/ip` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/ip.json](../../UETD/registry/modules/manifest/uproto/network/ip.json) |
| `iterated_ekf` | `ucon/estimators/kalman/iterated_ekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/iterated_ekf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/iterated_ekf.json) |
| `j1939` | `uproto/future/j1939` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/can/future/j1939.json](../../UETD/registry/modules/manifest/uproto/can/future/j1939.json) |
| `jacobian` | `umath/derivatives/jacobian` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/derivatives/jacobian.json](../../UETD/registry/modules/manifest/umath/derivatives/jacobian.json) |
| `jerk_profile` | `ucon/trajectory/profiles/jerk_profile` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/jerk_profile.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/jerk_profile.json) |
| `l1_adaptive` | `ucon/future/control/adaptive/l1_adaptive` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/adaptive/l1_adaptive.json](../../UETD/registry/modules/manifest/ucon/future/control/adaptive/l1_adaptive.json) |
| `latest_mailbox` | `umid/mailbox` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/messaging/mailbox.json](../../UETD/registry/modules/manifest/umid/messaging/mailbox.json) |
| `ldlt` | `umath/matrix/ldlt` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/ldlt.json](../../UETD/registry/modules/manifest/umath/matrix/ldlt.json) |
| `lead_lag` | `ucon/control/classical/lead_lag` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/classical/lead_lag.json](../../UETD/registry/modules/manifest/ucon/control/classical/lead_lag.json) |
| `least_squares` | `umath/optimization/least_squares` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/numerical/least_squares.json](../../UETD/registry/modules/manifest/umath/numerical/least_squares.json) |
| `lifecycle` | `uapp/lifecycle` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/runtime/lifecycle.json](../../UETD/registry/modules/manifest/uapp/runtime/lifecycle.json) |
| `lin` | `uproto/future/lin` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/industrial/lin.json](../../UETD/registry/modules/manifest/uproto/industrial/lin.json) |
| `linear_kf` | `ucon/estimators/kalman/linear_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/linear_kf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/linear_kf.json) |
| `linear_mpc` | `ucon/control/predictive/linear_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/linear_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/linear_mpc.json) |
| `lms` | `ucon/future/filtering/adaptive/lms` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/filtering/lms.json](../../UETD/registry/modules/manifest/ucon/future/filtering/lms.json) |
| `log` | `umid/log` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/logging/log.json](../../UETD/registry/modules/manifest/umid/logging/log.json) |
| `log_uart` | `umid/log_uart` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/logging/log_uart.json](../../UETD/registry/modules/manifest/umid/logging/log_uart.json) |
| `los_guidance` | `ucon/future/aerospace/guidance/los_guidance` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/aerospace/guidance/los_guidance.json](../../UETD/registry/modules/manifest/ucon/future/aerospace/guidance/los_guidance.json) |
| `lqg` | `ucon/control/state_space/lqg` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/state_space/lqg.json](../../UETD/registry/modules/manifest/ucon/control/state_space/lqg.json) |
| `lqi` | `ucon/control/state_space/lqi` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/state_space/lqi.json](../../UETD/registry/modules/manifest/ucon/control/state_space/lqi.json) |
| `lqr` | `ucon/control/state_space/lqr` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/state_space/lqr.json](../../UETD/registry/modules/manifest/ucon/control/state_space/lqr.json) |
| `lu` | `umath/matrix/lu` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/lu.json](../../UETD/registry/modules/manifest/umath/matrix/lu.json) |
| `luenberger` | `ucon/estimators/observers/luenberger` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/observers/luenberger.json](../../UETD/registry/modules/manifest/ucon/estimators/observers/luenberger.json) |
| `madgwick` | `ucon/estimators/attitude/madgwick` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/attitude/madgwick.json](../../UETD/registry/modules/manifest/ucon/estimators/attitude/madgwick.json) |
| `magnetometer` | `umid/mag` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/mag.json](../../UETD/registry/modules/manifest/umid/mag.json) |
| `mahony` | `ucon/estimators/attitude/mahony` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/attitude/mahony.json](../../UETD/registry/modules/manifest/ucon/estimators/attitude/mahony.json) |
| `matrix_core` | `umath/matrix/core` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/core.json](../../UETD/registry/modules/manifest/umath/matrix/core.json) |
| `mavlink` | `uproto/mavlink` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/mavlink.json](../../UETD/registry/modules/manifest/uproto/mavlink.json) |
| `medi_can` | `uproto/future/medi_can` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/can/future/medi_can.json](../../UETD/registry/modules/manifest/uproto/can/future/medi_can.json) |
| `median` | `ucon/filtering/runtime/median` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/runtime/median.json](../../UETD/registry/modules/manifest/ucon/filtering/runtime/median.json) |
| `mekf` | `ucon/estimators/attitude/mekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/attitude/mekf.json](../../UETD/registry/modules/manifest/ucon/estimators/attitude/mekf.json) |
| `mesh_network` | `uproto/future/connectivity/mesh` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/future/mesh.json](../../UETD/registry/modules/manifest/uproto/network/future/mesh.json) |
| `mhe` | `ucon/estimators/moving_horizon/mhe` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/mhe.json](../../UETD/registry/modules/manifest/ucon/estimators/mhe.json) |
| `minimum_snap_traj` | `ucon/future/trajectory/optimization/minimum_snap_traj` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/trajectory/minimum_snap_traj.json](../../UETD/registry/modules/manifest/ucon/future/trajectory/minimum_snap_traj.json) |
| `modbus` | `uproto/modbus` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/industrial/modbus.json](../../UETD/registry/modules/manifest/uproto/industrial/modbus.json) |
| `mode_select` | `ucon/infrastructure/blocks/mode_select` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/mode_select.json](../../UETD/registry/modules/manifest/ucon/infrastructure/mode_select.json) |
| `moving_average` | `ucon/filtering/runtime/moving_average` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/runtime/moving_average.json](../../UETD/registry/modules/manifest/ucon/filtering/runtime/moving_average.json) |
| `mqtt` | `uproto/mqtt` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/mqtt.json](../../UETD/registry/modules/manifest/uproto/network/mqtt.json) |
| `mrac_composite` | `ucon/future/control/adaptive/mrac_composite` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/adaptive/mrac_composite.json](../../UETD/registry/modules/manifest/ucon/future/control/adaptive/mrac_composite.json) |
| `mrac_lyapunov` | `ucon/future/control/adaptive/mrac_lyapunov` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/adaptive/mrac_lyapunov.json](../../UETD/registry/modules/manifest/ucon/future/control/adaptive/mrac_lyapunov.json) |
| `mrac_mit` | `ucon/future/control/adaptive/mrac_mit` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/adaptive/mrac_mit.json](../../UETD/registry/modules/manifest/ucon/future/control/adaptive/mrac_mit.json) |
| `msogi_fll` | `ucon/future/electrical/grid/msogi_fll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/grid/msogi_fll.json](../../UETD/registry/modules/manifest/ucon/future/electrical/grid/msogi_fll.json) |
| `msp` | `uproto/future/msp` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/rc/future/msp.json](../../UETD/registry/modules/manifest/uproto/rc/future/msp.json) |
| `mtpa_control` | `ucon/future/electrical/motor_control/mtpa_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/motor_control/mtpa_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/motor_control/mtpa_control.json) |
| `multirate_bridge` | `ucon/infrastructure/blocks/multirate_bridge` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/multirate_bridge.json](../../UETD/registry/modules/manifest/ucon/infrastructure/multirate_bridge.json) |
| `multivariate_normal` | `umath/probability/multivariate_normal` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/probability/multivariate_normal.json](../../UETD/registry/modules/manifest/umath/probability/multivariate_normal.json) |
| `nco` | `ucon/estimators/frequency/nco` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/frequency/nco.json](../../UETD/registry/modules/manifest/ucon/estimators/frequency/nco.json) |
| `ndi` | `ucon/control/nonlinear/ndi` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/nonlinear/ndi.json](../../UETD/registry/modules/manifest/ucon/control/nonlinear/ndi.json) |
| `netif` | `upal/netif` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/network/netif.json](../../UETD/registry/modules/manifest/upal/network/netif.json) |
| `nlms` | `ucon/future/filtering/adaptive/nlms` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/filtering/nlms.json](../../UETD/registry/modules/manifest/ucon/future/filtering/nlms.json) |
| `nmpc_rti` | `ucon/control/predictive/nmpc_rti` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/nmpc_rti.json](../../UETD/registry/modules/manifest/ucon/control/predictive/nmpc_rti.json) |
| `notch` | `ucon/filtering/runtime/notch` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/filtering/runtime/notch.json](../../UETD/registry/modules/manifest/ucon/filtering/runtime/notch.json) |
| `opamp` | `upal/opamp` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/analog/opamp.json](../../UETD/registry/modules/manifest/upal/analog/opamp.json) |
| `out_of_sequence_kf` | `ucon/future/estimators/kalman/out_of_sequence_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/out_of_sequence_kf.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/out_of_sequence_kf.json) |
| `particle` | `ucon/estimators/bayesian/particle` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/particle.json](../../UETD/registry/modules/manifest/ucon/estimators/particle.json) |
| `path_following_guidance` | `ucon/aerospace/guidance/path_following_guidance` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/guidance/path_following_guidance.json](../../UETD/registry/modules/manifest/ucon/aerospace/guidance/path_following_guidance.json) |
| `peak_current_mode_control` | `ucon/future/electrical/power_conversion/peak_current_mode_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/power_conversion/peak_current_mode_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/power_conversion/peak_current_mode_control.json) |
| `peak_tracker` | `ucon/future/signal_processing/statistics/peak_tracker` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/statistics/peak_tracker.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/statistics/peak_tracker.json) |
| `peripheral_extensions` | `upal/capability_contracts` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/capability_contracts.json](../../UETD/registry/modules/manifest/upal/capability_contracts.json) |
| `persistent_excitation_monitor` | `ucon/future/estimators/identification/persistent_excitation_monitor` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/persistent_excitation_monitor.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/persistent_excitation_monitor.json) |
| `phase_shift_full_bridge_control` | `ucon/future/electrical/power_conversion/phase_shift_full_bridge_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/power_conversion/phase_shift_full_bridge_control.json](../../UETD/registry/modules/manifest/ucon/future/electrical/power_conversion/phase_shift_full_bridge_control.json) |
| `physics_constants` | `uphysics/future/constants` | no authoring contract | contract not yet authored | [registry/modules/manifest/uphysics/constants.json](../../UETD/registry/modules/manifest/uphysics/constants.json) |
| `pid` | `ucon/control/classical/pid` | draft | draft | [registry/modules/manifest/ucon/control/classical/pid.json](../../UETD/registry/modules/manifest/ucon/control/classical/pid.json) |
| `pid_autotune` | `ucon/control/tuning/pid_autotune` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/pid_autotune.json](../../UETD/registry/modules/manifest/ucon/control/pid_autotune.json) |
| `pkpd_target_control` | `ucon/medical/profiles/pkpd_target_control` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/medical/profiles/pkpd_target_control.json](../../UETD/registry/modules/manifest/ucon/medical/profiles/pkpd_target_control.json) |
| `pll` | `ucon/estimators/frequency/pll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/frequency/pll.json](../../UETD/registry/modules/manifest/ucon/estimators/frequency/pll.json) |
| `pn_guidance` | `ucon/aerospace/guidance/pn_guidance` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/guidance/pn_guidance.json](../../UETD/registry/modules/manifest/ucon/aerospace/guidance/pn_guidance.json) |
| `pole_place` | `ucon/future/control/synthesis/pole_place` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/pole_place.json](../../UETD/registry/modules/manifest/ucon/future/control/pole_place.json) |
| `poly_traj_3` | `ucon/trajectory/profiles/poly_traj_3` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/poly_traj_3.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/poly_traj_3.json) |
| `poly_traj_5` | `ucon/trajectory/profiles/poly_traj_5` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/poly_traj_5.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/poly_traj_5.json) |
| `polynomial_core` | `umath/polynomial/core` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/polynomial/core.json](../../UETD/registry/modules/manifest/umath/polynomial/core.json) |
| `polyphase_resampler` | `ucon/future/signal_processing/resampling/polyphase_resampler` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/resampling/polyphase_resampler.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/resampling/polyphase_resampler.json) |
| `power_manager` | `upal/power` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/power.json](../../UETD/registry/modules/manifest/upal/power.json) |
| `ppm` | `uproto/ppm` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/rc/ppm.json](../../UETD/registry/modules/manifest/uproto/rc/ppm.json) |
| `ppp` | `uproto/future/connectivity/ppp` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/future/ppp.json](../../UETD/registry/modules/manifest/uproto/network/future/ppp.json) |
| `pressure` | `umid/pressure` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/pressure.json](../../UETD/registry/modules/manifest/umid/pressure.json) |
| `prng` | `umath/random/prng` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/probability/prng.json](../../UETD/registry/modules/manifest/umath/probability/prng.json) |
| `process_image` | `umid/process_image` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/data/process_image.json](../../UETD/registry/modules/manifest/umid/data/process_image.json) |
| `product_session` | `uapp/product_session` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/product_session.json](../../UETD/registry/modules/manifest/uapp/product_session.json) |
| `pure_pursuit_guidance` | `ucon/future/aerospace/guidance/pure_pursuit_guidance` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/aerospace/guidance/pure_pursuit_guidance.json](../../UETD/registry/modules/manifest/ucon/future/aerospace/guidance/pure_pursuit_guidance.json) |
| `qp_active_set` | `ucon/optimization/qp/qp_active_set` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/qp_active_set.json](../../UETD/registry/modules/manifest/ucon/qp_active_set.json) |
| `qp_admm` | `ucon/future/optimization/qp/qp_admm` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/optimization/qp/qp_admm.json](../../UETD/registry/modules/manifest/ucon/future/optimization/qp/qp_admm.json) |
| `qp_projected_gradient` | `ucon/future/optimization/qp/qp_projected_gradient` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/optimization/qp/qp_projected_gradient.json](../../UETD/registry/modules/manifest/ucon/future/optimization/qp/qp_projected_gradient.json) |
| `qr` | `umath/matrix/qr` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/qr.json](../../UETD/registry/modules/manifest/umath/matrix/qr.json) |
| `qspi` | `upal/qspi` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/storage/qspi.json](../../UETD/registry/modules/manifest/upal/storage/qspi.json) |
| `quaternion` | `umath/geometry/quaternion` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/geometry/quaternion.json](../../UETD/registry/modules/manifest/umath/geometry/quaternion.json) |
| `quaternion_pd` | `ucon/aerospace/attitude/quaternion_pd` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/attitude/quaternion_pd.json](../../UETD/registry/modules/manifest/ucon/aerospace/attitude/quaternion_pd.json) |
| `quest` | `ucon/estimators/attitude/quest` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/attitude/quest.json](../../UETD/registry/modules/manifest/ucon/estimators/attitude/quest.json) |
| `radio_transport` | `upal/radio_transport` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/network/radio_transport.json](../../UETD/registry/modules/manifest/upal/network/radio_transport.json) |
| `range` | `umid/range` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/range.json](../../UETD/registry/modules/manifest/umid/range.json) |
| `rate_limiter` | `ucon/infrastructure/blocks/rate_limiter` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/rate_limiter.json](../../UETD/registry/modules/manifest/ucon/infrastructure/rate_limiter.json) |
| `recursive_ar` | `ucon/future/estimators/identification/recursive_ar` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_ar.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_ar.json) |
| `recursive_arma` | `ucon/future/estimators/identification/recursive_arma` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_arma.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_arma.json) |
| `recursive_armax` | `ucon/future/estimators/identification/recursive_armax` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_armax.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_armax.json) |
| `recursive_arx` | `ucon/future/estimators/identification/recursive_arx` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_arx.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_arx.json) |
| `recursive_bj` | `ucon/future/estimators/identification/recursive_bj` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_bj.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_bj.json) |
| `recursive_iv` | `ucon/future/estimators/identification/recursive_iv` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_iv.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_iv.json) |
| `recursive_oe` | `ucon/future/estimators/identification/recursive_oe` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/recursive_oe.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/recursive_oe.json) |
| `reduced_obs` | `ucon/future/estimators/observers/reduced_obs` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/observers/reduced_obs.json](../../UETD/registry/modules/manifest/ucon/future/estimators/observers/reduced_obs.json) |
| `ref_model_1` | `ucon/trajectory/profiles/ref_model_1` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/ref_model_1.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/ref_model_1.json) |
| `ref_model_2` | `ucon/trajectory/profiles/ref_model_2` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/ref_model_2.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/ref_model_2.json) |
| `reference_governor` | `ucon/control/safety/reference_governor` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/reference_governor.json](../../UETD/registry/modules/manifest/ucon/control/reference_governor.json) |
| `repetitive` | `ucon/future/control/classical/repetitive` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/classical/repetitive.json](../../UETD/registry/modules/manifest/ucon/future/control/classical/repetitive.json) |
| `resonant_ekf_llc_src` | `ucon/future/estimators/kalman/power_conversion/resonant_ekf_llc_src` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/resonant_ekf_llc_src.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/resonant_ekf_llc_src.json) |
| `resonant_pr` | `ucon/electrical/grid/resonant_pr` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/resonant_pr.json](../../UETD/registry/modules/manifest/ucon/electrical/resonant_pr.json) |
| `ringbuf` | `umid/ringbuf` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/messaging/ringbuf.json](../../UETD/registry/modules/manifest/umid/messaging/ringbuf.json) |
| `rise` | `ucon/future/control/nonlinear/rise` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/rise.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/rise.json) |
| `rls` | `ucon/estimators/identification/rls` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/rls.json](../../UETD/registry/modules/manifest/ucon/estimators/rls.json) |
| `rms_window` | `ucon/future/signal_processing/statistics/rms_window` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/statistics/rms_window.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/statistics/rms_window.json) |
| `robust_kf` | `ucon/future/estimators/kalman/robust_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/robust_kf.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/robust_kf.json) |
| `robust_mpc` | `ucon/control/predictive/robust_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/robust_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/robust_mpc.json) |
| `roots` | `umath/polynomial/roots` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/polynomial/roots.json](../../UETD/registry/modules/manifest/umath/polynomial/roots.json) |
| `rpem` | `ucon/future/estimators/identification/rpem` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/identification/rpem.json](../../UETD/registry/modules/manifest/ucon/future/estimators/identification/rpem.json) |
| `rtc` | `upal/rtc` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/timing/rtc.json](../../UETD/registry/modules/manifest/upal/timing/rtc.json) |
| `s_curve` | `ucon/trajectory/profiles/s_curve` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/s_curve.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/s_curve.json) |
| `saturator` | `ucon/infrastructure/blocks/saturator` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/saturator.json](../../UETD/registry/modules/manifest/ucon/infrastructure/saturator.json) |
| `savgol_filter` | `ucon/future/filtering/runtime/savgol_filter` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/filtering/savgol_filter.json](../../UETD/registry/modules/manifest/ucon/future/filtering/savgol_filter.json) |
| `sbus` | `uproto/sbus` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/rc/sbus.json](../../UETD/registry/modules/manifest/uproto/rc/sbus.json) |
| `scalar_kf` | `ucon/estimators/kalman/scalar_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/scalar_kf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/scalar_kf.json) |
| `sdmmc` | `upal/sdmmc` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/storage/sdmmc.json](../../UETD/registry/modules/manifest/upal/storage/sdmmc.json) |
| `se2` | `umath/lie/se2` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/lie/se2.json](../../UETD/registry/modules/manifest/umath/lie/se2.json) |
| `se3` | `umath/lie/se3` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/lie/se3.json](../../UETD/registry/modules/manifest/umath/lie/se3.json) |
| `secure_update` | `uapp/secure_update` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/secure_update.json](../../UETD/registry/modules/manifest/uapp/secure_update.json) |
| `security_provider` | `usec/provider` | no authoring contract | contract not yet authored | [registry/modules/manifest/usec/provider.json](../../UETD/registry/modules/manifest/usec/provider.json) |
| `self_tune_reg` | `ucon/control/adaptive/self_tune_reg` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/adaptive/self_tune_reg.json](../../UETD/registry/modules/manifest/ucon/control/adaptive/self_tune_reg.json) |
| `sensor_fdi` | `ucon/infrastructure/fault_detection/sensor_fdi` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/sensor_fdi.json](../../UETD/registry/modules/manifest/ucon/infrastructure/sensor_fdi.json) |
| `sensor_service` | `umid/sensor` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/sensor.json](../../UETD/registry/modules/manifest/umid/sensor.json) |
| `sensor_voter` | `ucon/infrastructure/blocks/sensor_voter` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/sensor_voter.json](../../UETD/registry/modules/manifest/ucon/infrastructure/sensor_voter.json) |
| `sensorless_foc_observer` | `ucon/future/electrical/motor_control/sensorless_foc_observer` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/electrical/motor_control/sensorless_foc_observer.json](../../UETD/registry/modules/manifest/ucon/future/electrical/motor_control/sensorless_foc_observer.json) |
| `settings` | `umid/settings` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/data/settings.json](../../UETD/registry/modules/manifest/umid/data/settings.json) |
| `signal_validity` | `ucon/infrastructure/blocks/signal_validity` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/signal_validity.json](../../UETD/registry/modules/manifest/ucon/infrastructure/signal_validity.json) |
| `sliding_dft` | `ucon/estimators/frequency/sliding_dft` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/frequency/sliding_dft.json](../../UETD/registry/modules/manifest/ucon/estimators/frequency/sliding_dft.json) |
| `sliding_mode_observer` | `ucon/future/estimators/observers/sliding_mode_observer` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/observers/sliding_mode_observer.json](../../UETD/registry/modules/manifest/ucon/future/estimators/observers/sliding_mode_observer.json) |
| `smc` | `ucon/future/control/nonlinear/smc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/smc.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/smc.json) |
| `smith_predictor` | `ucon/control/classical/smith_predictor` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/classical/smith_predictor.json](../../UETD/registry/modules/manifest/ucon/control/classical/smith_predictor.json) |
| `so2` | `umath/lie/so2` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/lie/so2.json](../../UETD/registry/modules/manifest/umath/lie/so2.json) |
| `so3` | `umath/lie/so3` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/lie/so3.json](../../UETD/registry/modules/manifest/umath/lie/so3.json) |
| `so3_attitude` | `ucon/aerospace/attitude/so3_attitude` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/aerospace/attitude/so3_attitude.json](../../UETD/registry/modules/manifest/ucon/aerospace/attitude/so3_attitude.json) |
| `sogi` | `ucon/estimators/frequency/sogi` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/frequency/sogi.json](../../UETD/registry/modules/manifest/ucon/estimators/frequency/sogi.json) |
| `sogi_fll` | `ucon/electrical/grid/sogi_fll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/sogi_fll.json](../../UETD/registry/modules/manifest/ucon/electrical/sogi_fll.json) |
| `sogi_pll` | `ucon/electrical/grid/sogi_pll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/sogi_pll.json](../../UETD/registry/modules/manifest/ucon/electrical/sogi_pll.json) |
| `sparse_csr` | `umath/future/matrix/sparse_csr` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/sparse_csr.json](../../UETD/registry/modules/manifest/umath/matrix/sparse_csr.json) |
| `sparse_mpc` | `ucon/control/predictive/sparse_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/sparse_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/sparse_mpc.json) |
| `spectral_peak_interpolation` | `ucon/future/signal_processing/analysis/spectral_peak_interpolation` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/spectral_peak_interpolation.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/spectral_peak_interpolation.json) |
| `spi` | `upal/spi` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/serial/spi.json](../../UETD/registry/modules/manifest/upal/serial/spi.json) |
| `splines` | `umath/interpolation/splines` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/numerical/splines.json](../../UETD/registry/modules/manifest/umath/numerical/splines.json) |
| `sqrt_ekf` | `ucon/estimators/kalman/sqrt_ekf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/sqrt_ekf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/sqrt_ekf.json) |
| `sqrt_ukf` | `ucon/estimators/kalman/sqrt_ukf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/sqrt_ukf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/sqrt_ukf.json) |
| `srf_pll` | `ucon/electrical/grid/srf_pll` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/srf_pll.json](../../UETD/registry/modules/manifest/ucon/electrical/srf_pll.json) |
| `state_feedback` | `ucon/control/state_space/state_feedback` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/state_space/state_feedback.json](../../UETD/registry/modules/manifest/ucon/control/state_space/state_feedback.json) |
| `statemachine` | `uapp/statemachine` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/runtime/statemachine.json](../../UETD/registry/modules/manifest/uapp/runtime/statemachine.json) |
| `stochastic_mpc` | `ucon/control/predictive/stochastic_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/stochastic_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/stochastic_mpc.json) |
| `storage` | `umid/storage` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/data/storage.json](../../UETD/registry/modules/manifest/umid/data/storage.json) |
| `streaming` | `umath/statistics/streaming` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/probability/streaming.json](../../UETD/registry/modules/manifest/umath/probability/streaming.json) |
| `sum_junction` | `ucon/infrastructure/blocks/sum_junction` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/sum_junction.json](../../UETD/registry/modules/manifest/ucon/infrastructure/sum_junction.json) |
| `super_twisting` | `ucon/future/control/nonlinear/super_twisting` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/super_twisting.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/super_twisting.json) |
| `supervisor` | `uapp/supervisor` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/runtime/supervisor.json](../../UETD/registry/modules/manifest/uapp/runtime/supervisor.json) |
| `svd` | `umath/matrix/svd` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/svd.json](../../UETD/registry/modules/manifest/umath/matrix/svd.json) |
| `svpwm` | `ucon/electrical/motor_control/svpwm` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/electrical/svpwm.json](../../UETD/registry/modules/manifest/ucon/electrical/svpwm.json) |
| `symmetric_eigen` | `umath/matrix/symmetric_eigen` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/matrix/symmetric_eigen.json](../../UETD/registry/modules/manifest/umath/matrix/symmetric_eigen.json) |
| `synchronous_demod` | `ucon/future/signal_processing/demodulation/synchronous_demod` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/signal_processing/synchronous_demod.json](../../UETD/registry/modules/manifest/ucon/future/signal_processing/synchronous_demod.json) |
| `target_profiles` | `uhal/target_profiles` | no authoring contract | contract not yet authored | [registry/modules/manifest/uhal/target_profiles.json](../../UETD/registry/modules/manifest/uhal/target_profiles.json) |
| `temp` | `umid/temp` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/temp.json](../../UETD/registry/modules/manifest/umid/temp.json) |
| `terminal_smc` | `ucon/future/control/nonlinear/terminal_smc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/control/nonlinear/terminal_smc.json](../../UETD/registry/modules/manifest/ucon/future/control/nonlinear/terminal_smc.json) |
| `time_optimal_profile` | `ucon/future/trajectory/profiles/time_optimal_profile` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/trajectory/time_optimal_profile.json](../../UETD/registry/modules/manifest/ucon/future/trajectory/time_optimal_profile.json) |
| `time_sync` | `uproto/time_sync` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/time_sync.json](../../UETD/registry/modules/manifest/uproto/network/time_sync.json) |
| `timer` | `upal/timer` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/timing/timer.json](../../UETD/registry/modules/manifest/upal/timing/timer.json) |
| `tls` | `uproto/tls` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/tls.json](../../UETD/registry/modules/manifest/uproto/network/tls.json) |
| `touch` | `upal/touch` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/display/touch.json](../../UETD/registry/modules/manifest/upal/display/touch.json) |
| `tracking_mpc` | `ucon/control/predictive/tracking_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/tracking_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/tracking_mpc.json) |
| `transform3` | `umath/geometry/transform3` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/geometry/transform3.json](../../UETD/registry/modules/manifest/umath/geometry/transform3.json) |
| `trapezoid_profile` | `ucon/trajectory/profiles/trapezoid_profile` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/trajectory/profiles/trapezoid_profile.json](../../UETD/registry/modules/manifest/ucon/trajectory/profiles/trapezoid_profile.json) |
| `triad` | `ucon/estimators/attitude/triad` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/attitude/triad.json](../../UETD/registry/modules/manifest/ucon/estimators/attitude/triad.json) |
| `uart` | `upal/uart` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/serial/uart.json](../../UETD/registry/modules/manifest/upal/serial/uart.json) |
| `uavcan` | `uproto/uavcan` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/can/uavcan.json](../../UETD/registry/modules/manifest/uproto/can/uavcan.json) |
| `uavcan_v1` | `uproto/future/uavcan_v1` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/can/future/uavcan_v1.json](../../UETD/registry/modules/manifest/uproto/can/future/uavcan_v1.json) |
| `ucon_core` | `ucon/core` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/core.json](../../UETD/registry/modules/manifest/ucon/core.json) |
| `ucore` | `ucore` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucore.json](../../UETD/registry/modules/manifest/ucore.json) |
| `ud_kf` | `ucon/future/estimators/kalman/ud_kf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/kalman/ud_kf.json](../../UETD/registry/modules/manifest/ucon/future/estimators/kalman/ud_kf.json) |
| `uhal` | `uhal` | no authoring contract | contract not yet authored | [registry/modules/manifest/uhal.json](../../UETD/registry/modules/manifest/uhal.json) |
| `uio` | `ucon/future/estimators/observers/uio` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/future/estimators/observers/uio.json](../../UETD/registry/modules/manifest/ucon/future/estimators/observers/uio.json) |
| `ukf` | `ucon/estimators/kalman/ukf` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/ukf.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/ukf.json) |
| `ukf_merwe` | `ucon/estimators/kalman/ukf_merwe` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/kalman/ukf_merwe.json](../../UETD/registry/modules/manifest/ucon/estimators/kalman/ukf_merwe.json) |
| `umath_core` | `umath/core` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/core.json](../../UETD/registry/modules/manifest/umath/core.json) |
| `unit_delay` | `ucon/infrastructure/blocks/unit_delay` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/infrastructure/unit_delay.json](../../UETD/registry/modules/manifest/ucon/infrastructure/unit_delay.json) |
| `uos` | `uos` | no authoring contract | contract not yet authored | [registry/modules/manifest/uos.json](../../UETD/registry/modules/manifest/uos.json) |
| `usb` | `upal/usb` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/serial/usb.json](../../UETD/registry/modules/manifest/upal/serial/usb.json) |
| `usb_device` | `uproto/usb_device` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/usb_device.json](../../UETD/registry/modules/manifest/uproto/usb_device.json) |
| `vector` | `umath/vector/vector` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/vector/vector.json](../../UETD/registry/modules/manifest/umath/vector/vector.json) |
| `vector2` | `umath/vector/vector2` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/vector/vector2.json](../../UETD/registry/modules/manifest/umath/vector/vector2.json) |
| `vector3` | `umath/vector/vector3` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/vector/vector3.json](../../UETD/registry/modules/manifest/umath/vector/vector3.json) |
| `vector4` | `umath/vector/vector4` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/vector/vector4.json](../../UETD/registry/modules/manifest/umath/vector/vector4.json) |
| `ventilator_control_profile` | `ucon/medical/profiles/ventilator_control_profile` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/medical/profiles/ventilator_control_profile.json](../../UETD/registry/modules/manifest/ucon/medical/profiles/ventilator_control_profile.json) |
| `voltage` | `umid/voltage` | no authoring contract | contract not yet authored | [registry/modules/manifest/umid/voltage.json](../../UETD/registry/modules/manifest/umid/voltage.json) |
| `watchdog` | `upal/iwdg` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/watchdog/iwdg.json](../../UETD/registry/modules/manifest/upal/watchdog/iwdg.json) |
| `watchdog_gate` | `uapp/watchdog_gate` | no authoring contract | contract not yet authored | [registry/modules/manifest/uapp/runtime/watchdog_gate.json](../../UETD/registry/modules/manifest/uapp/runtime/watchdog_gate.json) |
| `wifi` | `uproto/wifi` | no authoring contract | contract not yet authored | [registry/modules/manifest/uproto/network/wifi.json](../../UETD/registry/modules/manifest/uproto/network/wifi.json) |
| `window_watchdog` | `upal/wwdg` | no authoring contract | contract not yet authored | [registry/modules/manifest/upal/watchdog/wwdg.json](../../UETD/registry/modules/manifest/upal/watchdog/wwdg.json) |
| `windows` | `umath/signal/windows` | no authoring contract | contract not yet authored | [registry/modules/manifest/umath/signal/windows.json](../../UETD/registry/modules/manifest/umath/signal/windows.json) |
| `x86` | `uhal/x86` | no authoring contract | contract not yet authored | [registry/modules/manifest/uhal/x86.json](../../UETD/registry/modules/manifest/uhal/x86.json) |
| `zero_cross_freq` | `ucon/estimators/frequency/zero_cross_freq` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/estimators/frequency/zero_cross_freq.json](../../UETD/registry/modules/manifest/ucon/estimators/frequency/zero_cross_freq.json) |
| `zone_mpc` | `ucon/control/predictive/zone_mpc` | no authoring contract | contract not yet authored | [registry/modules/manifest/ucon/control/predictive/zone_mpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/zone_mpc.json) |

## Module authoring contracts (4)

Detailed settings, ports, operations, constraints, and generation routes are shown only when the linked UETD manifest has an `authoring` contract. The complete JSON is included for review.

<details>
<summary>enmpc — ucon/control/predictive/enmpc · algorithm · authoring draft · generation draft</summary>

```json
{
  "composition": {
    "allowed_child_kinds": [],
    "required_blocks": [],
    "supports_blending": false,
    "supports_custom_parent": false,
    "supports_hybrid_mode": false,
    "supports_mode_switching": false,
    "supports_multi_rate": false,
    "supports_state_transfer": false
  },
  "contract_version": "1.0",
  "description": "Evaluate a precomputed nonlinear-MPC policy table over its declared state-feature domain and return the selected bounded command/trajectory together with table validity and region metadata. This is an algorithm, not a fallback or fusion strategy.",
  "display_name": "Explicit nonlinear model-predictive control",
  "generation": {
    "notes": [
      "The project hybrid_algorithm declaration composes enmpc with nmpc_rti or other compatible members; that composition is not part of this module.",
      "Supply the explicit policy data and validate table coverage, model feature mapping, and worst-case execution time for the selected target."
    ],
    "route_id": "uetd_enmpc",
    "status": "draft",
    "template_paths": []
  },
  "kind": "algorithm",
  "numerics": {
    "constraints": [
      "The selected state-feature map, table region data, command dimensions, and validity envelope are project/model-specific.",
      "Policy-table evaluation must use fixed storage and return explicit validity metadata; the enclosing hybrid policy owns fallback selection."
    ],
    "scalar_types": [
      "float32",
      "float64"
    ]
  },
  "operations": [],
  "parameters": [],
  "ports": [],
  "scheduling": {
    "contexts": [
      "task"
    ]
  },
  "status": "draft"
}
```

Source: [registry/modules/manifest/ucon/control/predictive/enmpc.json](../../UETD/registry/modules/manifest/ucon/control/predictive/enmpc.json).

</details>

<details>
<summary>fpga — umid/fpga · module · authoring reviewed · generation ready</summary>

```json
{
  "composition": {
    "required_blocks": [],
    "supports_blending": false,
    "supports_mode_switching": false,
    "supports_multi_rate": false
  },
  "contract_version": "1.0",
  "description": "UETD-owned MCU-side endpoint state for a project-declared protocol. The protocol defines typed messages, framing, timing, and validity rules; UETD builds the MCU codec and this module tracks link qualification and valid received data. It defines no peer identity handshake, hardware driver, or FPGA RTL.",
  "display_name": "FPGA communications endpoint",
  "generation": {
    "notes": [
      "UETD derives typed send/receive message ports from the project's declared protocol and selected .uefs link role.",
      "Every active wire field maps explicitly to a logical-record field; every active message is bound once by a component assignment or operating-system channel.",
      "UETD generates the MCU-side encoder/parser, protocol conversions, and receiver-local message timing from those declarations.",
      "The external FPGA design and its RTL/bitstream remain consumer-authored.",
      "This module does not add a contract-ID/hash handshake or select an identity/hash encoding."
    ],
    "route_id": "umid_fpga",
    "status": "ready",
    "template_paths": []
  },
  "kind": "module",
  "numerics": {
    "constraints": [
      "Freshness is evaluated only in the receiving MCU's monotonic clock domain.",
      "The generated protocol parser reports an update only after its declared framing, sequence, message, transaction, and integrity checks pass.",
      "A link drop or expired protocol validity invalidates cached data before the application can use it.",
      "Message fields and their wire encoding are supplied by the project protocol; this module defines no identity, contract-ID, or contract-hash fields."
    ]
  },
  "operations": [
    {
      "arguments": [
        {
          "description": "Whether the transport/protocol currently considers the FPGA endpoint link-qualified.",
          "id": "qualified",
          "value_type": "bool"
        }
      ],
      "description": "Update local link qualification from the selected transport/protocol; qualification loss clears cached valid data.",
      "id": "set_link_qualified",
      "result": "status",
      "symbol": "umid_fpga_set_link_qualified"
    },
    {
      "arguments": [
        {
          "description": "Monotonic timestamp from the receiving MCU.",
          "id": "received_at_us",
          "unit": "us",
          "value_type": "u64"
        }
      ],
      "description": "Record the receiver-local time of a complete message or transaction after the generated parser validates it.",
      "id": "note_valid_message",
      "result": "status",
      "symbol": "umid_fpga_note_valid_message"
    },
    {
      "description": "Invalidate cached peer data after timeout, malformed input, reset, or protocol-defined session loss.",
      "id": "invalidate_data",
      "result": "void",
      "symbol": "umid_fpga_invalidate_data"
    },
    {
      "arguments": [
        {
          "description": "Current time from the same receiving MCU monotonic clock.",
          "id": "now_us",
          "unit": "us",
          "value_type": "u64"
        },
        {
          "description": "Maximum permitted age for the selected protocol data.",
          "id": "maximum_age_us",
          "unit": "us",
          "value_type": "u64"
        }
      ],
      "description": "Check the last validated update against an application or protocol age limit using the receiver-local clock.",
      "id": "is_data_fresh",
      "result": "status",
      "symbol": "umid_fpga_is_data_fresh"
    }
  ],
  "parameters": [],
  "ports": [
    {
      "description": "True while the selected MCU transport/protocol reports the external FPGA endpoint as link-qualified.",
      "dimension": 1,
      "direction": "output",
      "id": "connected",
      "optional": false,
      "units": "1",
      "value_type": "bool"
    },
    {
      "description": "True after at least one complete validated protocol update and until the protocol timeout, link loss, parser fault, or reset invalidates it.",
      "dimension": 1,
      "direction": "output",
      "id": "has_valid_data",
      "optional": false,
      "units": "1",
      "value_type": "bool"
    },
    {
      "description": "Receiver-local monotonic timestamp of the most recent complete validated update.",
      "dimension": 1,
      "direction": "output",
      "id": "last_valid_message_at_us",
      "optional": false,
      "units": "us",
      "value_type": "u64"
    }
  ],
  "protocol_endpoint_binding": {
    "binding_forms": [
      "component_assignment",
      "operating_system_channel"
    ],
    "endpoint_role_source": "system_link_contract",
    "freshness_namespace": "fresh",
    "logical_payload_type_source": "imported_protocol_message_contract",
    "protocol_source": "project_or_imported_contract",
    "receive_namespace": "receive",
    "receive_timestamp": "receiver_local_monotonic_clock",
    "require_explicit_message_bindings": true,
    "require_wire_field_application_mapping": true,
    "send_namespace": "send",
    "unbound_message_policy": "error"
  },
  "scheduling": {
    "contexts": [
      "task",
      "main"
    ]
  },
  "status": "reviewed"
}
```

Source: [registry/modules/manifest/umid/fpga.json](../../UETD/registry/modules/manifest/umid/fpga.json).

</details>

<details>
<summary>h_bridge — uactuator/h_bridge · actuator · authoring draft · generation draft</summary>

```json
{
  "actuator_capabilities": {
    "safe_state_ids": []
  },
  "composition": {
    "required_blocks": [],
    "supports_blending": false,
    "supports_mode_switching": true,
    "supports_multi_rate": false
  },
  "contract_version": "1.0",
  "description": "Target-independent identity for a bidirectional power-stage actuator. The user's handwritten UETD target driver owns the electrical topology, pin mapping, timer behavior, and independent inhibit path.",
  "display_name": "H-bridge actuator",
  "generation": {
    "notes": [
      "TODO: Define the reviewed authoring contract before allowing project validation.",
      "TODO: Bind a manually implemented UETD target driver without embedding driver source in .uef."
    ],
    "route_id": "uetd_actuator_binding",
    "status": "draft",
    "template_paths": []
  },
  "kind": "actuator",
  "numerics": {
    "constraints": [
      "TODO: Define command units, range, and update semantics from the selected actuator and plant.",
      "TODO: Define the safe disabled state and hardware-inhibit relationship for each target driver."
    ],
    "scalar_types": [
      "float32",
      "float64"
    ]
  },
  "operations": [],
  "parameters": [],
  "ports": [],
  "scheduling": {
    "contexts": [
      "task"
    ]
  },
  "status": "draft"
}
```

Source: [registry/modules/manifest/uactuator/h_bridge.json](../../UETD/registry/modules/manifest/uactuator/h_bridge.json).

</details>

<details>
<summary>pid — ucon/control/classical/pid · algorithm · authoring draft · generation draft</summary>

```json
{
  "composition": {
    "required_blocks": [],
    "supports_blending": false,
    "supports_mode_switching": true,
    "supports_multi_rate": false
  },
  "contract_version": "1.0",
  "description": "Discrete Tustin PID/PI/PD controller with explicit saturation and anti-windup configuration.",
  "display_name": "Two-degree-of-freedom PID",
  "documentation": [
    "include/ucon/control/classical/pid.h"
  ],
  "generation": {
    "notes": [
      "Standalone component rendering and target-specific source/link checks are not yet verified."
    ],
    "route_id": "uetd_algorithm_instance",
    "status": "draft",
    "template_paths": []
  },
  "kind": "algorithm",
  "numerics": {
    "accumulator_types": [
      "float32",
      "float64"
    ],
    "constraints": [
      "All coefficients and limits must be finite.",
      "output_max must exceed output_min.",
      "integral_max must not be below integral_min.",
      "sample_time must be strictly positive."
    ],
    "fixed_point_formats": [],
    "scalar_types": [
      "float32",
      "float64",
      "q15",
      "q31"
    ]
  },
  "operations": [
    {
      "description": "Reset PID history and integral state before enabling or switching modes.",
      "id": "reset",
      "result": "status",
      "symbol": "ucon_pid_reset"
    }
  ],
  "parameters": [
    {
      "description": "Proportional gain.",
      "destination": "config_params.kp",
      "id": "kp",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Integral gain.",
      "destination": "config_params.ki",
      "id": "ki",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Derivative gain.",
      "destination": "config_params.kd",
      "id": "kd",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Derivative filter coefficient N.",
      "destination": "config_params.derivative_filter_n",
      "id": "derivative_filter_n",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Two-degree-of-freedom proportional setpoint weight.",
      "destination": "config_params.proportional_weight_b",
      "id": "proportional_weight_b",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Two-degree-of-freedom derivative setpoint weight.",
      "destination": "config_params.derivative_weight_c",
      "id": "derivative_weight_c",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Fixed controller sample period in seconds; must be strictly positive.",
      "destination": "config_params.sample_time",
      "exclusive_minimum": 0,
      "id": "sample_time",
      "required": true,
      "unit": "s",
      "value_type": "number"
    },
    {
      "description": "Lower output saturation limit.",
      "destination": "config_params.output_min",
      "id": "output_min",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Upper output saturation limit; must exceed output_min.",
      "destination": "config_params.output_max",
      "id": "output_max",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Lower integral-state limit.",
      "destination": "config_params.integral_min",
      "id": "integral_min",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Upper integral-state limit; must not be below integral_min.",
      "destination": "config_params.integral_max",
      "id": "integral_max",
      "required": true,
      "value_type": "number"
    },
    {
      "description": "Back-calculation tracking time in seconds; cross-field requirements follow the UETD API contract.",
      "destination": "config_params.tracking_time",
      "id": "tracking_time",
      "minimum": 0,
      "required": true,
      "unit": "s",
      "value_type": "number"
    },
    {
      "description": "Enable output saturation.",
      "destination": "config_params.saturation_enabled",
      "id": "saturation_enabled",
      "required": true,
      "value_type": "boolean"
    },
    {
      "description": "Anti-windup strategy.",
      "destination": "config_params.anti_windup",
      "enum_values": [
        "none",
        "clamp",
        "back_calculation",
        "conditional"
      ],
      "id": "anti_windup",
      "required": true,
      "value_type": "enum"
    }
  ],
  "ports": [
    {
      "description": "Setpoint input.",
      "dimension": 1,
      "direction": "input",
      "id": "reference",
      "optional": false,
      "units": "signal-defined",
      "value_type": "scalar"
    },
    {
      "description": "Measured process value.",
      "dimension": 1,
      "direction": "input",
      "id": "measurement",
      "optional": false,
      "units": "signal-defined",
      "value_type": "scalar"
    },
    {
      "description": "Optional additive feedforward input.",
      "dimension": 1,
      "direction": "input",
      "id": "feedforward",
      "optional": true,
      "units": "output-defined",
      "value_type": "scalar"
    },
    {
      "description": "Optional external tracking signal for back-calculation.",
      "dimension": 1,
      "direction": "input",
      "id": "tracking_output",
      "optional": true,
      "units": "output-defined",
      "value_type": "scalar"
    },
    {
      "description": "Controller output.",
      "dimension": 1,
      "direction": "output",
      "id": "output",
      "optional": false,
      "units": "output-defined",
      "value_type": "scalar"
    }
  ],
  "scheduling": {
    "contexts": [
      "task",
      "isr"
    ]
  },
  "status": "draft"
}
```

Source: [registry/modules/manifest/ucon/control/classical/pid.json](../../UETD/registry/modules/manifest/ucon/control/classical/pid.json).

</details>

## Public UETD C types and enums

Currently indexed declarations: 482 (436 public-header, 46 future-header). Explicit DSL spellings: 20; UMATH value candidates: 0.

These declarations are indexed from UETD public headers. `DSL type` is populated only when UETD explicitly exposes a project-language spelling. Header presence or autocomplete visibility does not grant permission to use a type as project data; its visibility and owning module contract determine that. `future_header` entries are discoverable drafts, not generation-ready API.

| DSL type | C type | Kind | DSL visibility | Source status | Header |
|---|---|---|---|---|---|
| `—` | `uactuator_class_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uactuator/actuator.h](../../UETD/include/uactuator/actuator.h) |
| `—` | `uactuator_command_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uactuator/actuator.h](../../UETD/include/uactuator/actuator.h) |
| `—` | `uactuator_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uactuator/actuator.h](../../UETD/include/uactuator/actuator.h) |
| `—` | `uactuator_feedback_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uactuator/actuator.h](../../UETD/include/uactuator/actuator.h) |
| `—` | `uactuator_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uactuator/actuator.h](../../UETD/include/uactuator/actuator.h) |
| `—` | `uapp_command_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_component_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/lifecycle.h](../../UETD/include/uapp/runtime/lifecycle.h) |
| `—` | `uapp_crash_backend_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/fault_history.h](../../UETD/include/uapp/diagnostics/fault_history.h) |
| `—` | `uapp_crash_record_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/diagnostics/fault_history.h](../../UETD/include/uapp/diagnostics/fault_history.h) |
| `—` | `uapp_crash_record_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/fault_history.h](../../UETD/include/uapp/diagnostics/fault_history.h) |
| `—` | `uapp_diagnostic_record_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/diagnostics.h](../../UETD/include/uapp/diagnostics/diagnostics.h) |
| `—` | `uapp_diagnostics_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/diagnostics.h](../../UETD/include/uapp/diagnostics/diagnostics.h) |
| `—` | `uapp_diagnostics_provider_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/diagnostics.h](../../UETD/include/uapp/diagnostics/diagnostics.h) |
| `—` | `uapp_diagnostics_slot_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/diagnostics.h](../../UETD/include/uapp/diagnostics/diagnostics.h) |
| `—` | `uapp_diagnostics_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/diagnostics.h](../../UETD/include/uapp/diagnostics/diagnostics.h) |
| `—` | `uapp_fault_record_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/fault.h](../../UETD/include/uapp/diagnostics/fault.h) |
| `—` | `uapp_fault_recovery_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/diagnostics/fault.h](../../UETD/include/uapp/diagnostics/fault.h) |
| `—` | `uapp_image_segment_role_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_maintenance_request_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/diagnostics/diagnostics.h](../../UETD/include/uapp/diagnostics/diagnostics.h) |
| `—` | `uapp_product_command_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_product_identity_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_product_outcome_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_product_provider_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_product_session_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_product_session_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_product_slot_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/product_session.h](../../UETD/include/uapp/product_session.h) |
| `—` | `uapp_sm_action_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_fault_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_guard_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_state_hook_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_state_hooks_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_state_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_transition_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_sm_unhandled_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/runtime/lifecycle.h](../../UETD/include/uapp/runtime/lifecycle.h) |
| `—` | `uapp_statemachine_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/statemachine.h](../../UETD/include/uapp/runtime/statemachine.h) |
| `—` | `uapp_supervisor_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/supervisor.h](../../UETD/include/uapp/runtime/supervisor.h) |
| `—` | `uapp_supervisor_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/supervisor.h](../../UETD/include/uapp/runtime/supervisor.h) |
| `—` | `uapp_supervisor_reason_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/runtime/supervisor.h](../../UETD/include/uapp/runtime/supervisor.h) |
| `—` | `uapp_supervisor_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/supervisor.h](../../UETD/include/uapp/runtime/supervisor.h) |
| `—` | `uapp_update_backend_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_boot_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_capabilities_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_descriptor_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_policy_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_progress_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_segment_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_session_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_update_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/secure_update.h](../../UETD/include/uapp/secure_update.h) |
| `—` | `uapp_watchdog_gate_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `uapp_watchdog_gate_event_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `uapp_watchdog_gate_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `uapp_watchdog_gate_reason_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `uapp_watchdog_gate_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `uapp_watchdog_lease_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `uapp_watchdog_refresh_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uapp/runtime/watchdog_gate.h](../../UETD/include/uapp/runtime/watchdog_gate.h) |
| `—` | `ucon_algorithm_scaffold_call_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucon/algorithm_call.h](../../UETD/include/ucon/algorithm_call.h) |
| `—` | `ucon_anti_windup_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/ucon/control/classical/pid.h](../../UETD/include/ucon/control/classical/pid.h) |
| `—` | `ucon_lead_lag_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucon/control/classical/lead_lag.h](../../UETD/include/ucon/control/classical/lead_lag.h) |
| `—` | `ucon_lead_lag_state_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucon/control/classical/lead_lag.h](../../UETD/include/ucon/control/classical/lead_lag.h) |
| `—` | `ucon_pid_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucon/control/classical/pid.h](../../UETD/include/ucon/control/classical/pid.h) |
| `—` | `ucon_pid_state_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucon/control/classical/pid.h](../../UETD/include/ucon/control/classical/pid.h) |
| `—` | `ucon_scalar_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucon/types.h](../../UETD/include/ucon/types.h) |
| `—` | `ucon_status_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/ucon/status.h](../../UETD/include/ucon/status.h) |
| `—` | `ucore_clock_stamp_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/deadline.h](../../UETD/include/ucore/deadline.h) |
| `—` | `ucore_codec_order_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/ucore/codec.h](../../UETD/include/ucore/codec.h) |
| `—` | `ucore_codec_reader_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/codec.h](../../UETD/include/ucore/codec.h) |
| `—` | `ucore_codec_writer_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/codec.h](../../UETD/include/ucore/codec.h) |
| `—` | `ucore_deadline_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/deadline.h](../../UETD/include/ucore/deadline.h) |
| `—` | `ucore_read_span_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/span.h](../../UETD/include/ucore/span.h) |
| `—` | `ucore_write_span_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/span.h](../../UETD/include/ucore/span.h) |
| `—` | `uetd_dur_us_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/time.h](../../UETD/include/ucore/time.h) |
| `f32` | `uetd_f32_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `f64` | `uetd_f64_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `i16` | `uetd_i16_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `i32` | `uetd_i32_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `i64` | `uetd_i64_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `i8` | `uetd_i8_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `q15` | `uetd_q15_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `q16` | `uetd_q16_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `q31` | `uetd_q31_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `q63` | `uetd_q63_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `—` | `uetd_status_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/ucore/status.h](../../UETD/include/ucore/status.h) |
| `—` | `uetd_target_profile_id_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uhal/target_profiles.h](../../UETD/include/uhal/target_profiles.h) |
| `—` | `uetd_target_profile_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/target_profiles.h](../../UETD/include/uhal/target_profiles.h) |
| `—` | `uetd_time_us_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/ucore/time.h](../../UETD/include/ucore/time.h) |
| `u16` | `uetd_u16_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `u32` | `uetd_u32_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `u64` | `uetd_u64_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `u8` | `uetd_u8_t` | alias | registered numeric type | public_header | [include/ucore/types.h](../../UETD/include/ucore/types.h) |
| `—` | `uhal_clk_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/clock.h](../../UETD/include/uhal/clock.h) |
| `—` | `uhal_critical_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/irq.h](../../UETD/include/uhal/irq.h) |
| `—` | `uhal_fault_context_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/fault.h](../../UETD/include/uhal/fault.h) |
| `—` | `uhal_gpio_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/gpio.h](../../UETD/include/uhal/gpio.h) |
| `—` | `uhal_gpio_mode_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uhal/gpio.h](../../UETD/include/uhal/gpio.h) |
| `—` | `uhal_gpio_pin_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/gpio.h](../../UETD/include/uhal/gpio.h) |
| `—` | `uhal_gpio_pull_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uhal/gpio.h](../../UETD/include/uhal/gpio.h) |
| `—` | `uhal_gpio_speed_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uhal/gpio.h](../../UETD/include/uhal/gpio.h) |
| `—` | `uhal_irq_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/target.h](../../UETD/include/uhal/target.h) |
| `—` | `uhal_mem_region_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/memory.h](../../UETD/include/uhal/memory.h) |
| `—` | `uhal_periph_clk_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uhal/clock.h](../../UETD/include/uhal/clock.h) |
| `—` | `uhal_reset_cause_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uhal/clock.h](../../UETD/include/uhal/clock.h) |
| `—` | `umath_accumulator_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/config.h](../../UETD/include/umath/config.h) |
| `complex` | `umath_complex_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `—` | `umath_csr_matrix_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/umath/matrix/sparse_csr.h](../../UETD/include/umath/matrix/sparse_csr.h) |
| `dual` | `umath_dual_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `—` | `umath_fft_plan_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/signal/fft.h](../../UETD/include/umath/signal/fft.h) |
| `—` | `umath_jacobian_scheme_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umath/derivatives/jacobian.h](../../UETD/include/umath/derivatives/jacobian.h) |
| `—` | `umath_ode_rhs_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/umath/numerical/integration.h](../../UETD/include/umath/numerical/integration.h) |
| `—` | `umath_pose2_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/lie/se2.h](../../UETD/include/umath/lie/se2.h) |
| `—` | `umath_prng_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/probability/prng.h](../../UETD/include/umath/probability/prng.h) |
| `quaternion` | `umath_quaternion_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `—` | `umath_scalar_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/config.h](../../UETD/include/umath/config.h) |
| `—` | `umath_status_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umath/status.h](../../UETD/include/umath/status.h) |
| `—` | `umath_transform3_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `vec2` | `umath_vec2_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `vec3` | `umath_vec3_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `vec4` | `umath_vec4_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `—` | `umath_vector_function_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/umath/derivatives/jacobian.h](../../UETD/include/umath/derivatives/jacobian.h) |
| `—` | `umid_blackbox_backend_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_blackbox_channel_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_blackbox_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_blackbox_overflow_policy_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_blackbox_record_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_blackbox_stats_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_blackbox_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/blackbox.h](../../UETD/include/umid/logging/blackbox.h) |
| `—` | `umid_calibration_record_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/calibration.h](../../UETD/include/umid/data/calibration.h) |
| `—` | `umid_current_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/current.h](../../UETD/include/umid/drivers/current.h) |
| `—` | `umid_encoder_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/encoder.h](../../UETD/include/umid/drivers/encoder.h) |
| `—` | `umid_encoder_type_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/drivers/encoder.h](../../UETD/include/umid/drivers/encoder.h) |
| `—` | `umid_event_bus_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/event_bus.h](../../UETD/include/umid/messaging/event_bus.h) |
| `—` | `umid_event_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/event_bus.h](../../UETD/include/umid/messaging/event_bus.h) |
| `—` | `umid_event_delivery_result_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/event_bus.h](../../UETD/include/umid/messaging/event_bus.h) |
| `—` | `umid_event_delivery_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/messaging/event_bus.h](../../UETD/include/umid/messaging/event_bus.h) |
| `—` | `umid_event_subscription_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/event_bus.h](../../UETD/include/umid/messaging/event_bus.h) |
| `—` | `umid_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/event_bus.h](../../UETD/include/umid/messaging/event_bus.h) |
| `—` | `umid_fpga_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/fpga.h](../../UETD/include/umid/drivers/fpga.h) |
| `—` | `umid_gnss_data_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/gnss.h](../../UETD/include/umid/drivers/gnss.h) |
| `—` | `umid_gnss_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/gnss.h](../../UETD/include/umid/drivers/gnss.h) |
| `—` | `umid_graphics_asset_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/ui/graphics.h](../../UETD/include/umid/ui/graphics.h) |
| `—` | `umid_graphics_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/ui/graphics.h](../../UETD/include/umid/ui/graphics.h) |
| `—` | `umid_graphics_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/ui/graphics.h](../../UETD/include/umid/ui/graphics.h) |
| `—` | `umid_health_entry_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/health.h](../../UETD/include/umid/health.h) |
| `—` | `umid_health_status_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/health.h](../../UETD/include/umid/health.h) |
| `—` | `umid_imu_bus_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/drivers/imu.h](../../UETD/include/umid/drivers/imu.h) |
| `—` | `umid_imu_data_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/imu.h](../../UETD/include/umid/drivers/imu.h) |
| `—` | `umid_imu_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/imu.h](../../UETD/include/umid/drivers/imu.h) |
| `—` | `umid_input_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/ui/input.h](../../UETD/include/umid/ui/input.h) |
| `—` | `umid_input_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/ui/input.h](../../UETD/include/umid/ui/input.h) |
| `—` | `umid_input_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/ui/input.h](../../UETD/include/umid/ui/input.h) |
| `—` | `umid_input_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/ui/input.h](../../UETD/include/umid/ui/input.h) |
| `—` | `umid_log_level_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/logging/log.h](../../UETD/include/umid/logging/log.h) |
| `—` | `umid_log_sink_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/logging/log.h](../../UETD/include/umid/logging/log.h) |
| `—` | `umid_log_write_record_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/umid/logging/log.h](../../UETD/include/umid/logging/log.h) |
| `—` | `umid_mag_data_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/magnetometer.h](../../UETD/include/umid/drivers/magnetometer.h) |
| `—` | `umid_magnetometer_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/magnetometer.h](../../UETD/include/umid/drivers/magnetometer.h) |
| `—` | `umid_mailbox_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/mailbox.h](../../UETD/include/umid/messaging/mailbox.h) |
| `—` | `umid_mailbox_metadata_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/mailbox.h](../../UETD/include/umid/messaging/mailbox.h) |
| `—` | `umid_mailbox_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/mailbox.h](../../UETD/include/umid/messaging/mailbox.h) |
| `—` | `umid_pool_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/buffer_pool.h](../../UETD/include/umid/messaging/buffer_pool.h) |
| `—` | `umid_pool_handle_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/buffer_pool.h](../../UETD/include/umid/messaging/buffer_pool.h) |
| `—` | `umid_pool_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/buffer_pool.h](../../UETD/include/umid/messaging/buffer_pool.h) |
| `—` | `umid_pressure_data_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/pressure.h](../../UETD/include/umid/drivers/pressure.h) |
| `—` | `umid_pressure_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/pressure.h](../../UETD/include/umid/drivers/pressure.h) |
| `—` | `umid_process_authority_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_command_semantics_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_direction_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_image_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_image_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_point_schema_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_point_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_quality_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_process_scan_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/process_image.h](../../UETD/include/umid/data/process_image.h) |
| `—` | `umid_range_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/range.h](../../UETD/include/umid/drivers/range.h) |
| `—` | `umid_range_type_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/drivers/range.h](../../UETD/include/umid/drivers/range.h) |
| `—` | `umid_ringbuf_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/messaging/ringbuf.h](../../UETD/include/umid/messaging/ringbuf.h) |
| `—` | `umid_sample_metadata_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/sensor.h](../../UETD/include/umid/sensor.h) |
| `—` | `umid_sample_quality_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/sensor.h](../../UETD/include/umid/sensor.h) |
| `—` | `umid_sensor_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/sensor.h](../../UETD/include/umid/sensor.h) |
| `—` | `umid_sensor_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/sensor.h](../../UETD/include/umid/sensor.h) |
| `—` | `umid_settings_activation_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/data/settings.h](../../UETD/include/umid/data/settings.h) |
| `—` | `umid_settings_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/settings.h](../../UETD/include/umid/data/settings.h) |
| `—` | `umid_settings_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/settings.h](../../UETD/include/umid/data/settings.h) |
| `—` | `umid_settings_transaction_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/settings.h](../../UETD/include/umid/data/settings.h) |
| `—` | `umid_store_backend_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/storage.h](../../UETD/include/umid/data/storage.h) |
| `—` | `umid_store_key_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/storage.h](../../UETD/include/umid/data/storage.h) |
| `—` | `umid_store_record_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/data/storage.h](../../UETD/include/umid/data/storage.h) |
| `—` | `umid_store_record_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/storage.h](../../UETD/include/umid/data/storage.h) |
| `—` | `umid_store_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/data/storage.h](../../UETD/include/umid/data/storage.h) |
| `—` | `umid_temp_type_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/umid/drivers/temperature.h](../../UETD/include/umid/drivers/temperature.h) |
| `—` | `umid_temperature_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/temperature.h](../../UETD/include/umid/drivers/temperature.h) |
| `—` | `umid_voltage_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/umid/drivers/voltage.h](../../UETD/include/umid/drivers/voltage.h) |
| `—` | `uos_clock_provider_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_deferred_callback_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uos/deferred.h](../../UETD/include/uos/deferred.h) |
| `—` | `uos_deferred_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/deferred.h](../../UETD/include/uos/deferred.h) |
| `—` | `uos_deferred_item_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/deferred.h](../../UETD/include/uos/deferred.h) |
| `—` | `uos_deferred_slot_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/deferred.h](../../UETD/include/uos/deferred.h) |
| `—` | `uos_deferred_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/deferred.h](../../UETD/include/uos/deferred.h) |
| `—` | `uos_deferred_token_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/deferred.h](../../UETD/include/uos/deferred.h) |
| `—` | `uos_dur_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_mutex_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_queue_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_sem_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_stack_word_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_status_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_task_fn_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_task_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `uos_time_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/uos/uos.h](../../UETD/include/uos/uos.h) |
| `—` | `upal_adc_channel_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/adc.h](../../UETD/include/upal/analog/adc.h) |
| `—` | `upal_adc_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/adc.h](../../UETD/include/upal/analog/adc.h) |
| `—` | `upal_adc_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/adc.h](../../UETD/include/upal/analog/adc.h) |
| `—` | `upal_block_device_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/block_device.h](../../UETD/include/upal/storage/block_device.h) |
| `—` | `upal_block_geometry_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/block_device.h](../../UETD/include/upal/storage/block_device.h) |
| `—` | `upal_block_transfer_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/block_device.h](../../UETD/include/upal/storage/block_device.h) |
| `—` | `upal_can_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/can.h](../../UETD/include/upal/can/can.h) |
| `—` | `upal_can_frame_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/can.h](../../UETD/include/upal/can/can.h) |
| `—` | `upal_can_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/can.h](../../UETD/include/upal/can/can.h) |
| `—` | `upal_can_rx_cb_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/can/can.h](../../UETD/include/upal/can/can.h) |
| `—` | `upal_can_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/can.h](../../UETD/include/upal/can/can.h) |
| `—` | `upal_capture_delivery_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_capture_edge_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_capture_overflow_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_comp_cb_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/analog/comp.h](../../UETD/include/upal/analog/comp.h) |
| `—` | `upal_comp_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/comp.h](../../UETD/include/upal/analog/comp.h) |
| `—` | `upal_comp_inm_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/analog/comp.h](../../UETD/include/upal/analog/comp.h) |
| `—` | `upal_comp_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/comp.h](../../UETD/include/upal/analog/comp.h) |
| `—` | `upal_dac_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/dac.h](../../UETD/include/upal/analog/dac.h) |
| `—` | `upal_dac_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/dac.h](../../UETD/include/upal/analog/dac.h) |
| `—` | `upal_display_backend_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_completion_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_format_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_info_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_outcome_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_region_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_request_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_display_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display.h](../../UETD/include/upal/display/display.h) |
| `—` | `upal_dma_callback_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dma_dir_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dma_event_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dma_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dma_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dma_width_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dma_xfer_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/dma.h](../../UETD/include/upal/dma.h) |
| `—` | `upal_dsi_c_encoding_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_clock_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_host_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_mode_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_phy_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_protocol_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_transfer_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_dsi_transfer_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/dsi_host.h](../../UETD/include/upal/display/dsi_host.h) |
| `—` | `upal_ethernet_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/ethernet.h](../../UETD/include/upal/network/ethernet.h) |
| `—` | `upal_ethernet_speed_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/network/ethernet.h](../../UETD/include/upal/network/ethernet.h) |
| `—` | `upal_ethernet_status_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/ethernet.h](../../UETD/include/upal/network/ethernet.h) |
| `—` | `upal_ethernet_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/ethernet.h](../../UETD/include/upal/network/ethernet.h) |
| `—` | `upal_fdcan_fd_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/fdcan_fd.h](../../UETD/include/upal/can/fdcan_fd.h) |
| `—` | `upal_fdcan_fd_frame_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/fdcan_fd.h](../../UETD/include/upal/can/fdcan_fd.h) |
| `—` | `upal_fdcan_fd_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/can/fdcan_fd.h](../../UETD/include/upal/can/fdcan_fd.h) |
| `—` | `upal_fmac_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/compute/fmac.h](../../UETD/include/upal/compute/fmac.h) |
| `—` | `upal_fmac_func_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/compute/fmac.h](../../UETD/include/upal/compute/fmac.h) |
| `—` | `upal_hrtim_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/hrtim.h](../../UETD/include/upal/timing/hrtim.h) |
| `—` | `upal_hrtim_pwm_cfg_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/hrtim.h](../../UETD/include/upal/timing/hrtim.h) |
| `—` | `upal_hrtim_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/hrtim.h](../../UETD/include/upal/timing/hrtim.h) |
| `—` | `upal_hrtim_timer_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/timing/hrtim.h](../../UETD/include/upal/timing/hrtim.h) |
| `—` | `upal_i2c_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/serial/i2c.h](../../UETD/include/upal/serial/i2c.h) |
| `—` | `upal_i2c_speed_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/serial/i2c.h](../../UETD/include/upal/serial/i2c.h) |
| `—` | `upal_i2c_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/serial/i2c.h](../../UETD/include/upal/serial/i2c.h) |
| `—` | `upal_ieee802154_address_mode_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_ieee802154_address_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_ieee802154_config_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_ieee802154_frame_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_ieee802154_ops_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_ieee802154_status_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_ieee802154_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/upal/network/ieee802154.h](../../UETD/include/upal/network/ieee802154.h) |
| `—` | `upal_netif_info_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/netif.h](../../UETD/include/upal/network/netif.h) |
| `—` | `upal_netif_link_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/netif.h](../../UETD/include/upal/network/netif.h) |
| `—` | `upal_netif_media_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/network/netif.h](../../UETD/include/upal/network/netif.h) |
| `—` | `upal_netif_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/netif.h](../../UETD/include/upal/network/netif.h) |
| `—` | `upal_netif_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/netif.h](../../UETD/include/upal/network/netif.h) |
| `—` | `upal_opamp_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/opamp.h](../../UETD/include/upal/analog/opamp.h) |
| `—` | `upal_opamp_mode_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/analog/opamp.h](../../UETD/include/upal/analog/opamp.h) |
| `—` | `upal_opamp_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/analog/opamp.h](../../UETD/include/upal/analog/opamp.h) |
| `—` | `upal_panel_profile_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display_panel.h](../../UETD/include/upal/display/display_panel.h) |
| `—` | `upal_panel_step_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/display_panel.h](../../UETD/include/upal/display/display_panel.h) |
| `—` | `upal_panel_step_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display_panel.h](../../UETD/include/upal/display/display_panel.h) |
| `—` | `upal_panel_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/display_panel.h](../../UETD/include/upal/display/display_panel.h) |
| `—` | `upal_power_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/power.h](../../UETD/include/upal/power.h) |
| `—` | `upal_power_lease_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/power.h](../../UETD/include/upal/power.h) |
| `—` | `upal_power_mode_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/power.h](../../UETD/include/upal/power.h) |
| `—` | `upal_power_status_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/power.h](../../UETD/include/upal/power.h) |
| `—` | `upal_power_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/power.h](../../UETD/include/upal/power.h) |
| `—` | `upal_qspi_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/qspi.h](../../UETD/include/upal/storage/qspi.h) |
| `—` | `upal_qspi_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/qspi.h](../../UETD/include/upal/storage/qspi.h) |
| `—` | `upal_radio_bus_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_exchange_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_poll_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_ready_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_receive_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_reset_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_send_fn` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_transport_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_radio_transport_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/network/radio_transport.h](../../UETD/include/upal/network/radio_transport.h) |
| `—` | `upal_rtc_date_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/rtc.h](../../UETD/include/upal/timing/rtc.h) |
| `—` | `upal_rtc_time_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/rtc.h](../../UETD/include/upal/timing/rtc.h) |
| `—` | `upal_sdmmc_card_info_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/sdmmc.h](../../UETD/include/upal/storage/sdmmc.h) |
| `—` | `upal_sdmmc_card_type_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/storage/sdmmc.h](../../UETD/include/upal/storage/sdmmc.h) |
| `—` | `upal_sdmmc_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/sdmmc.h](../../UETD/include/upal/storage/sdmmc.h) |
| `—` | `upal_sdmmc_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/storage/sdmmc.h](../../UETD/include/upal/storage/sdmmc.h) |
| `—` | `upal_sdmmc_width_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/storage/sdmmc.h](../../UETD/include/upal/storage/sdmmc.h) |
| `—` | `upal_spi_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/serial/spi.h](../../UETD/include/upal/serial/spi.h) |
| `—` | `upal_spi_mode_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/serial/spi.h](../../UETD/include/upal/serial/spi.h) |
| `—` | `upal_spi_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/serial/spi.h](../../UETD/include/upal/serial/spi.h) |
| `—` | `upal_sync_event_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_sync_group_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_sync_member_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_sync_member_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_timer_capture_callback_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_timer_capture_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_timer_capture_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_timer_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/timer.h](../../UETD/include/upal/timing/timer.h) |
| `—` | `upal_timer_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/timing/timer.h](../../UETD/include/upal/timing/timer.h) |
| `—` | `upal_touch_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/touch.h](../../UETD/include/upal/display/touch.h) |
| `—` | `upal_touch_contact_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/touch.h](../../UETD/include/upal/display/touch.h) |
| `—` | `upal_touch_frame_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/touch.h](../../UETD/include/upal/display/touch.h) |
| `—` | `upal_touch_phase_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/display/touch.h](../../UETD/include/upal/display/touch.h) |
| `—` | `upal_touch_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/display/touch.h](../../UETD/include/upal/display/touch.h) |
| `—` | `upal_transaction_completion_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/transaction.h](../../UETD/include/upal/transaction.h) |
| `—` | `upal_transaction_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/transaction.h](../../UETD/include/upal/transaction.h) |
| `—` | `upal_transaction_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/transaction.h](../../UETD/include/upal/transaction.h) |
| `—` | `upal_transaction_token_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/transaction.h](../../UETD/include/upal/transaction.h) |
| `—` | `upal_uart_advanced_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_de_release_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_duplex_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_echo_policy_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_flow_control_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_hw_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/serial/uart.h](../../UETD/include/upal/serial/uart.h) |
| `—` | `upal_uart_parity_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_stop_bits_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/serial/uart.h](../../UETD/include/upal/serial/uart.h) |
| `—` | `upal_uart_transfer_callback_t` | function_pointer | function pointer; unavailable to UEF-PL | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_uart_transfer_result_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_usb_profile_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_usb_profile_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `upal_usb_t` | alias | registered opaque type; C fields and constructors are not exposed | public_header | [include/upal/capability_contracts.h](../../UETD/include/upal/capability_contracts.h) |
| `—` | `uproto_bt_address_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_address_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_advertising_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_event_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_gatt_characteristic_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_gatt_service_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_security_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_bt_uuid_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/bluetooth.h](../../UETD/include/uproto/network/bluetooth.h) |
| `—` | `uproto_canopen_access_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_event_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_fd_config_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/canopen_fd.h](../../UETD/include/uproto/can/future/canopen_fd.h) |
| `—` | `uproto_canopen_fd_event_kind_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/canopen_fd.h](../../UETD/include/uproto/can/future/canopen_fd.h) |
| `—` | `uproto_canopen_fd_event_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/canopen_fd.h](../../UETD/include/uproto/can/future/canopen_fd.h) |
| `—` | `uproto_canopen_fd_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/canopen_fd.h](../../UETD/include/uproto/can/future/canopen_fd.h) |
| `—` | `uproto_canopen_fd_usdo_transaction_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/canopen_fd.h](../../UETD/include/uproto/can/future/canopen_fd.h) |
| `—` | `uproto_canopen_nmt_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_object_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_sdo_transaction_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_canopen_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_coap_method_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_coap_ops_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_coap_option_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_coap_request_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_coap_response_option_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_coap_response_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_coap_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/coap.h](../../UETD/include/uproto/network/future/coap.h) |
| `—` | `uproto_connection_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/connection_manager.h](../../UETD/include/uproto/network/connection_manager.h) |
| `—` | `uproto_connection_manager_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/connection_manager.h](../../UETD/include/uproto/network/connection_manager.h) |
| `—` | `uproto_connection_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/connection_manager.h](../../UETD/include/uproto/network/connection_manager.h) |
| `—` | `uproto_connection_status_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/connection_manager.h](../../UETD/include/uproto/network/connection_manager.h) |
| `—` | `uproto_crsf_rc_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/crsf.h](../../UETD/include/uproto/rc/crsf.h) |
| `—` | `uproto_crsf_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/crsf.h](../../UETD/include/uproto/rc/crsf.h) |
| `—` | `uproto_dshot_rate_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/rc/dshot.h](../../UETD/include/uproto/rc/dshot.h) |
| `—` | `uproto_dshot_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/dshot.h](../../UETD/include/uproto/rc/dshot.h) |
| `—` | `uproto_dshot_transport_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/rc/dshot.h](../../UETD/include/uproto/rc/dshot.h) |
| `—` | `uproto_framing_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/framing.h](../../UETD/include/uproto/framing.h) |
| `—` | `uproto_framing_profile_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/framing.h](../../UETD/include/uproto/framing.h) |
| `—` | `uproto_framing_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/framing.h](../../UETD/include/uproto/framing.h) |
| `—` | `uproto_future_call_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/scaffold_call.h](../../UETD/include/uproto/scaffold_call.h) |
| `—` | `uproto_http_header_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_http_method_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_http_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_http_request_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_http_response_header_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_http_response_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_http_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/http.h](../../UETD/include/uproto/network/http.h) |
| `—` | `uproto_ip_endpoint_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_ip_family_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_ip_interface_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_ip_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_ip_socket_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_ip_socket_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_ip_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/ip.h](../../UETD/include/uproto/network/ip.h) |
| `—` | `uproto_mavlink_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/mavlink.h](../../UETD/include/uproto/mavlink.h) |
| `—` | `uproto_medi_can_command_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_config_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_event_kind_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_event_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_identity_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_peer_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_result_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_snapshot_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_stage_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_medi_can_transaction_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/can/future/medi_can.h](../../UETD/include/uproto/can/future/medi_can.h) |
| `—` | `uproto_mesh_config_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_event_kind_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_event_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_ops_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_profile_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_role_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_state_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_mesh_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/mesh.h](../../UETD/include/uproto/network/future/mesh.h) |
| `—` | `uproto_modbus_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_function_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_mapping_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_role_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_transaction_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_modbus_transport_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/industrial/industrial_contracts.h](../../UETD/include/uproto/industrial/industrial_contracts.h) |
| `—` | `uproto_mqtt_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_mqtt_event_kind_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_mqtt_event_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_mqtt_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_mqtt_qos_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_mqtt_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_mqtt_version_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/mqtt.h](../../UETD/include/uproto/network/mqtt.h) |
| `—` | `uproto_ppm_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/ppm.h](../../UETD/include/uproto/rc/ppm.h) |
| `—` | `uproto_ppm_frame_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/ppm.h](../../UETD/include/uproto/rc/ppm.h) |
| `—` | `uproto_ppm_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/ppm.h](../../UETD/include/uproto/rc/ppm.h) |
| `—` | `uproto_ppp_auth_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/ppp.h](../../UETD/include/uproto/network/future/ppp.h) |
| `—` | `uproto_ppp_config_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/ppp.h](../../UETD/include/uproto/network/future/ppp.h) |
| `—` | `uproto_ppp_ops_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/ppp.h](../../UETD/include/uproto/network/future/ppp.h) |
| `—` | `uproto_ppp_state_t` | enum | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/ppp.h](../../UETD/include/uproto/network/future/ppp.h) |
| `—` | `uproto_ppp_status_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/ppp.h](../../UETD/include/uproto/network/future/ppp.h) |
| `—` | `uproto_ppp_t` | struct | future-only type; unavailable to UEF-PL | future_header | [include/uproto/network/future/ppp.h](../../UETD/include/uproto/network/future/ppp.h) |
| `—` | `uproto_sbus_frame_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/sbus.h](../../UETD/include/uproto/rc/sbus.h) |
| `—` | `uproto_sbus_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/rc/sbus.h](../../UETD/include/uproto/rc/sbus.h) |
| `—` | `uproto_stream_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/uproto.h](../../UETD/include/uproto/uproto.h) |
| `—` | `uproto_time_mapping_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/time_sync.h](../../UETD/include/uproto/network/time_sync.h) |
| `—` | `uproto_time_source_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/time_sync.h](../../UETD/include/uproto/network/time_sync.h) |
| `—` | `uproto_time_sync_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/time_sync.h](../../UETD/include/uproto/network/time_sync.h) |
| `—` | `uproto_time_sync_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/time_sync.h](../../UETD/include/uproto/network/time_sync.h) |
| `—` | `uproto_tls_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/tls.h](../../UETD/include/uproto/network/tls.h) |
| `—` | `uproto_tls_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/tls.h](../../UETD/include/uproto/network/tls.h) |
| `—` | `uproto_tls_profile_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/tls.h](../../UETD/include/uproto/network/tls.h) |
| `—` | `uproto_tls_session_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/tls.h](../../UETD/include/uproto/network/tls.h) |
| `—` | `uproto_tls_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/tls.h](../../UETD/include/uproto/network/tls.h) |
| `—` | `uproto_uavcan_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/can/uavcan.h](../../UETD/include/uproto/can/uavcan.h) |
| `—` | `uproto_usb_class_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/usb_device.h](../../UETD/include/uproto/usb_device.h) |
| `—` | `uproto_usb_device_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/usb_device.h](../../UETD/include/uproto/usb_device.h) |
| `—` | `uproto_usb_device_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/usb_device.h](../../UETD/include/uproto/usb_device.h) |
| `—` | `uproto_waveform_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/uproto.h](../../UETD/include/uproto/uproto.h) |
| `—` | `uproto_wifi_ap_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_link_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_ops_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_scan_result_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_security_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_state_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_station_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `uproto_wifi_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/uproto/network/wifi.h](../../UETD/include/uproto/network/wifi.h) |
| `—` | `usec_authorization_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/usec/provider.h](../../UETD/include/usec/provider.h) |
| `—` | `usec_config_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/usec/provider.h](../../UETD/include/usec/provider.h) |
| `—` | `usec_key_handle_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/usec/provider.h](../../UETD/include/usec/provider.h) |
| `—` | `usec_operation_t` | enum | registered enum; enumerators are available by qualified name | public_header | [include/usec/provider.h](../../UETD/include/usec/provider.h) |
| `—` | `usec_provider_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/usec/provider.h](../../UETD/include/usec/provider.h) |
| `—` | `usec_request_t` | struct | registered opaque type; C fields and constructors are not exposed | public_header | [include/usec/provider.h](../../UETD/include/usec/provider.h) |

## UMATH function declarations (331)

This index combines public header declarations with UETD-owned operation names, compatibility aliases, and binding status. Same-name functions are typed overloads selected from reviewed logical parameter types. The C symbol is shown below for source traceability. Callable scope is limited to custom algorithms and nonlinear model bodies, and generation requires an implemented function plus a reviewed DSL binding.

| DSL operation | Compatibility aliases | C symbol | Module | Implementation | DSL binding | Header |
|---|---|---|---|---|---|
| `add(signature pending)` | `complex_add` | `umath_complex_add` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `argument(signature pending)` | `complex_argument` | `umath_complex_argument` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `conjugate(signature pending)` | `complex_conjugate` | `umath_complex_conjugate` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `cos(signature pending)` | `complex_cos` | `umath_complex_cos` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `cosh(signature pending)` | `complex_cosh` | `umath_complex_cosh` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `div(signature pending)` | `complex_div` | `umath_complex_div` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `exp(signature pending)` | `complex_exp` | `umath_complex_exp` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `from_polar(signature pending)` | `complex_from_polar` | `umath_complex_from_polar` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `log(signature pending)` | `complex_log` | `umath_complex_log` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `magnitude(signature pending)` | `complex_magnitude` | `umath_complex_magnitude` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `mul(signature pending)` | `complex_mul` | `umath_complex_mul` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `pow(signature pending)` | `complex_pow` | `umath_complex_pow` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `sin(signature pending)` | `complex_sin` | `umath_complex_sin` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `sinh(signature pending)` | `complex_sinh` | `umath_complex_sinh` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `sqrt(signature pending)` | `complex_sqrt` | `umath_complex_sqrt` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `sub(signature pending)` | `complex_sub` | `umath_complex_sub` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `tan(signature pending)` | `complex_tan` | `umath_complex_tan` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `tanh(signature pending)` | `complex_tanh` | `umath_complex_tanh` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `to_polar(signature pending)` | `complex_to_polar` | `umath_complex_to_polar` | `umath/complex` | implemented | signature_only | [include/umath/complex.h](../../UETD/include/umath/complex.h) |
| `is_finite(signature pending)` | `accumulator_is_finite` | `umath_accumulator_is_finite` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `acos(signature pending)` | — | `umath_acos` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `asin(signature pending)` | — | `umath_asin` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `atan(signature pending)` | — | `umath_atan` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `atan2(y: scalar, x: scalar) -> scalar<rad>` | — | `umath_atan2` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `cos(radians: scalar<rad>) -> scalar` | — | `umath_cos` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `cosc(signature pending)` | — | `umath_cosc` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `cosh(signature pending)` | — | `umath_cosh` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `exp(value: scalar<1>) -> scalar` | — | `umath_exp` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `fma(signature pending)` | — | `umath_fma` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `hypot(signature pending)` | — | `umath_hypot` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `invsqrt(signature pending)` | — | `umath_invsqrt` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `ln(value: scalar<1>) -> scalar` | — | `umath_ln` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `abs(value: scalar) -> scalar` | `scalar_abs` | `umath_scalar_abs` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `acosh(signature pending)` | `scalar_acosh` | `umath_scalar_acosh` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `add(signature pending)` | `scalar_add` | `umath_scalar_add` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `asinh(signature pending)` | `scalar_asinh` | `umath_scalar_asinh` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `atanh(signature pending)` | `scalar_atanh` | `umath_scalar_atanh` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `cbrt(signature pending)` | `scalar_cbrt` | `umath_scalar_cbrt` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `ceil(signature pending)` | `scalar_ceil` | `umath_scalar_ceil` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `clamp(value: scalar, minimum: scalar, maximum: scalar) -> scalar` | `scalar_clamp` | `umath_scalar_clamp` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `copysign(signature pending)` | `scalar_copysign` | `umath_scalar_copysign` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `div(signature pending)` | `scalar_div` | `umath_scalar_div` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `exp2(signature pending)` | `scalar_exp2` | `umath_scalar_exp2` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `expm1(signature pending)` | `scalar_expm1` | `umath_scalar_expm1` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `floor(signature pending)` | `scalar_floor` | `umath_scalar_floor` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `fmod(signature pending)` | `scalar_fmod` | `umath_scalar_fmod` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `is_finite(signature pending)` | `scalar_is_finite` | `umath_scalar_is_finite` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `log10(signature pending)` | `scalar_log10` | `umath_scalar_log10` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `log1p(signature pending)` | `scalar_log1p` | `umath_scalar_log1p` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `log2(signature pending)` | `scalar_log2` | `umath_scalar_log2` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `max(left: scalar, right: scalar) -> scalar` | `scalar_max` | `umath_scalar_max` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `min(left: scalar, right: scalar) -> scalar` | `scalar_min` | `umath_scalar_min` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `modf(signature pending)` | `scalar_modf` | `umath_scalar_modf` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `mul(signature pending)` | `scalar_mul` | `umath_scalar_mul` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `neg(signature pending)` | `scalar_neg` | `umath_scalar_neg` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `pow(signature pending)` | `scalar_pow` | `umath_scalar_pow` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `powi(signature pending)` | `scalar_powi` | `umath_scalar_powi` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `reciprocal(signature pending)` | `scalar_reciprocal` | `umath_scalar_reciprocal` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `remainder(signature pending)` | `scalar_remainder` | `umath_scalar_remainder` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `round(signature pending)` | `scalar_round` | `umath_scalar_round` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sign(signature pending)` | `scalar_sign` | `umath_scalar_sign` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sub(signature pending)` | `scalar_sub` | `umath_scalar_sub` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `trunc(signature pending)` | `scalar_trunc` | `umath_scalar_trunc` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sin(radians: scalar<rad>) -> scalar` | — | `umath_sin` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sinc(signature pending)` | — | `umath_sinc` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sincc(signature pending)` | — | `umath_sincc` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sinh(signature pending)` | — | `umath_sinh` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `sqrt(value: scalar) -> scalar` | — | `umath_sqrt` | `umath/core` | implemented | ready | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `string(signature pending)` | `status_string` | `umath_status_string` | `umath/core` | implemented | signature_only | [include/umath/status.h](../../UETD/include/umath/status.h) |
| `tan(signature pending)` | — | `umath_tan` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `tanh(signature pending)` | — | `umath_tanh` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `wrap_angle_pi(signature pending)` | — | `umath_wrap_angle_pi` | `umath/core` | implemented | signature_only | [include/umath/scalar.h](../../UETD/include/umath/scalar.h) |
| `abs(signature pending)` | `dual_abs` | `umath_dual_abs` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `acos(signature pending)` | `dual_acos` | `umath_dual_acos` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `acosh(signature pending)` | `dual_acosh` | `umath_dual_acosh` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `add(signature pending)` | `dual_add` | `umath_dual_add` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `asin(signature pending)` | `dual_asin` | `umath_dual_asin` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `asinh(signature pending)` | `dual_asinh` | `umath_dual_asinh` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `atan(signature pending)` | `dual_atan` | `umath_dual_atan` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `atan2(signature pending)` | `dual_atan2` | `umath_dual_atan2` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `atanh(signature pending)` | `dual_atanh` | `umath_dual_atanh` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `cbrt(signature pending)` | `dual_cbrt` | `umath_dual_cbrt` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `ceil(signature pending)` | `dual_ceil` | `umath_dual_ceil` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `clamp(signature pending)` | `dual_clamp` | `umath_dual_clamp` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `constant(signature pending)` | `dual_constant` | `umath_dual_constant` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `cos(signature pending)` | `dual_cos` | `umath_dual_cos` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `cosh(signature pending)` | `dual_cosh` | `umath_dual_cosh` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `div(signature pending)` | `dual_div` | `umath_dual_div` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `exp(signature pending)` | `dual_exp` | `umath_dual_exp` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `exp2(signature pending)` | `dual_exp2` | `umath_dual_exp2` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `expm1(signature pending)` | `dual_expm1` | `umath_dual_expm1` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `floor(signature pending)` | `dual_floor` | `umath_dual_floor` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `fma(signature pending)` | `dual_fma` | `umath_dual_fma` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `fmod(signature pending)` | `dual_fmod` | `umath_dual_fmod` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `hypot(signature pending)` | `dual_hypot` | `umath_dual_hypot` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `invsqrt(signature pending)` | `dual_invsqrt` | `umath_dual_invsqrt` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `ln(signature pending)` | `dual_ln` | `umath_dual_ln` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `log10(signature pending)` | `dual_log10` | `umath_dual_log10` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `log1p(signature pending)` | `dual_log1p` | `umath_dual_log1p` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `log2(signature pending)` | `dual_log2` | `umath_dual_log2` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `max(signature pending)` | `dual_max` | `umath_dual_max` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `min(signature pending)` | `dual_min` | `umath_dual_min` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `mul(signature pending)` | `dual_mul` | `umath_dual_mul` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `neg(signature pending)` | `dual_neg` | `umath_dual_neg` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `pow(signature pending)` | `dual_pow` | `umath_dual_pow` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `powi(signature pending)` | `dual_powi` | `umath_dual_powi` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `reciprocal(signature pending)` | `dual_reciprocal` | `umath_dual_reciprocal` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `remainder(signature pending)` | `dual_remainder` | `umath_dual_remainder` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `round(signature pending)` | `dual_round` | `umath_dual_round` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `seed(signature pending)` | `dual_seed` | `umath_dual_seed` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `sin(signature pending)` | `dual_sin` | `umath_dual_sin` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `sinh(signature pending)` | `dual_sinh` | `umath_dual_sinh` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `sqrt(signature pending)` | `dual_sqrt` | `umath_dual_sqrt` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `sub(signature pending)` | `dual_sub` | `umath_dual_sub` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `tan(signature pending)` | `dual_tan` | `umath_dual_tan` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `tanh(signature pending)` | `dual_tanh` | `umath_dual_tanh` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `trunc(signature pending)` | `dual_trunc` | `umath_dual_trunc` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `wrap_angle_pi(signature pending)` | `dual_wrap_angle_pi` | `umath_dual_wrap_angle_pi` | `umath/derivatives/dual` | scaffold | signature_only | [include/umath/derivatives/dual.h](../../UETD/include/umath/derivatives/dual.h) |
| `finite_difference(signature pending)` | `jacobian_finite_difference` | `umath_jacobian_finite_difference` | `umath/derivatives/jacobian` | scaffold | signature_only | [include/umath/derivatives/jacobian.h](../../UETD/include/umath/derivatives/jacobian.h) |
| `workspace_count(signature pending)` | `jacobian_workspace_count` | `umath_jacobian_workspace_count` | `umath/derivatives/jacobian` | scaffold | signature_only | [include/umath/derivatives/jacobian.h](../../UETD/include/umath/derivatives/jacobian.h) |
| `abs_sat(signature pending)` | `q15_abs_sat` | `umath_q15_abs_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `add_sat(signature pending)` | `q15_add_sat` | `umath_q15_add_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `div_sat(signature pending)` | `q15_div_sat` | `umath_q15_div_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `from_scalar(signature pending)` | `q15_from_scalar` | `umath_q15_from_scalar` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `mul_sat(signature pending)` | `q15_mul_sat` | `umath_q15_mul_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `sqrt(signature pending)` | `q15_sqrt` | `umath_q15_sqrt` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `sub_sat(signature pending)` | `q15_sub_sat` | `umath_q15_sub_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_q16_sat(signature pending)` | `q15_to_q16_sat`, `convert_sat` | `umath_q15_to_q16_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_q31_sat(signature pending)` | `q15_to_q31_sat`, `convert_sat` | `umath_q15_to_q31_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_scalar(signature pending)` | `q15_to_scalar` | `umath_q15_to_scalar` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `abs_sat(signature pending)` | `q16_abs_sat` | `umath_q16_abs_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `add_sat(signature pending)` | `q16_add_sat` | `umath_q16_add_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `div_sat(signature pending)` | `q16_div_sat` | `umath_q16_div_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `from_scalar(signature pending)` | `q16_from_scalar` | `umath_q16_from_scalar` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `mul_sat(signature pending)` | `q16_mul_sat` | `umath_q16_mul_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `sqrt(signature pending)` | `q16_sqrt` | `umath_q16_sqrt` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `sub_sat(signature pending)` | `q16_sub_sat` | `umath_q16_sub_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_q15_sat(signature pending)` | `q16_to_q15_sat`, `convert_sat` | `umath_q16_to_q15_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_q31_sat(signature pending)` | `q16_to_q31_sat`, `convert_sat` | `umath_q16_to_q31_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_scalar(signature pending)` | `q16_to_scalar` | `umath_q16_to_scalar` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `abs_sat(signature pending)` | `q31_abs_sat` | `umath_q31_abs_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `add_sat(signature pending)` | `q31_add_sat` | `umath_q31_add_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `div_sat(signature pending)` | `q31_div_sat` | `umath_q31_div_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `from_scalar(signature pending)` | `q31_from_scalar` | `umath_q31_from_scalar` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `mul_sat(signature pending)` | `q31_mul_sat` | `umath_q31_mul_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `sqrt(signature pending)` | `q31_sqrt` | `umath_q31_sqrt` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `sub_sat(signature pending)` | `q31_sub_sat` | `umath_q31_sub_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_q15_sat(signature pending)` | `q31_to_q15_sat`, `convert_sat` | `umath_q31_to_q15_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_q16_sat(signature pending)` | `q31_to_q16_sat`, `convert_sat` | `umath_q31_to_q16_sat` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `to_scalar(signature pending)` | `q31_to_scalar` | `umath_q31_to_scalar` | `umath/fixed` | implemented | signature_only | [include/umath/fixed.h](../../UETD/include/umath/fixed.h) |
| `add(signature pending)` | `csr_add` | `umath_csr_add` | `umath/future/matrix/sparse_csr` | future | signature_only | [include/umath/matrix/sparse_csr.h](../../UETD/include/umath/matrix/sparse_csr.h) |
| `matvec(signature pending)` | `csr_matvec` | `umath_csr_matvec` | `umath/future/matrix/sparse_csr` | future | signature_only | [include/umath/matrix/sparse_csr.h](../../UETD/include/umath/matrix/sparse_csr.h) |
| `validate(signature pending)` | `csr_validate` | `umath_csr_validate` | `umath/future/matrix/sparse_csr` | future | signature_only | [include/umath/matrix/sparse_csr.h](../../UETD/include/umath/matrix/sparse_csr.h) |
| `conjugate(signature pending)` | `quaternion_conjugate` | `umath_quaternion_conjugate` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `dot(signature pending)` | `quaternion_dot` | `umath_quaternion_dot` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `exp(signature pending)` | `quaternion_exp` | `umath_quaternion_exp` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `from_axis_angle(signature pending)` | `quaternion_from_axis_angle` | `umath_quaternion_from_axis_angle` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `from_matrix(signature pending)` | `quaternion_from_matrix` | `umath_quaternion_from_matrix` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `identity(signature pending)` | `quaternion_identity` | `umath_quaternion_identity` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `log(signature pending)` | `quaternion_log` | `umath_quaternion_log` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `multiply(signature pending)` | `quaternion_multiply` | `umath_quaternion_multiply` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `norm(signature pending)` | `quaternion_norm` | `umath_quaternion_norm` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `normalize(signature pending)` | `quaternion_normalize` | `umath_quaternion_normalize` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `rotate(signature pending)` | `quaternion_rotate` | `umath_quaternion_rotate` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `slerp(signature pending)` | `quaternion_slerp` | `umath_quaternion_slerp` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `to_axis_angle(signature pending)` | `quaternion_to_axis_angle` | `umath_quaternion_to_axis_angle` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `to_matrix(signature pending)` | `quaternion_to_matrix` | `umath_quaternion_to_matrix` | `umath/geometry/quaternion` | implemented | signature_only | [include/umath/geometry/quaternion.h](../../UETD/include/umath/geometry/quaternion.h) |
| `compose(signature pending)` | `transform3_compose` | `umath_transform3_compose` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `from_matrix(signature pending)` | `transform3_from_matrix` | `umath_transform3_from_matrix` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `identity(signature pending)` | `transform3_identity` | `umath_transform3_identity` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `interpolate(signature pending)` | `transform3_interpolate` | `umath_transform3_interpolate` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `inverse(signature pending)` | `transform3_inverse` | `umath_transform3_inverse` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `point(signature pending)` | `transform3_point` | `umath_transform3_point` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `to_matrix(signature pending)` | `transform3_to_matrix` | `umath_transform3_to_matrix` | `umath/geometry/transform3` | implemented | signature_only | [include/umath/geometry/transform3.h](../../UETD/include/umath/geometry/transform3.h) |
| `bezier_evaluate(signature pending)` | — | `umath_bezier_evaluate` | `umath/interpolation/splines` | scaffold | signature_only | [include/umath/numerical/splines.h](../../UETD/include/umath/numerical/splines.h) |
| `basis(signature pending)` | `bspline_basis` | `umath_bspline_basis` | `umath/interpolation/splines` | scaffold | signature_only | [include/umath/numerical/splines.h](../../UETD/include/umath/numerical/splines.h) |
| `derivative(signature pending)` | `bspline_derivative` | `umath_bspline_derivative` | `umath/interpolation/splines` | scaffold | signature_only | [include/umath/numerical/splines.h](../../UETD/include/umath/numerical/splines.h) |
| `cubic_hermite(signature pending)` | — | `umath_cubic_hermite` | `umath/interpolation/splines` | scaffold | signature_only | [include/umath/numerical/splines.h](../../UETD/include/umath/numerical/splines.h) |
| `adjoint(signature pending)` | `se2_adjoint` | `umath_se2_adjoint` | `umath/lie/se2` | scaffold | signature_only | [include/umath/lie/se2.h](../../UETD/include/umath/lie/se2.h) |
| `exp(signature pending)` | `se2_exp` | `umath_se2_exp` | `umath/lie/se2` | scaffold | signature_only | [include/umath/lie/se2.h](../../UETD/include/umath/lie/se2.h) |
| `log(signature pending)` | `se2_log` | `umath_se2_log` | `umath/lie/se2` | scaffold | signature_only | [include/umath/lie/se2.h](../../UETD/include/umath/lie/se2.h) |
| `adjoint(signature pending)` | `se3_adjoint` | `umath_se3_adjoint` | `umath/lie/se3` | scaffold | signature_only | [include/umath/lie/se3.h](../../UETD/include/umath/lie/se3.h) |
| `exp(signature pending)` | `se3_exp` | `umath_se3_exp` | `umath/lie/se3` | scaffold | signature_only | [include/umath/lie/se3.h](../../UETD/include/umath/lie/se3.h) |
| `left_jacobian(signature pending)` | `se3_left_jacobian` | `umath_se3_left_jacobian` | `umath/lie/se3` | scaffold | signature_only | [include/umath/lie/se3.h](../../UETD/include/umath/lie/se3.h) |
| `left_jacobian_inverse(signature pending)` | `se3_left_jacobian_inverse` | `umath_se3_left_jacobian_inverse` | `umath/lie/se3` | scaffold | signature_only | [include/umath/lie/se3.h](../../UETD/include/umath/lie/se3.h) |
| `log(signature pending)` | `se3_log` | `umath_se3_log` | `umath/lie/se3` | scaffold | signature_only | [include/umath/lie/se3.h](../../UETD/include/umath/lie/se3.h) |
| `exp(signature pending)` | `so2_exp` | `umath_so2_exp` | `umath/lie/so2` | scaffold | signature_only | [include/umath/lie/so2.h](../../UETD/include/umath/lie/so2.h) |
| `log(signature pending)` | `so2_log` | `umath_so2_log` | `umath/lie/so2` | scaffold | signature_only | [include/umath/lie/so2.h](../../UETD/include/umath/lie/so2.h) |
| `exp(signature pending)` | `so3_exp` | `umath_so3_exp` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `hat(signature pending)` | `so3_hat` | `umath_so3_hat` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `left_jacobian(signature pending)` | `so3_left_jacobian` | `umath_so3_left_jacobian` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `left_jacobian_inverse(signature pending)` | `so3_left_jacobian_inverse` | `umath_so3_left_jacobian_inverse` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `log(signature pending)` | `so3_log` | `umath_so3_log` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `right_jacobian(signature pending)` | `so3_right_jacobian` | `umath_so3_right_jacobian` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `right_jacobian_inverse(signature pending)` | `so3_right_jacobian_inverse` | `umath_so3_right_jacobian_inverse` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `vee(signature pending)` | `so3_vee` | `umath_so3_vee` | `umath/lie/so3` | scaffold | signature_only | [include/umath/lie/so3.h](../../UETD/include/umath/lie/so3.h) |
| `factor(signature pending)` | `cholesky_factor` | `umath_cholesky_factor` | `umath/matrix/cholesky` | implemented | signature_only | [include/umath/matrix/cholesky.h](../../UETD/include/umath/matrix/cholesky.h) |
| `inverse(signature pending)` | `cholesky_inverse` | `umath_cholesky_inverse` | `umath/matrix/cholesky` | implemented | signature_only | [include/umath/matrix/cholesky.h](../../UETD/include/umath/matrix/cholesky.h) |
| `log_determinant(signature pending)` | `cholesky_log_determinant` | `umath_cholesky_log_determinant` | `umath/matrix/cholesky` | implemented | signature_only | [include/umath/matrix/cholesky.h](../../UETD/include/umath/matrix/cholesky.h) |
| `solve(signature pending)` | `cholesky_solve` | `umath_cholesky_solve` | `umath/matrix/cholesky` | implemented | signature_only | [include/umath/matrix/cholesky.h](../../UETD/include/umath/matrix/cholesky.h) |
| `add(signature pending)` | `matrix_add` | `umath_matrix_add` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `determinant(signature pending)` | `matrix_determinant` | `umath_matrix_determinant` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `determinant_workspace_count(signature pending)` | `matrix_determinant_workspace_count` | `umath_matrix_determinant_workspace_count` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `diagonal(signature pending)` | `matrix_diagonal` | `umath_matrix_diagonal` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `elementwise_mul(signature pending)` | `matrix_elementwise_mul` | `umath_matrix_elementwise_mul` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `exponential(signature pending)` | `matrix_exponential` | `umath_matrix_exponential` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `exponential_workspace_count(signature pending)` | `matrix_exponential_workspace_count` | `umath_matrix_exponential_workspace_count` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `identity(signature pending)` | `matrix_identity` | `umath_matrix_identity` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `inverse(signature pending)` | `matrix_inverse` | `umath_matrix_inverse` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `inverse_workspace_count(signature pending)` | `matrix_inverse_workspace_count` | `umath_matrix_inverse_workspace_count` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `multiply(signature pending)` | `matrix_multiply` | `umath_matrix_multiply` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `norm_frobenius(signature pending)` | `matrix_norm_frobenius` | `umath_matrix_norm_frobenius` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `norm_infinity(signature pending)` | `matrix_norm_infinity` | `umath_matrix_norm_infinity` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `norm_one(signature pending)` | `matrix_norm_one` | `umath_matrix_norm_one` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `outer_product(signature pending)` | `matrix_outer_product` | `umath_matrix_outer_product` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `rank_one_update(signature pending)` | `matrix_rank_one_update` | `umath_matrix_rank_one_update` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `scale(signature pending)` | `matrix_scale` | `umath_matrix_scale` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `sub(signature pending)` | `matrix_sub` | `umath_matrix_sub` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `symmetrize(signature pending)` | `matrix_symmetrize` | `umath_matrix_symmetrize` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `trace(signature pending)` | `matrix_trace` | `umath_matrix_trace` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `transpose(signature pending)` | `matrix_transpose` | `umath_matrix_transpose` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `vector(signature pending)` | `matrix_vector` | `umath_matrix_vector` | `umath/matrix/core` | implemented | signature_only | [include/umath/matrix/core.h](../../UETD/include/umath/matrix/core.h) |
| `ldlt_factor(signature pending)` | `matrix_ldlt_factor` | `umath_matrix_ldlt_factor` | `umath/matrix/ldlt` | scaffold | signature_only | [include/umath/matrix/ldlt.h](../../UETD/include/umath/matrix/ldlt.h) |
| `ldlt_solve(signature pending)` | `matrix_ldlt_solve` | `umath_matrix_ldlt_solve` | `umath/matrix/ldlt` | scaffold | signature_only | [include/umath/matrix/ldlt.h](../../UETD/include/umath/matrix/ldlt.h) |
| `lu_determinant(signature pending)` | `matrix_lu_determinant` | `umath_matrix_lu_determinant` | `umath/matrix/lu` | scaffold | signature_only | [include/umath/matrix/lu.h](../../UETD/include/umath/matrix/lu.h) |
| `lu_factor(signature pending)` | `matrix_lu_factor` | `umath_matrix_lu_factor` | `umath/matrix/lu` | scaffold | signature_only | [include/umath/matrix/lu.h](../../UETD/include/umath/matrix/lu.h) |
| `lu_solve(signature pending)` | `matrix_lu_solve` | `umath_matrix_lu_solve` | `umath/matrix/lu` | scaffold | signature_only | [include/umath/matrix/lu.h](../../UETD/include/umath/matrix/lu.h) |
| `qr_factor(signature pending)` | `matrix_qr_factor` | `umath_matrix_qr_factor` | `umath/matrix/qr` | scaffold | signature_only | [include/umath/matrix/qr.h](../../UETD/include/umath/matrix/qr.h) |
| `qr_least_squares(signature pending)` | `matrix_qr_least_squares` | `umath_matrix_qr_least_squares` | `umath/matrix/qr` | scaffold | signature_only | [include/umath/matrix/qr.h](../../UETD/include/umath/matrix/qr.h) |
| `svd(signature pending)` | `matrix_svd` | `umath_matrix_svd` | `umath/matrix/svd` | scaffold | signature_only | [include/umath/matrix/svd.h](../../UETD/include/umath/matrix/svd.h) |
| `svd_solve(signature pending)` | `matrix_svd_solve` | `umath_matrix_svd_solve` | `umath/matrix/svd` | scaffold | signature_only | [include/umath/matrix/svd.h](../../UETD/include/umath/matrix/svd.h) |
| `symmetric_eigen(signature pending)` | `matrix_symmetric_eigen` | `umath_matrix_symmetric_eigen` | `umath/matrix/symmetric_eigen` | scaffold | signature_only | [include/umath/matrix/symmetric_eigen.h](../../UETD/include/umath/matrix/symmetric_eigen.h) |
| `simpson(signature pending)` | `integrate_simpson` | `umath_integrate_simpson` | `umath/numerical/integration` | scaffold | signature_only | [include/umath/numerical/integration.h](../../UETD/include/umath/numerical/integration.h) |
| `trapezoid(signature pending)` | `integrate_trapezoid` | `umath_integrate_trapezoid` | `umath/numerical/integration` | scaffold | signature_only | [include/umath/numerical/integration.h](../../UETD/include/umath/numerical/integration.h) |
| `dopri5_step(signature pending)` | `ode_dopri5_step` | `umath_ode_dopri5_step` | `umath/numerical/integration` | scaffold | signature_only | [include/umath/numerical/integration.h](../../UETD/include/umath/numerical/integration.h) |
| `rk4_step(signature pending)` | `ode_rk4_step` | `umath_ode_rk4_step` | `umath/numerical/integration` | scaffold | signature_only | [include/umath/numerical/integration.h](../../UETD/include/umath/numerical/integration.h) |
| `least_squares_solve(signature pending)` | — | `umath_least_squares_solve` | `umath/optimization/least_squares` | scaffold | signature_only | [include/umath/numerical/least_squares.h](../../UETD/include/umath/numerical/least_squares.h) |
| `residual_norm(signature pending)` | — | `umath_residual_norm` | `umath/optimization/least_squares` | scaffold | signature_only | [include/umath/numerical/least_squares.h](../../UETD/include/umath/numerical/least_squares.h) |
| `weighted_least_squares_solve(signature pending)` | — | `umath_weighted_least_squares_solve` | `umath/optimization/least_squares` | scaffold | signature_only | [include/umath/numerical/least_squares.h](../../UETD/include/umath/numerical/least_squares.h) |
| `add(signature pending)` | `polynomial_add` | `umath_polynomial_add` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `compose(signature pending)` | `polynomial_compose` | `umath_polynomial_compose` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `derivative_coefficients(signature pending)` | `polynomial_derivative_coefficients` | `umath_polynomial_derivative_coefficients` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `derivative_evaluate(signature pending)` | `polynomial_derivative_evaluate` | `umath_polynomial_derivative_evaluate` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `evaluate(signature pending)` | `polynomial_evaluate` | `umath_polynomial_evaluate` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `integral_coefficients(signature pending)` | `polynomial_integral_coefficients` | `umath_polynomial_integral_coefficients` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `integral_evaluate(signature pending)` | `polynomial_integral_evaluate` | `umath_polynomial_integral_evaluate` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `multiply(signature pending)` | `polynomial_multiply` | `umath_polynomial_multiply` | `umath/polynomial/core` | implemented | signature_only | [include/umath/polynomial/core.h](../../UETD/include/umath/polynomial/core.h) |
| `roots_complex(signature pending)` | `poly_roots_complex` | `umath_poly_roots_complex` | `umath/polynomial/roots` | scaffold | signature_only | [include/umath/polynomial/roots.h](../../UETD/include/umath/polynomial/roots.h) |
| `roots_real(signature pending)` | `poly_roots_real` | `umath_poly_roots_real` | `umath/polynomial/roots` | scaffold | signature_only | [include/umath/polynomial/roots.h](../../UETD/include/umath/polynomial/roots.h) |
| `chi_square_cdf(signature pending)` | — | `umath_chi_square_cdf` | `umath/probability/distributions` | scaffold | signature_only | [include/umath/probability/distributions.h](../../UETD/include/umath/probability/distributions.h) |
| `softmax(signature pending)` | `log_softmax` | `umath_log_softmax` | `umath/probability/distributions` | scaffold | signature_only | [include/umath/probability/distributions.h](../../UETD/include/umath/probability/distributions.h) |
| `logsumexp(signature pending)` | — | `umath_logsumexp` | `umath/probability/distributions` | scaffold | signature_only | [include/umath/probability/distributions.h](../../UETD/include/umath/probability/distributions.h) |
| `cdf(signature pending)` | `normal_cdf` | `umath_normal_cdf` | `umath/probability/distributions` | scaffold | signature_only | [include/umath/probability/distributions.h](../../UETD/include/umath/probability/distributions.h) |
| `logpdf(signature pending)` | `normal_logpdf` | `umath_normal_logpdf` | `umath/probability/distributions` | scaffold | signature_only | [include/umath/probability/distributions.h](../../UETD/include/umath/probability/distributions.h) |
| `softmax(signature pending)` | — | `umath_softmax` | `umath/probability/distributions` | scaffold | signature_only | [include/umath/probability/distributions.h](../../UETD/include/umath/probability/distributions.h) |
| `multivariate_normal_logpdf_cholesky(signature pending)` | — | `umath_multivariate_normal_logpdf_cholesky` | `umath/probability/multivariate_normal` | scaffold | signature_only | [include/umath/probability/multivariate_normal.h](../../UETD/include/umath/probability/multivariate_normal.h) |
| `normal(signature pending)` | `prng_normal` | `umath_prng_normal` | `umath/random/prng` | scaffold | signature_only | [include/umath/probability/prng.h](../../UETD/include/umath/probability/prng.h) |
| `seed(signature pending)` | `prng_seed` | `umath_prng_seed` | `umath/random/prng` | scaffold | signature_only | [include/umath/probability/prng.h](../../UETD/include/umath/probability/prng.h) |
| `u32(signature pending)` | `prng_u32` | `umath_prng_u32` | `umath/random/prng` | scaffold | signature_only | [include/umath/probability/prng.h](../../UETD/include/umath/probability/prng.h) |
| `uniform(signature pending)` | `prng_uniform` | `umath_prng_uniform` | `umath/random/prng` | scaffold | signature_only | [include/umath/probability/prng.h](../../UETD/include/umath/probability/prng.h) |
| `execute(signature pending)` | `fft_execute` | `umath_fft_execute` | `umath/signal/fft` | scaffold | signature_only | [include/umath/signal/fft.h](../../UETD/include/umath/signal/fft.h) |
| `init(signature pending)` | `fft_plan_init` | `umath_fft_plan_init` | `umath/signal/fft` | scaffold | signature_only | [include/umath/signal/fft.h](../../UETD/include/umath/signal/fft.h) |
| `real_forward(signature pending)` | `fft_real_forward` | `umath_fft_real_forward` | `umath/signal/fft` | scaffold | signature_only | [include/umath/signal/fft.h](../../UETD/include/umath/signal/fft.h) |
| `blackman(signature pending)` | `window_blackman` | `umath_window_blackman` | `umath/signal/windows` | scaffold | signature_only | [include/umath/signal/windows.h](../../UETD/include/umath/signal/windows.h) |
| `hamming(signature pending)` | `window_hamming` | `umath_window_hamming` | `umath/signal/windows` | scaffold | signature_only | [include/umath/signal/windows.h](../../UETD/include/umath/signal/windows.h) |
| `hann(signature pending)` | `window_hann` | `umath_window_hann` | `umath/signal/windows` | scaffold | signature_only | [include/umath/signal/windows.h](../../UETD/include/umath/signal/windows.h) |
| `kaiser(signature pending)` | `window_kaiser` | `umath_window_kaiser` | `umath/signal/windows` | scaffold | signature_only | [include/umath/signal/windows.h](../../UETD/include/umath/signal/windows.h) |
| `bessel_i0(signature pending)` | — | `umath_bessel_i0` | `umath/special/functions` | scaffold | signature_only | [include/umath/functions.h](../../UETD/include/umath/functions.h) |
| `erf(signature pending)` | — | `umath_erf` | `umath/special/functions` | scaffold | signature_only | [include/umath/functions.h](../../UETD/include/umath/functions.h) |
| `gamma(signature pending)` | `log_gamma` | `umath_log_gamma` | `umath/special/functions` | scaffold | signature_only | [include/umath/functions.h](../../UETD/include/umath/functions.h) |
| `covariance(signature pending)` | `stats_covariance` | `umath_stats_covariance` | `umath/statistics/streaming` | scaffold | signature_only | [include/umath/probability/streaming.h](../../UETD/include/umath/probability/streaming.h) |
| `mean(signature pending)` | `stats_mean` | `umath_stats_mean` | `umath/statistics/streaming` | scaffold | signature_only | [include/umath/probability/streaming.h](../../UETD/include/umath/probability/streaming.h) |
| `quantile(signature pending)` | `stats_quantile` | `umath_stats_quantile` | `umath/statistics/streaming` | scaffold | signature_only | [include/umath/probability/streaming.h](../../UETD/include/umath/probability/streaming.h) |
| `rms(signature pending)` | `stats_rms` | `umath_stats_rms` | `umath/statistics/streaming` | scaffold | signature_only | [include/umath/probability/streaming.h](../../UETD/include/umath/probability/streaming.h) |
| `standard_deviation(signature pending)` | `stats_standard_deviation` | `umath_stats_standard_deviation` | `umath/statistics/streaming` | scaffold | signature_only | [include/umath/probability/streaming.h](../../UETD/include/umath/probability/streaming.h) |
| `variance(signature pending)` | `stats_variance` | `umath_stats_variance` | `umath/statistics/streaming` | scaffold | signature_only | [include/umath/probability/streaming.h](../../UETD/include/umath/probability/streaming.h) |
| `add(signature pending)` | `vector_add` | `umath_vector_add` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `clamp(signature pending)` | `vector_clamp` | `umath_vector_clamp` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `distance(signature pending)` | `vector_distance` | `umath_vector_distance` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `div(signature pending)` | `vector_div` | `umath_vector_div` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `dot(signature pending)` | `vector_dot` | `umath_vector_dot` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `lerp(signature pending)` | `vector_lerp` | `umath_vector_lerp` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `mul(signature pending)` | `vector_mul` | `umath_vector_mul` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `norm(signature pending)` | `vector_norm` | `umath_vector_norm` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `normalize(signature pending)` | `vector_normalize` | `umath_vector_normalize` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `scale(signature pending)` | `vector_scale` | `umath_vector_scale` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `sub(signature pending)` | `vector_sub` | `umath_vector_sub` | `umath/vector/vector` | scaffold | signature_only | [include/umath/vector/vector.h](../../UETD/include/umath/vector/vector.h) |
| `add(signature pending)` | `vec2_add` | `umath_vec2_add` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `cross(left: vec2, right: vec2) -> scalar` | `vec2_cross` | `umath_vec2_cross` | `umath/vector/vector2` | implemented | ready | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `distance(signature pending)` | `vec2_distance` | `umath_vec2_distance` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `div(signature pending)` | `vec2_div` | `umath_vec2_div` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `dot(signature pending)` | `vec2_dot` | `umath_vec2_dot` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `lerp(signature pending)` | `vec2_lerp` | `umath_vec2_lerp` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `mul(signature pending)` | `vec2_mul` | `umath_vec2_mul` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `norm(signature pending)` | `vec2_norm` | `umath_vec2_norm` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `normalize(signature pending)` | `vec2_normalize` | `umath_vec2_normalize` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `project(signature pending)` | `vec2_project` | `umath_vec2_project` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `reflect(signature pending)` | `vec2_reflect` | `umath_vec2_reflect` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `reject(signature pending)` | `vec2_reject` | `umath_vec2_reject` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `scale(signature pending)` | `vec2_scale` | `umath_vec2_scale` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `signed_angle(signature pending)` | `vec2_signed_angle` | `umath_vec2_signed_angle` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `sub(signature pending)` | `vec2_sub` | `umath_vec2_sub` | `umath/vector/vector2` | implemented | signature_only | [include/umath/vector/vector2.h](../../UETD/include/umath/vector/vector2.h) |
| `add(signature pending)` | `vec3_add` | `umath_vec3_add` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `angle(signature pending)` | `vec3_angle` | `umath_vec3_angle` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `cross(left: vec3, right: vec3) -> vec3` | `vec3_cross` | `umath_vec3_cross` | `umath/vector/vector3` | implemented | ready | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `distance(signature pending)` | `vec3_distance` | `umath_vec3_distance` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `div(signature pending)` | `vec3_div` | `umath_vec3_div` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `dot(signature pending)` | `vec3_dot` | `umath_vec3_dot` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `lerp(signature pending)` | `vec3_lerp` | `umath_vec3_lerp` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `mul(signature pending)` | `vec3_mul` | `umath_vec3_mul` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `norm(signature pending)` | `vec3_norm` | `umath_vec3_norm` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `normalize(signature pending)` | `vec3_normalize` | `umath_vec3_normalize` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `project(signature pending)` | `vec3_project` | `umath_vec3_project` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `reflect(signature pending)` | `vec3_reflect` | `umath_vec3_reflect` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `reject(signature pending)` | `vec3_reject` | `umath_vec3_reject` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `scale(signature pending)` | `vec3_scale` | `umath_vec3_scale` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `sub(signature pending)` | `vec3_sub` | `umath_vec3_sub` | `umath/vector/vector3` | implemented | signature_only | [include/umath/vector/vector3.h](../../UETD/include/umath/vector/vector3.h) |
| `add(signature pending)` | `vec4_add` | `umath_vec4_add` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `angle(signature pending)` | `vec4_angle` | `umath_vec4_angle` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `distance(signature pending)` | `vec4_distance` | `umath_vec4_distance` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `div(signature pending)` | `vec4_div` | `umath_vec4_div` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `dot(signature pending)` | `vec4_dot` | `umath_vec4_dot` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `lerp(signature pending)` | `vec4_lerp` | `umath_vec4_lerp` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `mul(signature pending)` | `vec4_mul` | `umath_vec4_mul` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `norm(signature pending)` | `vec4_norm` | `umath_vec4_norm` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `normalize(signature pending)` | `vec4_normalize` | `umath_vec4_normalize` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `project(signature pending)` | `vec4_project` | `umath_vec4_project` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `reflect(signature pending)` | `vec4_reflect` | `umath_vec4_reflect` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `reject(signature pending)` | `vec4_reject` | `umath_vec4_reject` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `scale(signature pending)` | `vec4_scale` | `umath_vec4_scale` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |
| `sub(signature pending)` | `vec4_sub` | `umath_vec4_sub` | `umath/vector/vector4` | implemented | signature_only | [include/umath/vector/vector4.h](../../UETD/include/umath/vector/vector4.h) |

## Source catalogues

- [Module index](../../UETD/registry/modules/registry.json)
- [Type index](../../UETD/registry/types/registry.json)
- [UMATH function index](../../UETD/registry/umath/registry.json)
- [UETD public headers](../../UETD/include/)
