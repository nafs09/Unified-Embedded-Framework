#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uproto/future/scaffold_call.h>

typedef uetd_status_t (*uproto_scaffold_operation_t)(uproto_future_call_t* call);
typedef struct {
    const char* name;
    uproto_scaffold_operation_t operation;
} uproto_scaffold_case_t;

uetd_status_t uproto_elrs_validate_profile(uproto_future_call_t* call);
uetd_status_t uproto_elrs_init(uproto_future_call_t* call);
uetd_status_t uproto_elrs_process(uproto_future_call_t* call);
uetd_status_t uproto_elrs_get_link_state(uproto_future_call_t* call);
uetd_status_t uproto_elrs_reset(uproto_future_call_t* call);
uetd_status_t uproto_iec60870_validate_profile(uproto_future_call_t* call);
uetd_status_t uproto_iec60870_init(uproto_future_call_t* call);
uetd_status_t uproto_iec60870_process(uproto_future_call_t* call);
uetd_status_t uproto_iec60870_send_asdu(uproto_future_call_t* call);
uetd_status_t uproto_iec60870_next_event(uproto_future_call_t* call);
uetd_status_t uproto_iec60870_reset(uproto_future_call_t* call);
uetd_status_t uproto_j1939_init(uproto_future_call_t* call);
uetd_status_t uproto_j1939_process_frame(uproto_future_call_t* call);
uetd_status_t uproto_j1939_claim_address(uproto_future_call_t* call);
uetd_status_t uproto_j1939_send_pgn(uproto_future_call_t* call);
uetd_status_t uproto_j1939_receive_pgn(uproto_future_call_t* call);
uetd_status_t uproto_j1939_reset(uproto_future_call_t* call);
uetd_status_t uproto_lin_validate_schedule(uproto_future_call_t* call);
uetd_status_t uproto_lin_init(uproto_future_call_t* call);
uetd_status_t uproto_lin_schedule_header(uproto_future_call_t* call);
uetd_status_t uproto_lin_process_response(uproto_future_call_t* call);
uetd_status_t uproto_lin_reset(uproto_future_call_t* call);
uetd_status_t uproto_msp_validate_config(uproto_future_call_t* call);
uetd_status_t uproto_msp_init(uproto_future_call_t* call);
uetd_status_t uproto_msp_feed_bytes(uproto_future_call_t* call);
uetd_status_t uproto_msp_encode_request(uproto_future_call_t* call);
uetd_status_t uproto_msp_take_response(uproto_future_call_t* call);
uetd_status_t uproto_msp_reset(uproto_future_call_t* call);
uetd_status_t uproto_uavcan_v1_init(uproto_future_call_t* call);
uetd_status_t uproto_uavcan_v1_spin(uproto_future_call_t* call);
uetd_status_t uproto_uavcan_v1_publish(uproto_future_call_t* call);
uetd_status_t uproto_uavcan_v1_subscribe(uproto_future_call_t* call);
uetd_status_t uproto_uavcan_v1_reset(uproto_future_call_t* call);

static const uproto_scaffold_case_t cases[] = {
    { "uproto_elrs_validate_profile", uproto_elrs_validate_profile },
    { "uproto_elrs_init", uproto_elrs_init },
    { "uproto_elrs_process", uproto_elrs_process },
    { "uproto_elrs_get_link_state", uproto_elrs_get_link_state },
    { "uproto_elrs_reset", uproto_elrs_reset },
    { "uproto_iec60870_validate_profile", uproto_iec60870_validate_profile },
    { "uproto_iec60870_init", uproto_iec60870_init },
    { "uproto_iec60870_process", uproto_iec60870_process },
    { "uproto_iec60870_send_asdu", uproto_iec60870_send_asdu },
    { "uproto_iec60870_next_event", uproto_iec60870_next_event },
    { "uproto_iec60870_reset", uproto_iec60870_reset },
    { "uproto_j1939_init", uproto_j1939_init },
    { "uproto_j1939_process_frame", uproto_j1939_process_frame },
    { "uproto_j1939_claim_address", uproto_j1939_claim_address },
    { "uproto_j1939_send_pgn", uproto_j1939_send_pgn },
    { "uproto_j1939_receive_pgn", uproto_j1939_receive_pgn },
    { "uproto_j1939_reset", uproto_j1939_reset },
    { "uproto_lin_validate_schedule", uproto_lin_validate_schedule },
    { "uproto_lin_init", uproto_lin_init },
    { "uproto_lin_schedule_header", uproto_lin_schedule_header },
    { "uproto_lin_process_response", uproto_lin_process_response },
    { "uproto_lin_reset", uproto_lin_reset },
    { "uproto_msp_validate_config", uproto_msp_validate_config },
    { "uproto_msp_init", uproto_msp_init },
    { "uproto_msp_feed_bytes", uproto_msp_feed_bytes },
    { "uproto_msp_encode_request", uproto_msp_encode_request },
    { "uproto_msp_take_response", uproto_msp_take_response },
    { "uproto_msp_reset", uproto_msp_reset },
    { "uproto_uavcan_v1_init", uproto_uavcan_v1_init },
    { "uproto_uavcan_v1_spin", uproto_uavcan_v1_spin },
    { "uproto_uavcan_v1_publish", uproto_uavcan_v1_publish },
    { "uproto_uavcan_v1_subscribe", uproto_uavcan_v1_subscribe },
    { "uproto_uavcan_v1_reset", uproto_uavcan_v1_reset },
};
int main(void) {
    static const unsigned char instance_before[] = {0x10u, 0x20u, 0x30u, 0x40u};
    static const unsigned char configuration_before[] = {0x51u, 0x61u, 0x71u, 0x81u};
    static const unsigned char input_before[] = {0x02u, 0x7Fu, 0x80u, 0xFEu};
    static const unsigned char output_before[] = {0xD1u, 0x4Bu, 0xA6u, 0x39u};
    static const unsigned char transport_before[] = {0x91u, 0xA1u, 0xB1u, 0xC1u};
    unsigned char instance[sizeof(instance_before)];
    unsigned char configuration[sizeof(configuration_before)];
    unsigned char input[sizeof(input_before)];
    unsigned char output[sizeof(output_before)];
    unsigned char transport[sizeof(transport_before)];
    size_t failures = 0u;
    size_t index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        uproto_future_call_t call;
        uproto_future_call_t before;
        memcpy(instance, instance_before, sizeof(instance));
        memcpy(configuration, configuration_before, sizeof(configuration));
        memcpy(input, input_before, sizeof(input));
        memcpy(output, output_before, sizeof(output));
        memcpy(transport, transport_before, sizeof(transport));
        call.instance = instance;
        call.config = configuration;
        call.input = input;
        call.input_size = (uetd_u32_t)sizeof(input);
        call.output = output;
        call.output_capacity = (uetd_u32_t)sizeof(output);
        call.output_size = 0xA5A5u;
        call.transport_context = transport;
        before = call;

        if (cases[index].operation(NULL) != UETD_INVALID_ARG) {
            printf("FAIL %s: NULL call must return UETD_INVALID_ARG\n", cases[index].name);
            ++failures;
        }
        if (cases[index].operation(&call) != UETD_NOT_SUPPORTED) {
            printf("FAIL %s: valid call must return UETD_NOT_SUPPORTED\n", cases[index].name);
            ++failures;
        }
        if (call.instance != before.instance || call.config != before.config || call.input != before.input ||
            call.input_size != before.input_size || call.output != before.output ||
            call.output_capacity != before.output_capacity || call.output_size != before.output_size ||
            call.transport_context != before.transport_context ||
            memcmp(instance, instance_before, sizeof(instance)) != 0 ||
            memcmp(configuration, configuration_before, sizeof(configuration)) != 0 ||
            memcmp(input, input_before, sizeof(input)) != 0 ||
            memcmp(output, output_before, sizeof(output)) != 0 ||
            memcmp(transport, transport_before, sizeof(transport)) != 0) {
            printf("FAIL %s: scaffold changed caller-owned storage\n", cases[index].name);
            ++failures;
        }
    }

    if (failures != 0u) {
        printf("%zu UPROTO scaffold contract check(s) failed across %zu operations\n",
               failures, sizeof(cases) / sizeof(cases[0]));
        return 1;
    }
    printf("Validated %zu future UPROTO scaffold operations\n", sizeof(cases) / sizeof(cases[0]));
    return 0;
}
