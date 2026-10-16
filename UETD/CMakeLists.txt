cmake_minimum_required(VERSION 3.20)
project(UETD VERSION 1.1.0 LANGUAGES C)

# UAPP FSM capacities affect the public machine/event layout. Cache them once
# and publish the same values to UETD and every consumer of UETD::Core.
set(UAPP_SM_MAX_STATES "16" CACHE STRING "Maximum states in one UAPP FSM (1..255)")
set(UAPP_SM_MAX_TRANSITIONS "32" CACHE STRING "Maximum transition rows in one UAPP FSM (0..255)")
set(UAPP_SM_EVENT_QUEUE_CAPACITY "8" CACHE STRING "Events stored in one UAPP FSM queue (1..255)")
set(UAPP_SM_EVENT_PAYLOAD_CAPACITY "16" CACHE STRING "Payload bytes stored in one UAPP FSM event (1..255)")
foreach(_uapp_sm_capacity IN ITEMS
    UAPP_SM_MAX_STATES UAPP_SM_MAX_TRANSITIONS
    UAPP_SM_EVENT_QUEUE_CAPACITY UAPP_SM_EVENT_PAYLOAD_CAPACITY)
  if(NOT "${${_uapp_sm_capacity}}" MATCHES "^[0-9]+$")
    message(FATAL_ERROR "${_uapp_sm_capacity} must be a nonnegative integer")
  endif()
endforeach()
if(UAPP_SM_MAX_STATES LESS 1 OR UAPP_SM_MAX_STATES GREATER 255)
  message(FATAL_ERROR "UAPP_SM_MAX_STATES must be in [1, 255]")
endif()
if(UAPP_SM_MAX_TRANSITIONS GREATER 255)
  message(FATAL_ERROR "UAPP_SM_MAX_TRANSITIONS must be in [0, 255]")
endif()
if(UAPP_SM_EVENT_QUEUE_CAPACITY LESS 1 OR UAPP_SM_EVENT_QUEUE_CAPACITY GREATER 255)
  message(FATAL_ERROR "UAPP_SM_EVENT_QUEUE_CAPACITY must be in [1, 255]")
endif()
if(UAPP_SM_EVENT_PAYLOAD_CAPACITY LESS 1 OR UAPP_SM_EVENT_PAYLOAD_CAPACITY GREATER 255)
  message(FATAL_ERROR "UAPP_SM_EVENT_PAYLOAD_CAPACITY must be in [1, 255]")
endif()

option(UETD_BUILD_EXAMPLE "Build the portable component-lifecycle example" OFF)
option(UETD_BUILD_TESTS "Build UETD's scaffold contract tests" OFF)
# The scalar choice affects UMATH/UCON ABI; the accumulator choice controls
# numerical precision and cost. Publish both on UETD::Core for consistent use.
set(UETD_UMATH_SCALAR_TYPE "uetd_f32_t" CACHE STRING
  "UMATH public scalar type: uetd_f32_t or uetd_f64_t")
set_property(CACHE UETD_UMATH_SCALAR_TYPE PROPERTY STRINGS uetd_f32_t uetd_f64_t)
if(NOT UETD_UMATH_SCALAR_TYPE STREQUAL "uetd_f32_t" AND
   NOT UETD_UMATH_SCALAR_TYPE STREQUAL "uetd_f64_t")
  message(FATAL_ERROR "UETD_UMATH_SCALAR_TYPE must be uetd_f32_t or uetd_f64_t")
endif()
set(UETD_UMATH_ACCUMULATOR_TYPE "SCALAR" CACHE STRING
  "UMATH intermediate type: SCALAR, uetd_f32_t, or uetd_f64_t")
set_property(CACHE UETD_UMATH_ACCUMULATOR_TYPE PROPERTY STRINGS SCALAR uetd_f32_t uetd_f64_t)
if(NOT UETD_UMATH_ACCUMULATOR_TYPE STREQUAL "SCALAR" AND
   NOT UETD_UMATH_ACCUMULATOR_TYPE STREQUAL "uetd_f32_t" AND
   NOT UETD_UMATH_ACCUMULATOR_TYPE STREQUAL "uetd_f64_t")
  message(FATAL_ERROR
    "UETD_UMATH_ACCUMULATOR_TYPE must be SCALAR, uetd_f32_t, or uetd_f64_t")
endif()
set(UETD_UMATH_DEFINITIONS UMATH_SCALAR_TYPE=${UETD_UMATH_SCALAR_TYPE})
if(NOT UETD_UMATH_ACCUMULATOR_TYPE STREQUAL "SCALAR")
  list(APPEND UETD_UMATH_DEFINITIONS
    UMATH_ACCUMULATOR_TYPE=${UETD_UMATH_ACCUMULATOR_TYPE})
endif()
option(UETD_UMATH_ALLOW_FP_CONTRACTION
  "Allow the compiler to fuse ordinary multiply-add expressions (explicit umath_fma is always fused)" OFF)
set(UETD_UMATH_TARGET_SCALAR_MATH_HEADER "" CACHE STRING
  "Optional header that overrides internal UMATH math dispatch for the selected target")
# Select one target backend at configure time; the host simulator and Cortex-M CMSIS code provide
# the same UHAL contract and must never be compiled into one firmware/runtime library together.
set(UETD_TARGET "HOST" CACHE STRING "UHAL implementation: HOST or CORTEX_M")
set_property(CACHE UETD_TARGET PROPERTY STRINGS HOST CORTEX_M)
set(UETD_UOS_BACKEND "BAREMETAL" CACHE STRING "UOS backend: BAREMETAL or FREERTOS")
set_property(CACHE UETD_UOS_BACKEND PROPERTY STRINGS BAREMETAL FREERTOS)
set(UETD_CMSIS_INCLUDE_DIR "" CACHE PATH "CMSIS/device include directory for Cortex-M builds")
set(UETD_CMSIS_DEVICE_HEADER "" CACHE STRING "Quoted CMSIS device header name, for example stm32h743xx.h")
set(_UETD_FREERTOS_DEFAULT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FreeRTOS-Kernel-11.3.1")
set(UETD_FREERTOS_ROOT "${_UETD_FREERTOS_DEFAULT_ROOT}" CACHE PATH
  "UETD-managed FreeRTOS Kernel 11.3.1 source root")
# Existing build directories may cache the former empty placeholder path. Move
# that stale default to the now-populated source tree on the next configure.
if(NOT EXISTS "${UETD_FREERTOS_ROOT}/tasks.c" AND
   EXISTS "${_UETD_FREERTOS_DEFAULT_ROOT}/tasks.c")
  set(UETD_FREERTOS_ROOT "${_UETD_FREERTOS_DEFAULT_ROOT}" CACHE PATH
    "UETD-managed FreeRTOS Kernel 11.3.1 source root" FORCE)
endif()
set(UETD_FREERTOS_PROFILE "" CACHE STRING
  "Exact architecture/compiler profile key from the UETD integration manifest")
set(UETD_FREERTOS_CONFIG_INCLUDE_DIR "" CACHE PATH
  "Generated project directory containing FreeRTOSConfig.h")
option(UETD_ENABLE_FATFS "Build the optional Phase 1 UMID diskio binding against an external FatFS package" OFF)
set(UETD_FATFS_INCLUDE_DIR "" CACHE PATH "Directory containing FatFS ff.h and diskio.h")
set(UETD_FATFS_TARGET "" CACHE STRING "Optional existing CMake target that provides FatFS")
option(UETD_BUILD_FUTURE_UCON_SCAFFOLDS
  "Compile fail-closed UCON scaffolds catalogued as planned for later work" OFF)
option(UETD_BUILD_FUTURE_UMATH_SCAFFOLDS
  "Compile fail-closed UMATH outlines that remain under src/umath/future" OFF)
option(UETD_BUILD_FUTURE_UPROTO_SCAFFOLDS
  "Compile fail-closed UPROTO protocol outlines that are not selectable yet" OFF)

function(uetd_check_package_relative_path _relative _description)
  string(REPLACE "\\" "/" _normalized "${_relative}")
  if(IS_ABSOLUTE "${_relative}" OR "${_normalized}" MATCHES "^[A-Za-z]:" OR
     "${_normalized}" MATCHES "(^|/)\\.\\.(/|$)")
    message(FATAL_ERROR "${_description} must stay inside the selected UETD dependency package: ${_relative}")
  endif()
endfunction()

set(UETD_PORTABLE_SOURCES
  src/ucore/assert.c
  src/ucore/bitops.c
  src/ucore/checksum.c
  src/ucore/status.c
  src/uhal/target_profiles.c
  src/uhal/targets/stm32g4/profile.c
  src/uhal/targets/stm32f4/profile.c
  src/uhal/targets/stm32h7/profile.c
  src/uhal/targets/stm32f7/profile.c
  src/uhal/targets/stm32g0/profile.c
  src/uhal/targets/nrf52840/profile.c
  src/uhal/targets/stm32l4plus/profile.c
  src/uapp/fault.c
  src/uapp/lifecycle.c
  src/uapp/statemachine.c
  src/uapp/supervisor.c
  src/uapp/watchdog_gate.c
)

# Each module has its own translation unit. CONFIGURE_DEPENDS lets a newly added peripheral or
# protocol module enter the build while keeping source ownership visible in the directory tree.
file(GLOB_RECURSE UETD_UPAL_SOURCES CONFIGURE_DEPENDS src/upal/*/*.c)
# This recursive source glob includes the manually maintained UMID driver tree.
file(GLOB_RECURSE UETD_UMID_SOURCES CONFIGURE_DEPENDS src/umid/*.c)
file(GLOB_RECURSE UETD_UPROTO_SOURCES CONFIGURE_DEPENDS src/uproto/*.c)
if(NOT UETD_BUILD_FUTURE_UPROTO_SCAFFOLDS)
  # Planned protocol operation outlines stay out of ordinary firmware archives.
  list(FILTER UETD_UPROTO_SOURCES EXCLUDE REGEX "/future/")
endif()
# Phase 0/1 UMATH outline sources are compiled as fail-closed symbols for API visibility;
# only the later catalogue outlines under /future/ are excluded from ordinary firmware archives.
file(GLOB_RECURSE UETD_UMATH_SOURCES CONFIGURE_DEPENDS src/umath/*.c)
if(NOT UETD_BUILD_FUTURE_UMATH_SCAFFOLDS)
  list(FILTER UETD_UMATH_SOURCES EXCLUDE REGEX "/future/")
endif()
file(GLOB_RECURSE UETD_UCON_SOURCES CONFIGURE_DEPENDS src/ucon/*.c)
if(NOT UETD_BUILD_FUTURE_UCON_SCAFFOLDS)
  # Later catalog entries remain in the repository for implementation work but
  # do not enter ordinary firmware libraries unless a developer opts in.
  list(FILTER UETD_UCON_SOURCES EXCLUDE REGEX "/future/")
endif()
if(UETD_ENABLE_FATFS)
  if(NOT UETD_FATFS_INCLUDE_DIR)
    message(FATAL_ERROR "UETD_ENABLE_FATFS requires UETD_FATFS_INCLUDE_DIR")
  endif()
else()
  # The diskio adapter includes third-party FatFS headers and remains opt-in for ordinary UETD builds.
  list(FILTER UETD_UMID_SOURCES EXCLUDE REGEX "/fatfs\\.c$")
endif()

# Pick exactly one architecture implementation for UHAL and publish its target macros to clients.
if(UETD_TARGET STREQUAL "HOST")
  file(GLOB_RECURSE UETD_UHAL_SOURCES CONFIGURE_DEPENDS src/uhal/x86/*.c)
  set(UETD_TARGET_DEFINITIONS UHAL_HOST_SIMULATION=1)
elseif(UETD_TARGET STREQUAL "CORTEX_M")
  if(NOT UETD_CMSIS_INCLUDE_DIR OR NOT UETD_CMSIS_DEVICE_HEADER)
    message(FATAL_ERROR
      "CORTEX_M requires UETD_CMSIS_INCLUDE_DIR and UETD_CMSIS_DEVICE_HEADER")
  endif()
  file(GLOB_RECURSE UETD_UHAL_SOURCES CONFIGURE_DEPENDS src/uhal/arm_cm/*.c)
  set(UETD_TARGET_DEFINITIONS
    UETD_TARGET_CORTEX_M=1
    UETD_CMSIS_DEVICE_HEADER=\"${UETD_CMSIS_DEVICE_HEADER}\")
else()
  message(FATAL_ERROR
    "UETD_TARGET='${UETD_TARGET}' has no implemented UHAL backend. The currently "
    "selectable implementations are HOST and CORTEX_M. AVR and ESP-IDF target "
    "work is scaffolded under src/uhal/avr and src/uhal/esp_idf; complete the "
    "architecture-neutral IRQ/critical-section contract before adding either "
    "backend to this selector.")
endif()

# UOS also has mutually exclusive backends: bare-metal synchronization reports unsupported rather
# than pretending to provide tasking, while the FreeRTOS adapter forwards to the pinned UETD-managed kernel.
if(UETD_UOS_BACKEND STREQUAL "BAREMETAL")
  set(UETD_UOS_SOURCES src/uos/baremetal/baremetal.c)
elseif(UETD_UOS_BACKEND STREQUAL "FREERTOS")
  if(NOT EXISTS "${UETD_FREERTOS_ROOT}/tasks.c")
    message(FATAL_ERROR
      "The UETD-managed FreeRTOS Kernel source is missing. Expected third_party/FreeRTOS-Kernel-11.3.1 or set UETD_FREERTOS_ROOT.")
  endif()
  set(_freertos_lock "${UETD_FREERTOS_ROOT}/freertos-kernel.lock.json")
  set(_freertos_integration "${UETD_FREERTOS_ROOT}/uetd-integration.json")
  if(NOT EXISTS "${_freertos_lock}" OR NOT EXISTS "${_freertos_integration}")
    message(FATAL_ERROR "FreeRTOS lock and UETD integration manifest are required")
  endif()
  file(READ "${_freertos_lock}" _freertos_lock_json)
  string(JSON _freertos_revision ERROR_VARIABLE _freertos_lock_error
    GET "${_freertos_lock_json}" revision)
  string(LENGTH "${_freertos_revision}" _freertos_revision_length)
  if(_freertos_lock_error OR NOT _freertos_revision_length EQUAL 40 OR
     NOT _freertos_revision MATCHES "^[0-9a-fA-F]+$")
    message(FATAL_ERROR "FreeRTOS is not pinned to a full upstream commit in ${_freertos_lock}")
  endif()
  file(READ "${_freertos_integration}" _freertos_integration_json)
  if(NOT UETD_FREERTOS_PROFILE)
    message(FATAL_ERROR "FREERTOS requires UETD_FREERTOS_PROFILE from uetd-integration.json")
  endif()
  string(JSON _freertos_profile_type ERROR_VARIABLE _freertos_profile_error
    TYPE "${_freertos_integration_json}" profiles "${UETD_FREERTOS_PROFILE}")
  if(_freertos_profile_error OR NOT _freertos_profile_type STREQUAL "OBJECT")
    message(FATAL_ERROR "Unknown FreeRTOS profile '${UETD_FREERTOS_PROFILE}' in ${_freertos_integration}")
  endif()
  string(JSON _freertos_profile_architecture ERROR_VARIABLE _freertos_profile_arch_error
    GET "${_freertos_integration_json}" profiles "${UETD_FREERTOS_PROFILE}" architecture)
  string(JSON _freertos_profile_compiler ERROR_VARIABLE _freertos_profile_compiler_error
    GET "${_freertos_integration_json}" profiles "${UETD_FREERTOS_PROFILE}" compiler)
  if(_freertos_profile_arch_error OR _freertos_profile_compiler_error OR
     NOT _freertos_profile_architecture OR NOT _freertos_profile_compiler)
    message(FATAL_ERROR
      "FreeRTOS profile '${UETD_FREERTOS_PROFILE}' must declare architecture and compiler")
  endif()
  if(NOT UETD_FREERTOS_CONFIG_INCLUDE_DIR)
    message(FATAL_ERROR
      "FREERTOS requires the generated project directory containing FreeRTOSConfig.h.")
  endif()
  string(JSON _common_source_count LENGTH "${_freertos_integration_json}" common_sources)
  set(_freertos_sources)
  if(_common_source_count GREATER 0)
    math(EXPR _common_source_last "${_common_source_count} - 1")
    foreach(_index RANGE 0 ${_common_source_last})
      string(JSON _relative GET "${_freertos_integration_json}" common_sources ${_index})
      uetd_check_package_relative_path("${_relative}" "FreeRTOS common source path")
      list(APPEND _freertos_sources "${UETD_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  string(JSON _port_source_count LENGTH "${_freertos_integration_json}"
    profiles "${UETD_FREERTOS_PROFILE}" port_sources)
  if(_port_source_count GREATER 0)
    math(EXPR _port_source_last "${_port_source_count} - 1")
    foreach(_index RANGE 0 ${_port_source_last})
      string(JSON _relative GET "${_freertos_integration_json}"
        profiles "${UETD_FREERTOS_PROFILE}" port_sources ${_index})
      uetd_check_package_relative_path("${_relative}" "FreeRTOS port source path")
      list(APPEND _freertos_sources "${UETD_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  string(JSON _heap_source GET "${_freertos_integration_json}"
    profiles "${UETD_FREERTOS_PROFILE}" heap_source)
  if(_heap_source)
    uetd_check_package_relative_path("${_heap_source}" "FreeRTOS heap source path")
    list(APPEND _freertos_sources "${UETD_FREERTOS_ROOT}/${_heap_source}")
  endif()
  foreach(_freertos_source IN LISTS _freertos_sources)
    if(NOT EXISTS "${_freertos_source}")
      message(FATAL_ERROR "Selected FreeRTOS source does not exist: ${_freertos_source}")
    endif()
  endforeach()
  string(JSON _common_include_count LENGTH "${_freertos_integration_json}"
    common_include_directories)
  set(_freertos_include_directories)
  if(_common_include_count GREATER 0)
    math(EXPR _common_include_last "${_common_include_count} - 1")
    foreach(_index RANGE 0 ${_common_include_last})
      string(JSON _relative GET "${_freertos_integration_json}" common_include_directories ${_index})
      uetd_check_package_relative_path("${_relative}" "FreeRTOS common include path")
      list(APPEND _freertos_include_directories "${UETD_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  string(JSON _port_include_count LENGTH "${_freertos_integration_json}"
    profiles "${UETD_FREERTOS_PROFILE}" include_directories)
  if(_port_include_count GREATER 0)
    math(EXPR _port_include_last "${_port_include_count} - 1")
    foreach(_index RANGE 0 ${_port_include_last})
      string(JSON _relative GET "${_freertos_integration_json}"
        profiles "${UETD_FREERTOS_PROFILE}" include_directories ${_index})
      uetd_check_package_relative_path("${_relative}" "FreeRTOS profile include path")
      list(APPEND _freertos_include_directories "${UETD_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  foreach(_freertos_include IN LISTS _freertos_include_directories)
    if(NOT IS_DIRECTORY "${_freertos_include}")
      message(FATAL_ERROR "Selected FreeRTOS include directory does not exist: ${_freertos_include}")
    endif()
  endforeach()
  string(JSON _freertos_define_count LENGTH "${_freertos_integration_json}"
    profiles "${UETD_FREERTOS_PROFILE}" defines)
  set(_freertos_definitions)
  if(_freertos_define_count GREATER 0)
    math(EXPR _freertos_define_last "${_freertos_define_count} - 1")
    foreach(_index RANGE 0 ${_freertos_define_last})
      string(JSON _define GET "${_freertos_integration_json}"
        profiles "${UETD_FREERTOS_PROFILE}" defines ${_index})
      if(_define MATCHES "[;\r\n]" OR _define MATCHES "^-")
        message(FATAL_ERROR "Unsafe FreeRTOS compile definition in profile '${UETD_FREERTOS_PROFILE}': ${_define}")
      endif()
      list(APPEND _freertos_definitions "${_define}")
    endforeach()
  endif()
  string(JSON _freertos_license_count LENGTH "${_freertos_lock_json}" license_files)
  if(_freertos_license_count LESS 1)
    message(FATAL_ERROR "FreeRTOS lock must list upstream license text files for generated/installable projects")
  endif()
  set(_freertos_license_paths)
  math(EXPR _freertos_license_last "${_freertos_license_count} - 1")
  foreach(_index RANGE 0 ${_freertos_license_last})
    string(JSON _relative GET "${_freertos_lock_json}" license_files ${_index})
    uetd_check_package_relative_path("${_relative}" "FreeRTOS license file path")
    set(_license_path "${UETD_FREERTOS_ROOT}/${_relative}")
    if(NOT EXISTS "${_license_path}")
      message(FATAL_ERROR "FreeRTOS license file does not exist: ${_license_path}")
    endif()
    list(APPEND _freertos_license_paths "${_license_path}")
  endforeach()
  set(UETD_UOS_SOURCES src/uos/freertos/freertos.c)
else()
  message(FATAL_ERROR "UETD_UOS_BACKEND must be BAREMETAL or FREERTOS")
endif()

add_library(uetd_core STATIC
  ${UETD_PORTABLE_SOURCES}
  ${UETD_UHAL_SOURCES}
  ${UETD_UOS_SOURCES}
  ${UETD_UPAL_SOURCES}
  ${UETD_UMID_SOURCES}
  ${UETD_UPROTO_SOURCES}
  ${UETD_UMATH_SOURCES}
  ${UETD_UCON_SOURCES})
add_library(UETD::Core ALIAS uetd_core)

target_include_directories(uetd_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(uetd_core PUBLIC c_std_11)
set_target_properties(uetd_core PROPERTIES
  C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
  EXPORT_NAME Core)
target_compile_definitions(uetd_core PUBLIC
  ${UETD_TARGET_DEFINITIONS}
  ${UETD_UMATH_DEFINITIONS}
  UAPP_SM_MAX_STATES=${UAPP_SM_MAX_STATES}
  UAPP_SM_MAX_TRANSITIONS=${UAPP_SM_MAX_TRANSITIONS}
  UAPP_SM_EVENT_QUEUE_CAPACITY=${UAPP_SM_EVENT_QUEUE_CAPACITY}
  UAPP_SM_EVENT_PAYLOAD_CAPACITY=${UAPP_SM_EVENT_PAYLOAD_CAPACITY})

if(UETD_TARGET STREQUAL "CORTEX_M")
  target_include_directories(uetd_core PUBLIC ${UETD_CMSIS_INCLUDE_DIR})
endif()
if(UETD_UOS_BACKEND STREQUAL "FREERTOS")
  target_sources(uetd_core PRIVATE ${_freertos_sources})
  target_compile_definitions(uetd_core PRIVATE ${_freertos_definitions})
  target_include_directories(uetd_core PRIVATE
    ${_freertos_include_directories}
    "${UETD_FREERTOS_CONFIG_INCLUDE_DIR}")
endif()
if(UETD_ENABLE_FATFS)
  target_include_directories(uetd_core PUBLIC ${UETD_FATFS_INCLUDE_DIR})
  target_compile_definitions(uetd_core PRIVATE UETD_ENABLE_FATFS=1)
  if(UETD_FATFS_TARGET)
    target_link_libraries(uetd_core PUBLIC ${UETD_FATFS_TARGET})
  endif()
endif()

if(MSVC)
  target_compile_options(uetd_core PRIVATE /W4)
  if(MSVC_VERSION LESS 1930)
    if(UETD_UMATH_ALLOW_FP_CONTRACTION)
      message(FATAL_ERROR "UETD_UMATH_ALLOW_FP_CONTRACTION requires Visual Studio 2022 or newer")
    endif()
    # Older MSVC versions could contract under /fp:precise; strict keeps the
    # default separate-operation policy on those toolchains.
    target_compile_options(uetd_core PRIVATE /fp:strict)
  else()
    # Current MSVC disables implicit contraction under /fp:precise.
    target_compile_options(uetd_core PRIVATE /fp:precise)
    if(UETD_UMATH_ALLOW_FP_CONTRACTION)
      target_compile_options(uetd_core PRIVATE /fp:contract)
    endif()
  endif()
else()
  target_compile_options(uetd_core PRIVATE -Wall -Wextra -Wpedantic)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    if(UETD_UMATH_ALLOW_FP_CONTRACTION)
      target_compile_options(uetd_core PRIVATE -ffp-contract=on)
    else()
      target_compile_options(uetd_core PRIVATE -ffp-contract=off)
    endif()
  elseif(UETD_UMATH_ALLOW_FP_CONTRACTION)
    message(FATAL_ERROR
      "UETD_UMATH_ALLOW_FP_CONTRACTION is not configured for ${CMAKE_C_COMPILER_ID}; set the equivalent option in the toolchain")
  else()
    message(WARNING
      "Cannot enforce disabled FP contraction for ${CMAKE_C_COMPILER_ID}; configure the equivalent option in the toolchain")
  endif()
  target_link_libraries(uetd_core PUBLIC m)
endif()
if(UETD_UMATH_TARGET_SCALAR_MATH_HEADER)
  target_compile_definitions(uetd_core PRIVATE
    "UETD_UMATH_TARGET_SCALAR_MATH_HEADER=\"${UETD_UMATH_TARGET_SCALAR_MATH_HEADER}\"")
endif()

if(UETD_BUILD_EXAMPLE)
  add_executable(uetd_example examples/minimal/main.c)
  target_link_libraries(uetd_example PRIVATE UETD::Core)
endif()

if(UETD_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(TARGETS uetd_core EXPORT UETDTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT UETDTargets NAMESPACE UETD:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UETD)
if(UETD_UOS_BACKEND STREQUAL "FREERTOS")
  install(FILES ${_freertos_license_paths}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/UETD/FreeRTOS-Kernel)
endif()
