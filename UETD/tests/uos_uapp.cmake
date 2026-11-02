foreach(_suite uapp_runtime uapp_services uapp_update)
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_suite}_contracts.c")
    add_executable(uetd_${_suite}_contracts "${_suite}_contracts.c")
    target_link_libraries(uetd_${_suite}_contracts PRIVATE UETD::Core)
    add_test(NAME uetd.${_suite}_contracts COMMAND uetd_${_suite}_contracts)
  endif()
endforeach()

# Exercise u8 ring/count/state boundaries without changing the library ABI.
add_executable(uetd_uapp_capacity_contracts uapp_runtime_contracts.c
  "${PROJECT_SOURCE_DIR}/src/uapp/runtime/statemachine.c"
  "${PROJECT_SOURCE_DIR}/src/uapp/runtime/lifecycle.c"
  "${PROJECT_SOURCE_DIR}/src/uos/deferred.c"
  "${PROJECT_SOURCE_DIR}/src/ucore/deadline.c" "${PROJECT_SOURCE_DIR}/src/ucore/span.c")
target_include_directories(uetd_uapp_capacity_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_uapp_capacity_contracts PRIVATE c_std_11)
target_compile_definitions(uetd_uapp_capacity_contracts PRIVATE UAPP_SM_MAX_STATES=255
  UAPP_SM_MAX_TRANSITIONS=255 UAPP_SM_EVENT_QUEUE_CAPACITY=255 UAPP_SM_EVENT_PAYLOAD_CAPACITY=255)
add_test(NAME uetd.uapp_capacity_contracts COMMAND uetd_uapp_capacity_contracts)

add_executable(uetd_uos_time_contracts uos_time_contracts.c
  "${PROJECT_SOURCE_DIR}/src/uos/common.c" "${PROJECT_SOURCE_DIR}/src/uos/baremetal.c")
target_include_directories(uetd_uos_time_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_uos_time_contracts PRIVATE c_std_11)
add_test(NAME uetd.uos_time_contracts COMMAND uetd_uos_time_contracts)

add_executable(uetd_uapp_supervision_contracts uapp_supervision_contracts.c
  "${PROJECT_SOURCE_DIR}/src/uapp/runtime/supervisor.c"
  "${PROJECT_SOURCE_DIR}/src/uapp/runtime/watchdog_gate.c" "${PROJECT_SOURCE_DIR}/src/umid/health.c")
target_include_directories(uetd_uapp_supervision_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_uapp_supervision_contracts PRIVATE c_std_11)
add_test(NAME uetd.uapp_supervision_contracts COMMAND uetd_uapp_supervision_contracts)

# These are kernel-integration tests, not substitutes for target port validation.
if(WIN32 AND MSVC)
  set(_kernel "${PROJECT_SOURCE_DIR}/third_party/FreeRTOS-Kernel-11.3.1")
  foreach(_width 16 32 64)
    set(_target uetd_uos_freertos_${_width})
    add_executable(${_target} uos_freertos_contracts.c
      "${PROJECT_SOURCE_DIR}/src/uos/freertos.c" "${PROJECT_SOURCE_DIR}/src/uos/common.c"
      "${PROJECT_SOURCE_DIR}/src/uhal/platform/host/core.c" "${PROJECT_SOURCE_DIR}/src/uhal/platform/host/timing.c"
      "${_kernel}/tasks.c" "${_kernel}/list.c" "${_kernel}/queue.c"
      "${_kernel}/event_groups.c" "${_kernel}/timers.c" "${_kernel}/portable/MSVC-MingW/port.c")
    target_include_directories(${_target} PRIVATE "${PROJECT_SOURCE_DIR}/include"
      "${CMAKE_CURRENT_LIST_DIR}" "${_kernel}/include" "${_kernel}/portable/MSVC-MingW")
    target_compile_definitions(${_target} PRIVATE UHAL_HOST_SIMULATION=1 UOS_FREERTOS_NATIVE_STORAGE=1 UOS_TEST_TICK_WIDTH=TICK_TYPE_WIDTH_${_width}_BITS)
    target_compile_features(${_target} PRIVATE c_std_11)
    target_link_libraries(${_target} PRIVATE winmm)
    add_test(NAME uetd.uos_freertos_${_width} COMMAND ${_target})
    set_tests_properties(uetd.uos_freertos_${_width} PROPERTIES TIMEOUT 20)
  endforeach()
endif()

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
  execute_process(COMMAND "${Python3_EXECUTABLE}" -c "import jinja2" RESULT_VARIABLE _jinja_result)
  if(_jinja_result EQUAL 0)
    set(_manual_tasks "${CMAKE_CURRENT_BINARY_DIR}/manual-tasks")
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/render_manual_runtime.py"
      "${CMAKE_CURRENT_LIST_DIR}/manual_tasks.json" "${_manual_tasks}" COMMAND_ERROR_IS_FATAL ANY)
    add_executable(uetd_manual_tasks_contracts manual_tasks_contracts.c "${_manual_tasks}/tasks.c")
    target_include_directories(uetd_manual_tasks_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include" "${_manual_tasks}")
    target_compile_features(uetd_manual_tasks_contracts PRIVATE c_std_11)
    if(WIN32 AND MSVC)
      # Compile the generated FreeRTOSConfig and native stack/control types too;
      # this executable substitutes only task scheduling calls to inspect cadence.
      target_include_directories(uetd_manual_tasks_contracts PRIVATE "${CMAKE_CURRENT_LIST_DIR}"
        "${_kernel}/include" "${_kernel}/portable/MSVC-MingW")
      target_compile_definitions(uetd_manual_tasks_contracts PRIVATE UOS_FREERTOS_NATIVE_STORAGE=1)
    endif()
    add_test(NAME uetd.manual_tasks_contracts COMMAND uetd_manual_tasks_contracts)
    add_test(NAME uetd.manual_tasks_creation_failure COMMAND uetd_manual_tasks_contracts fail)
    add_test(NAME uetd.manual_runtime_templates COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/test_manual_runtime.py")
  endif()
endif()
