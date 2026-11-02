# First-pass unsupported contracts, linked through the ordinary UETD archive.
uetd_add_scaffold_contract_test(uetd_first_pass_expansion_contracts UETD_NOT_SUPPORTED
  LINK_CORE
  SOURCES
    "${PROJECT_SOURCE_DIR}/src/upal/capability_contracts.c"
    "${PROJECT_SOURCE_DIR}/src/ucore/codec.c"
    "${PROJECT_SOURCE_DIR}/src/upal/transaction.c"
    "${PROJECT_SOURCE_DIR}/src/upal/display/dsi_host.c"
    "${PROJECT_SOURCE_DIR}/src/upal/display/display_panel.c"
    "${PROJECT_SOURCE_DIR}/src/upal/display/touch.c"
    "${PROJECT_SOURCE_DIR}/src/upal/power.c"
    "${PROJECT_SOURCE_DIR}/src/upal/storage/block_device.c"
    "${PROJECT_SOURCE_DIR}/src/umid/messaging/buffer_pool.c"
    "${PROJECT_SOURCE_DIR}/src/umid/messaging/mailbox.c"
    "${PROJECT_SOURCE_DIR}/src/umid/messaging/event_bus.c"
    "${PROJECT_SOURCE_DIR}/src/umid/sensor.c"
    "${PROJECT_SOURCE_DIR}/src/umid/data/settings.c"
    "${PROJECT_SOURCE_DIR}/src/umid/ui/input.c"
    "${PROJECT_SOURCE_DIR}/src/umid/ui/graphics.c"
    "${PROJECT_SOURCE_DIR}/src/uactuator/actuator.c"
    "${PROJECT_SOURCE_DIR}/src/usec/provider.c"
    "${PROJECT_SOURCE_DIR}/src/uproto/network/connection_manager.c"
    "${PROJECT_SOURCE_DIR}/src/uproto/usb_device.c"
    "${PROJECT_SOURCE_DIR}/src/uproto/network/time_sync.c"
    "${PROJECT_SOURCE_DIR}/src/uproto/framing.c")
