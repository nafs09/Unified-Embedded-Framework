file(GLOB_RECURSE _ucon_sources CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/ucon/*.c")
set(_ucon_scaffold_sources)
set(_ucon_declarations "")
set(_ucon_cases "")
set(_ucon_count 0)
foreach(_source IN LISTS _ucon_sources)
  file(STRINGS "${_source}" _functions
    REGEX "^ucon_status_t ucon_[A-Za-z0-9_]+\\(const ucon_algorithm_scaffold_call_t\\* call\\)")
  if(_functions)
    list(APPEND _ucon_scaffold_sources "${_source}")
    foreach(_function_line IN LISTS _functions)
      string(REGEX MATCH "^ucon_status_t (ucon_[A-Za-z0-9_]+)\\(" _match "${_function_line}")
      set(_function "${CMAKE_MATCH_1}")
      string(APPEND _ucon_declarations
        "ucon_status_t ${_function}(const ucon_algorithm_scaffold_call_t* call);\n")
      string(APPEND _ucon_cases "    { \"${_function}\", ${_function} },\n")
      math(EXPR _ucon_count "${_ucon_count} + 1")
    endforeach()
  endif()
endforeach()
if(NOT _ucon_count)
  message(FATAL_ERROR "No UCON scaffold operations were found for the contract suite")
endif()

set(_ucon_test_source "${CMAKE_CURRENT_BINARY_DIR}/ucon_scaffold_contracts.c")
file(WRITE "${_ucon_test_source}" [=[
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ucon/detail/algorithm_call.h>
#include <ucon/status.h>

typedef ucon_status_t (*ucon_scaffold_operation_t)(const ucon_algorithm_scaffold_call_t* call);
typedef struct {
    const char* name;
    ucon_scaffold_operation_t operation;
} ucon_scaffold_case_t;

]=])
file(APPEND "${_ucon_test_source}" "${_ucon_declarations}\nstatic const ucon_scaffold_case_t cases[] = {\n${_ucon_cases}};\n")
file(APPEND "${_ucon_test_source}" [=[
int main(void) {
    static const unsigned char configuration_before[] = {0x13u, 0x57u, 0x9Bu, 0xDFu};
    static const unsigned char state_before[] = {0xA5u, 0x5Au, 0xC3u, 0x3Cu};
    static const ucon_scalar_t inputs_before[] = {(ucon_scalar_t)1.25, (ucon_scalar_t)-2.5};
    static const ucon_scalar_t outputs_before[] = {(ucon_scalar_t)71.0, (ucon_scalar_t)-19.0};
    unsigned char configuration[sizeof(configuration_before)];
    unsigned char state[sizeof(state_before)];
    ucon_scalar_t inputs[sizeof(inputs_before) / sizeof(inputs_before[0])];
    ucon_scalar_t outputs[sizeof(outputs_before) / sizeof(outputs_before[0])];
    size_t failures = 0u;
    size_t index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        ucon_algorithm_scaffold_call_t call;
        ucon_algorithm_scaffold_call_t before;
        memcpy(configuration, configuration_before, sizeof(configuration));
        memcpy(state, state_before, sizeof(state));
        memcpy(inputs, inputs_before, sizeof(inputs));
        memcpy(outputs, outputs_before, sizeof(outputs));
        call.configuration = configuration;
        call.state = state;
        call.inputs = inputs;
        call.input_count = sizeof(inputs) / sizeof(inputs[0]);
        call.outputs = outputs;
        call.output_count = sizeof(outputs) / sizeof(outputs[0]);
        before = call;

        if (cases[index].operation(NULL) != UCON_INVALID_ARGUMENT) {
            printf("FAIL %s: NULL call must return UCON_INVALID_ARGUMENT\n", cases[index].name);
            ++failures;
        }
        if (cases[index].operation(&call) != UCON_NOT_IMPLEMENTED) {
            printf("FAIL %s: valid call must return UCON_NOT_IMPLEMENTED\n", cases[index].name);
            ++failures;
        }
        if (call.configuration != before.configuration || call.state != before.state ||
            call.inputs != before.inputs || call.input_count != before.input_count ||
            call.outputs != before.outputs || call.output_count != before.output_count ||
            memcmp(configuration, configuration_before, sizeof(configuration)) != 0 ||
            memcmp(state, state_before, sizeof(state)) != 0 ||
            memcmp(inputs, inputs_before, sizeof(inputs)) != 0 ||
            memcmp(outputs, outputs_before, sizeof(outputs)) != 0) {
            printf("FAIL %s: scaffold changed caller-owned storage\n", cases[index].name);
            ++failures;
        }
    }

    if (failures != 0u) {
        printf("%zu UCON scaffold contract check(s) failed across %zu operations\n",
               failures, sizeof(cases) / sizeof(cases[0]));
        return 1;
    }
    printf("Validated %zu UCON scaffold operations\n", sizeof(cases) / sizeof(cases[0]));
    return 0;
}
]=])

add_executable(uetd_ucon_scaffold_contracts ${_ucon_scaffold_sources} "${_ucon_test_source}")
target_include_directories(uetd_ucon_scaffold_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_ucon_scaffold_contracts PRIVATE c_std_11)
target_compile_definitions(uetd_ucon_scaffold_contracts PRIVATE ${UETD_UMATH_DEFINITIONS})
add_test(NAME uetd.ucon_scaffold_contracts COMMAND uetd_ucon_scaffold_contracts)

file(GLOB_RECURSE _uproto_future_sources CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/uproto/future/*.c")
set(_uproto_scaffold_sources)
set(_uproto_declarations "")
set(_uproto_cases "")
set(_uproto_count 0)
foreach(_source IN LISTS _uproto_future_sources)
  file(STRINGS "${_source}" _functions
    REGEX "^uetd_status_t uproto_[A-Za-z0-9_]+\\(uproto_future_call_t\\* call\\)")
  if(_functions)
    list(APPEND _uproto_scaffold_sources "${_source}")
    foreach(_function_line IN LISTS _functions)
      string(REGEX MATCH "^uetd_status_t (uproto_[A-Za-z0-9_]+)\\(" _match "${_function_line}")
      set(_function "${CMAKE_MATCH_1}")
      string(APPEND _uproto_declarations
        "uetd_status_t ${_function}(uproto_future_call_t* call);\n")
      string(APPEND _uproto_cases "    { \"${_function}\", ${_function} },\n")
      math(EXPR _uproto_count "${_uproto_count} + 1")
    endforeach()
  endif()
endforeach()
if(NOT _uproto_count)
  message(FATAL_ERROR "No future UPROTO scaffold operations were found for the contract suite")
endif()

set(_uproto_test_source "${CMAKE_CURRENT_BINARY_DIR}/uproto_scaffold_contracts.c")
file(WRITE "${_uproto_test_source}" [=[
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <uproto/future/scaffold_call.h>

typedef uetd_status_t (*uproto_scaffold_operation_t)(uproto_future_call_t* call);
typedef struct {
    const char* name;
    uproto_scaffold_operation_t operation;
} uproto_scaffold_case_t;

]=])
file(APPEND "${_uproto_test_source}" "${_uproto_declarations}\nstatic const uproto_scaffold_case_t cases[] = {\n${_uproto_cases}};\n")
file(APPEND "${_uproto_test_source}" [=[
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
]=])

add_executable(uetd_uproto_future_scaffold_contracts
  ${_uproto_scaffold_sources} "${_uproto_test_source}")
target_include_directories(uetd_uproto_future_scaffold_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_uproto_future_scaffold_contracts PRIVATE c_std_11)
add_test(NAME uetd.uproto_future_scaffold_contracts
  COMMAND uetd_uproto_future_scaffold_contracts)

include("${CMAKE_CURRENT_LIST_DIR}/generate_scaffold_contracts.cmake")

file(GLOB_RECURSE _upal_sources CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/upal/*.c")
set(_upal_fdcan_fd_source)
set(_upal_contract_sources)
foreach(_source IN LISTS _upal_sources)
  if(_source MATCHES "/fdcan_fd/")
    set(_upal_fdcan_fd_source "${_source}")
  else()
    list(APPEND _upal_contract_sources "${_source}")
  endif()
endforeach()
uetd_add_scaffold_contract_test(uetd_upal_scaffold_contracts UETD_NOT_SUPPORTED
  SOURCES ${_upal_contract_sources})
# Compile peripheral outlines against their hardware-facing declarations; the host
# capability profile otherwise hides those declarations or substitutes inline APIs.
target_compile_definitions(uetd_upal_scaffold_contracts PRIVATE
  UHAL_HAS_DMA=1 UHAL_HAS_UART=1 UHAL_HAS_SPI=1 UHAL_HAS_I2C=1
  UHAL_HAS_CAN=1 UHAL_HAS_FDCAN=1 UHAL_HAS_USB_FS=1 UHAL_HAS_USB_HS=1
  UHAL_HAS_QSPI=1 UHAL_HAS_SDMMC=1 UHAL_HAS_ETH=1 UHAL_HAS_ADC=1
  UHAL_HAS_DAC=1 UHAL_HAS_COMP=1 UHAL_HAS_OPAMP=1 UHAL_HAS_HRTIM=1
  UHAL_HAS_LPTIM=1 UHAL_HAS_IWDG=1 UHAL_HAS_WWDG=1 UHAL_HAS_RTC=1
  UHAL_HAS_FLASH_WRITE=1 UHAL_HAS_CRC=1 UHAL_HAS_CORDIC=1 UHAL_HAS_FMAC=1)
add_executable(uetd_upal_fdcan_fd_scaffold_contracts
  upal_fdcan_fd_scaffolds.c "${_upal_fdcan_fd_source}")
target_include_directories(uetd_upal_fdcan_fd_scaffold_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_upal_fdcan_fd_scaffold_contracts PRIVATE c_std_11)
add_test(NAME uetd.upal_fdcan_fd_scaffold_contracts COMMAND uetd_upal_fdcan_fd_scaffold_contracts)

file(GLOB_RECURSE _umid_candidates CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/umid/drivers/*.c")
set(_umid_scaffold_sources)
foreach(_source IN LISTS _umid_candidates)
  file(STRINGS "${_source}" _scaffold_marker REGEX "Source scaffold")
  if(_scaffold_marker)
    list(APPEND _umid_scaffold_sources "${_source}")
  endif()
endforeach()
uetd_add_scaffold_contract_test(uetd_umid_scaffold_contracts UETD_NOT_SUPPORTED
  SOURCES ${_umid_scaffold_sources})

file(GLOB _uproto_legacy_candidates CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/uproto/*.c")
set(_uproto_legacy_sources)
foreach(_source IN LISTS _uproto_legacy_candidates)
  if(NOT _source MATCHES "/ppm\\.c$")
    list(APPEND _uproto_legacy_sources "${_source}")
  endif()
endforeach()
uetd_add_scaffold_contract_test(uetd_uproto_scaffold_contracts UETD_NOT_SUPPORTED
  SOURCES ${_uproto_legacy_sources})

file(GLOB_RECURSE _uos_safertos_sources CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/uos/future/safertos/*.c")
uetd_add_scaffold_contract_test(uetd_uos_safertos_scaffold_contracts 0
  SOURCES ${_uos_safertos_sources})

file(GLOB_RECURSE _umath_candidates CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/umath/*.c")
set(_umath_scaffold_sources)
foreach(_source IN LISTS _umath_candidates)
  file(STRINGS "${_source}" _stub_returns REGEX "return UMATH_NOT_IMPLEMENTED;")
  if(_stub_returns)
    list(APPEND _umath_scaffold_sources "${_source}")
  endif()
endforeach()
uetd_add_scaffold_contract_test(uetd_umath_scaffold_contracts UMATH_NOT_IMPLEMENTED
  SOURCES ${_umath_scaffold_sources})

file(GLOB_RECURSE _target_profile_sources CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/uhal/targets/*/profile.c")
add_executable(uetd_target_profile_scaffold_contracts
  target_profile_scaffolds.c
  "${PROJECT_SOURCE_DIR}/src/uhal/target_profiles.c"
  ${_target_profile_sources})
target_include_directories(uetd_target_profile_scaffold_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_target_profile_scaffold_contracts PRIVATE c_std_11)
add_test(NAME uetd.target_profile_scaffold_contracts COMMAND uetd_target_profile_scaffold_contracts)

add_custom_target(uetd_scaffold_contracts)
add_dependencies(uetd_scaffold_contracts
  uetd_ucon_scaffold_contracts
  uetd_upal_scaffold_contracts
  uetd_upal_fdcan_fd_scaffold_contracts
  uetd_umid_scaffold_contracts
  uetd_uproto_scaffold_contracts
  uetd_uproto_future_scaffold_contracts
  uetd_uos_safertos_scaffold_contracts
  uetd_umath_scaffold_contracts
  uetd_target_profile_scaffold_contracts)
