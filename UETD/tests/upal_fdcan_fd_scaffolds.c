#include <stdio.h>
#include <string.h>

#include <upal/fdcan_fd.h>

int main(void) {
    size_t failures = 0u;
    upal_fdcan_fd_config_t config = {0};
    upal_fdcan_fd_t driver;
    upal_fdcan_fd_t driver_before;
    upal_fdcan_fd_frame_t frame;
    unsigned char driver_bytes[sizeof(driver)];
    unsigned char frame_bytes[sizeof(frame)];

    config.instance = &driver;
    config.nominal_bitrate = 500000u;
    config.data_bitrate = 2000000u;
    config.tx_element_count = 1u;
    config.rx_element_count = 1u;
    config.nominal_sample_point_percent = 80u;
    config.data_sample_point_percent = 75u;

    memset(&driver, 0xA5, sizeof(driver));
    memcpy(driver_bytes, &driver, sizeof(driver));
    if (upal_fdcan_fd_init(&driver, &config) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_fdcan_fd_init: valid configuration must fail closed\n");
        ++failures;
    }
    if (memcmp(&driver, driver_bytes, sizeof(driver)) != 0) {
        printf("FAIL upal_fdcan_fd_init: changed driver storage\n");
        ++failures;
    }
    if (upal_fdcan_fd_init(NULL, &config) != UETD_INVALID_ARG ||
        upal_fdcan_fd_init(&driver, NULL) != UETD_INVALID_ARG) {
        printf("FAIL upal_fdcan_fd_init: invalid pointers must be rejected\n");
        ++failures;
    }

    memset(&driver, 0, sizeof(driver));
    driver.config = &config;
    driver.initialized = true;
    memcpy(&driver_before, &driver, sizeof(driver));
    if (upal_fdcan_fd_deinit(&driver) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_fdcan_fd_deinit: initialized driver must fail closed\n");
        ++failures;
    }
    if (memcmp(&driver, &driver_before, sizeof(driver)) != 0) {
        printf("FAIL upal_fdcan_fd_deinit: changed driver storage\n");
        ++failures;
    }
    if (upal_fdcan_fd_deinit(NULL) != UETD_INVALID_ARG) {
        printf("FAIL upal_fdcan_fd_deinit: null driver must be rejected\n");
        ++failures;
    }

    memset(&frame, 0, sizeof(frame));
    frame.identifier = 0x123u;
    memset(frame_bytes, 0, sizeof(frame_bytes));
    memcpy(frame_bytes, &frame, sizeof(frame));
    if (upal_fdcan_fd_transmit(&driver, &frame) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_fdcan_fd_transmit: valid frame must fail closed\n");
        ++failures;
    }
    if (memcmp(&frame, frame_bytes, sizeof(frame)) != 0) {
        printf("FAIL upal_fdcan_fd_transmit: changed caller frame\n");
        ++failures;
    }
    frame.data_length = 9u;
    if (upal_fdcan_fd_transmit(&driver, &frame) != UETD_INVALID_ARG) {
        printf("FAIL upal_fdcan_fd_transmit: invalid DLC must be rejected\n");
        ++failures;
    }

    memset(&frame, 0x5A, sizeof(frame));
    memcpy(frame_bytes, &frame, sizeof(frame));
    if (upal_fdcan_fd_receive(&driver, &frame) != UETD_NOT_SUPPORTED) {
        printf("FAIL upal_fdcan_fd_receive: valid output must fail closed\n");
        ++failures;
    }
    if (memcmp(&frame, frame_bytes, sizeof(frame)) != 0) {
        printf("FAIL upal_fdcan_fd_receive: changed caller frame\n");
        ++failures;
    }
    if (upal_fdcan_fd_receive(&driver, NULL) != UETD_INVALID_ARG) {
        printf("FAIL upal_fdcan_fd_receive: null output must be rejected\n");
        ++failures;
    }

    if (failures != 0u) {
        printf("%zu UPAL FDCAN-FD scaffold check(s) failed\n", failures);
        return 1;
    }
    puts("Validated UPAL FDCAN-FD fail-closed and argument-validation contracts");
    return 0;
}
