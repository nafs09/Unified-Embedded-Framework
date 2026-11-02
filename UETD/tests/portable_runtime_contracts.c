/* Portable behavior and archive-link checks, independent of physical hardware. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <uapp/runtime/watchdog_gate.h>
#include <ucon/control/classical/lead_lag.h>
#include <ucon/control/classical/pid.h>
#include <ucore/checksum.h>
#include <umath/matrix/core.h>
#include <umath/matrix/cholesky.h>
#include <umid/messaging/ringbuf.h>
#include <uproto/rc/ppm.h>

#define CHECK(condition) do { if (!(condition)) { \
    printf("FAIL line %d: %s\n", __LINE__, #condition); return 1; } } while (0)

static uetd_status_t refresh(void* context) {
    ++*(unsigned*)context;
    return UETD_OK;
}

int main(void) {
    const uetd_u8_t check[] = "123456789";
    CHECK(uetd_crc8(check, 9, 0) == 0xF4u);
    CHECK(uetd_crc16_ccitt(check, 9, 0xFFFFu) == 0x29B1u);
    CHECK((uetd_crc32(check, 9, 0xFFFFFFFFu) ^ 0xFFFFFFFFu) == 0xCBF43926u);

    {
        umid_ringbuf_t ring;
        uetd_u8_t storage[4], output[4];
        const uetd_u8_t first[] = {1, 2, 3, 4}, second[] = {5, 6, 7};
        const uetd_u8_t expected[] = {4, 5, 6, 7};
        umid_ringbuf_init(&ring, storage, 4);
        CHECK(umid_ringbuf_write(&ring, first, 4) == 4);
        CHECK(umid_ringbuf_write(&ring, first, 1) == 0);
        CHECK(umid_ringbuf_read(&ring, output, 3) == 3);
        CHECK(umid_ringbuf_write(&ring, second, 3) == 3);
        CHECK(umid_ringbuf_read(&ring, output, 4) == 4);
        CHECK(memcmp(output, expected, 4) == 0);
    }
    {
        uproto_ppm_t decoder;
        uproto_ppm_frame_t frame;
        const uproto_ppm_config_t config = {900, 2100, 3000, 2, 2};
        CHECK(uproto_ppm_init(&decoder, &config) == UETD_OK);
        CHECK(uproto_ppm_capture_interval(&decoder, 3000, 3000) == UETD_NOT_READY);
        CHECK(uproto_ppm_capture_interval(&decoder, 1000, 4000) == UETD_OK);
        CHECK(uproto_ppm_capture_interval(&decoder, 2000, 6000) == UETD_OK);
        CHECK(uproto_ppm_capture_interval(&decoder, 3000, 9000) == UETD_OK);
        CHECK(uproto_ppm_get_frame(&decoder, &frame) == UETD_OK);
        CHECK(frame.channel_count == 2 && frame.channels_us[0] == 1000 && frame.channels_us[1] == 2000);
        CHECK(frame.timestamp_us == 9000 && frame.sequence == 1);
        CHECK(uproto_ppm_capture_interval(&decoder, 100, 9100) == UETD_INVALID_ARG);
        CHECK(!uproto_ppm_available(&decoder));
    }
    {
        unsigned feeds = 0;
        uapp_watchdog_gate_t gate;
        const uetd_u8_t id = umid_health_register("audit-test");
        const uapp_watchdog_lease_t lease = {id, UMID_HEALTH_WARN, 1000};
        const uapp_watchdog_gate_cfg_t config = {&lease, 1, UINT64_MAX, refresh, NULL, &feeds};
        CHECK(id != UMID_HEALTH_INVALID_ID);
        umid_health_report(id, UMID_HEALTH_FAIL, 42);
        CHECK(uapp_watchdog_gate_init(&gate, &config) == UETD_OK);
        CHECK(uapp_watchdog_gate_tick(&gate) == UETD_OK && feeds == 1);
        CHECK(gate.last_reason == UAPP_WDG_GATE_BOOT_GRACE);
        uapp_watchdog_gate_latch(&gate, 42);
        CHECK(uapp_watchdog_gate_tick(&gate) == UETD_NOT_READY && feeds == 1);
    }
    {
        ucon_pid_state_t state, before;
        ucon_pid_config_t config = {0};
        ucon_scalar_t output = 0;
        config.kp = 2; config.sample_time = (ucon_scalar_t)0.1;
        config.proportional_weight_b = 1;
        config.integral_min = -10; config.integral_max = 10;
        CHECK(ucon_pid_init(&state, &config) == UCON_OK);
        CHECK(ucon_pid_step(&state, &config, 3, 1, 0, NULL, &output) == UCON_OK && output == 4);
        before = state; output = 91;
        CHECK(ucon_pid_step(&state, &config, (ucon_scalar_t)NAN, 1, 0, NULL, &output) == UCON_INVALID_ARGUMENT);
        CHECK(output == 91 && memcmp(&before, &state, sizeof(state)) == 0);
    }
    {
        const ucon_lead_lag_config_t config = {2, 1, 1, (ucon_scalar_t)0.01};
        ucon_lead_lag_state_t state;
        ucon_scalar_t output;
        CHECK(ucon_lead_lag_init(&state, &config) == UCON_OK);
        CHECK(ucon_lead_lag_step(&state, &config, 3, &output) == UCON_OK);
        CHECK(fabs((double)output - 6) < 1e-5);
    }
    {
        const umath_scalar_t matrix[] = {4, 1, 1, 3};
        umath_scalar_t lower[] = {4, 1, 1, 3}, inverse[4], product[4], exponential[4];
        umath_accumulator_t workspace[12];
        size_t count;
        CHECK(umath_matrix_inverse_workspace_count(2, &count) == UMATH_OK && count <= 12);
        CHECK(umath_matrix_inverse(matrix, 2, inverse, workspace, 12) == UMATH_OK);
        CHECK(umath_matrix_multiply(matrix, inverse, 2, 2, 2, product) == UMATH_OK);
        CHECK(fabs((double)product[0] - 1) < 1e-5 && fabs((double)product[3] - 1) < 1e-5);
        CHECK(fabs((double)product[1]) < 1e-5 && fabs((double)product[2]) < 1e-5);
        CHECK(umath_cholesky_factor(lower, 2) == UMATH_OK);
        CHECK(umath_matrix_exponential_workspace_count(2, &count) == UMATH_OK && count <= 12);
        CHECK(umath_matrix_exponential((umath_scalar_t[]){0, 0, 0, 0}, 2, exponential, workspace, 12) == UMATH_OK);
        CHECK(exponential[0] == 1 && exponential[3] == 1 && exponential[1] == 0 && exponential[2] == 0);
    }
    puts("Portable runtime and archive-link contracts passed");
    return 0;
}
