cmake_minimum_required(VERSION 3.20)
project(UEF VERSION 1.1.0 LANGUAGES C)

option(UEF_BUILD_EXAMPLE "Build the portable component-lifecycle example" OFF)
# The scalar choice affects UMATH/UCON ABI; the accumulator choice controls
# numerical precision and cost. Publish both on UEF::Core for consistent use.
set(UEF_UMATH_SCALAR_TYPE "uef_f32_t" CACHE STRING
  "UMATH public scalar type: uef_f32_t or uef_f64_t")
set_property(CACHE UEF_UMATH_SCALAR_TYPE PROPERTY STRINGS uef_f32_t uef_f64_t)
if(NOT UEF_UMATH_SCALAR_TYPE STREQUAL "uef_f32_t" AND
   NOT UEF_UMATH_SCALAR_TYPE STREQUAL "uef_f64_t")
  message(FATAL_ERROR "UEF_UMATH_SCALAR_TYPE must be uef_f32_t or uef_f64_t")
endif()
set(UEF_UMATH_ACCUMULATOR_TYPE "SCALAR" CACHE STRING
  "UMATH intermediate type: SCALAR, uef_f32_t, or uef_f64_t")
set_property(CACHE UEF_UMATH_ACCUMULATOR_TYPE PROPERTY STRINGS SCALAR uef_f32_t uef_f64_t)
if(NOT UEF_UMATH_ACCUMULATOR_TYPE STREQUAL "SCALAR" AND
   NOT UEF_UMATH_ACCUMULATOR_TYPE STREQUAL "uef_f32_t" AND
   NOT UEF_UMATH_ACCUMULATOR_TYPE STREQUAL "uef_f64_t")
  message(FATAL_ERROR
    "UEF_UMATH_ACCUMULATOR_TYPE must be SCALAR, uef_f32_t, or uef_f64_t")
endif()
set(UEF_UMATH_DEFINITIONS UMATH_SCALAR_TYPE=${UEF_UMATH_SCALAR_TYPE})
if(NOT UEF_UMATH_ACCUMULATOR_TYPE STREQUAL "SCALAR")
  list(APPEND UEF_UMATH_DEFINITIONS
    UMATH_ACCUMULATOR_TYPE=${UEF_UMATH_ACCUMULATOR_TYPE})
endif()
# Select one target backend at configure time; the host simulator and Cortex-M CMSIS code provide
# the same UHAL contract and must never be compiled into one firmware/runtime library together.
set(UEF_TARGET "HOST" CACHE STRING "UHAL implementation: HOST or CORTEX_M")
set_property(CACHE UEF_TARGET PROPERTY STRINGS HOST CORTEX_M)
set(UEF_UOS_BACKEND "BAREMETAL" CACHE STRING "UOS backend: BAREMETAL or FREERTOS")
set_property(CACHE UEF_UOS_BACKEND PROPERTY STRINGS BAREMETAL FREERTOS)
set(UEF_CMSIS_INCLUDE_DIR "" CACHE PATH "CMSIS/device include directory for Cortex-M builds")
set(UEF_CMSIS_DEVICE_HEADER "" CACHE STRING "Quoted CMSIS device header name, for example stm32h743xx.h")
set(UEF_FREERTOS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos-kernel" CACHE PATH
  "Pinned UEF-managed FreeRTOS Kernel source root")
set(UEF_FREERTOS_PROFILE "" CACHE STRING
  "Exact architecture/compiler profile key from the UEF integration manifest")
set(UEF_FREERTOS_CONFIG_INCLUDE_DIR "" CACHE PATH
  "Generated project directory containing FreeRTOSConfig.h")
option(UEF_ENABLE_FATFS "Build the optional Phase 1 UMID diskio binding against an external FatFS package" OFF)
set(UEF_FATFS_INCLUDE_DIR "" CACHE PATH "Directory containing FatFS ff.h and diskio.h")
set(UEF_FATFS_TARGET "" CACHE STRING "Optional existing CMake target that provides FatFS")
option(UEF_BUILD_FUTURE_UCON_SCAFFOLDS
  "Compile fail-closed UCON scaffolds catalogued as planned for later work" OFF)
option(UEF_BUILD_FUTURE_UMATH_SCAFFOLDS
  "Compile fail-closed UMATH outlines that remain under src/umath/future" OFF)
option(UEF_BUILD_FUTURE_UPROTO_SCAFFOLDS
  "Compile fail-closed UPROTO protocol outlines that are not selectable yet" OFF)

function(uef_check_package_relative_path _relative _description)
  string(REPLACE "\\" "/" _normalized "${_relative}")
  if(IS_ABSOLUTE "${_relative}" OR "${_normalized}" MATCHES "^[A-Za-z]:" OR
     "${_normalized}" MATCHES "(^|/)\\.\\.(/|$)")
    message(FATAL_ERROR "${_description} must stay inside the selected UEF dependency package: ${_relative}")
  endif()
endfunction()

set(UEF_PORTABLE_SOURCES
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
)

# Each module has its own translation unit. CONFIGURE_DEPENDS lets a newly added peripheral or
# protocol module enter the build while keeping source ownership visible in the directory tree.
file(GLOB_RECURSE UEF_UPAL_SOURCES CONFIGURE_DEPENDS src/upal/*/*.c)
file(GLOB_RECURSE UEF_UMID_SOURCES CONFIGURE_DEPENDS src/umid/*.c)
file(GLOB_RECURSE UEF_UPROTO_SOURCES CONFIGURE_DEPENDS src/uproto/*.c)
if(NOT UEF_BUILD_FUTURE_UPROTO_SCAFFOLDS)
  # Planned protocol operation outlines stay out of ordinary firmware archives.
  list(FILTER UEF_UPROTO_SOURCES EXCLUDE REGEX "/future/")
endif()
# Phase 0/1 UMATH outline sources are compiled as fail-closed symbols for API visibility;
# only the later catalogue outlines under /future/ are excluded from ordinary firmware archives.
file(GLOB_RECURSE UEF_UMATH_SOURCES CONFIGURE_DEPENDS src/umath/*.c)
if(NOT UEF_BUILD_FUTURE_UMATH_SCAFFOLDS)
  list(FILTER UEF_UMATH_SOURCES EXCLUDE REGEX "/future/")
endif()
file(GLOB_RECURSE UEF_UCON_SOURCES CONFIGURE_DEPENDS src/ucon/*.c)
if(NOT UEF_BUILD_FUTURE_UCON_SCAFFOLDS)
  # Later catalog entries remain in the repository for implementation work but
  # do not enter ordinary firmware libraries unless a developer opts in.
  list(FILTER UEF_UCON_SOURCES EXCLUDE REGEX "/future/")
endif()
if(UEF_ENABLE_FATFS)
  if(NOT UEF_FATFS_INCLUDE_DIR)
    message(FATAL_ERROR "UEF_ENABLE_FATFS requires UEF_FATFS_INCLUDE_DIR")
  endif()
else()
  # The diskio adapter includes third-party FatFS headers and remains opt-in for ordinary UEF builds.
  list(FILTER UEF_UMID_SOURCES EXCLUDE REGEX "/fatfs\\.c$")
endif()

# Pick exactly one architecture implementation for UHAL and publish its target macros to clients.
if(UEF_TARGET STREQUAL "HOST")
  file(GLOB_RECURSE UEF_UHAL_SOURCES CONFIGURE_DEPENDS src/uhal/x86/*.c)
  set(UEF_TARGET_DEFINITIONS UHAL_HOST_SIMULATION=1)
elseif(UEF_TARGET STREQUAL "CORTEX_M")
  if(NOT UEF_CMSIS_INCLUDE_DIR OR NOT UEF_CMSIS_DEVICE_HEADER)
    message(FATAL_ERROR
      "CORTEX_M requires UEF_CMSIS_INCLUDE_DIR and UEF_CMSIS_DEVICE_HEADER")
  endif()
  file(GLOB_RECURSE UEF_UHAL_SOURCES CONFIGURE_DEPENDS src/uhal/arm_cm/*.c)
  set(UEF_TARGET_DEFINITIONS
    UEF_TARGET_CORTEX_M=1
    UEF_CMSIS_DEVICE_HEADER=\"${UEF_CMSIS_DEVICE_HEADER}\")
else()
  message(FATAL_ERROR "UEF_TARGET must be HOST or CORTEX_M")
endif()

# UOS also has mutually exclusive backends: bare-metal synchronization reports unsupported rather
# than pretending to provide tasking, while the FreeRTOS adapter forwards to the pinned UEF-managed kernel.
if(UEF_UOS_BACKEND STREQUAL "BAREMETAL")
  set(UEF_UOS_SOURCES src/uos/baremetal/uos_baremetal.c)
elseif(UEF_UOS_BACKEND STREQUAL "FREERTOS")
  if(NOT EXISTS "${UEF_FREERTOS_ROOT}/tasks.c")
    message(FATAL_ERROR
      "The UEF-managed FreeRTOS Kernel checkout is missing. Populate the pinned source under third_party/freertos-kernel before selecting FREERTOS.")
  endif()
  set(_freertos_lock "${CMAKE_CURRENT_SOURCE_DIR}/third_party/freertos-kernel.lock.json")
  set(_freertos_integration "${UEF_FREERTOS_ROOT}/uef-integration.json")
  if(NOT EXISTS "${_freertos_lock}" OR NOT EXISTS "${_freertos_integration}")
    message(FATAL_ERROR "FreeRTOS lock and UEF integration manifest are required")
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
  if(NOT UEF_FREERTOS_PROFILE)
    message(FATAL_ERROR "FREERTOS requires UEF_FREERTOS_PROFILE from uef-integration.json")
  endif()
  string(JSON _freertos_profile_type ERROR_VARIABLE _freertos_profile_error
    TYPE "${_freertos_integration_json}" profiles "${UEF_FREERTOS_PROFILE}")
  if(_freertos_profile_error OR NOT _freertos_profile_type STREQUAL "OBJECT")
    message(FATAL_ERROR "Unknown FreeRTOS profile '${UEF_FREERTOS_PROFILE}' in ${_freertos_integration}")
  endif()
  string(JSON _freertos_profile_architecture ERROR_VARIABLE _freertos_profile_arch_error
    GET "${_freertos_integration_json}" profiles "${UEF_FREERTOS_PROFILE}" architecture)
  string(JSON _freertos_profile_compiler ERROR_VARIABLE _freertos_profile_compiler_error
    GET "${_freertos_integration_json}" profiles "${UEF_FREERTOS_PROFILE}" compiler)
  if(_freertos_profile_arch_error OR _freertos_profile_compiler_error OR
     NOT _freertos_profile_architecture OR NOT _freertos_profile_compiler)
    message(FATAL_ERROR
      "FreeRTOS profile '${UEF_FREERTOS_PROFILE}' must declare architecture and compiler")
  endif()
  if(NOT UEF_FREERTOS_CONFIG_INCLUDE_DIR)
    message(FATAL_ERROR
      "FREERTOS requires the generated project directory containing FreeRTOSConfig.h.")
  endif()
  string(JSON _common_source_count LENGTH "${_freertos_integration_json}" common_sources)
  set(_freertos_sources)
  if(_common_source_count GREATER 0)
    math(EXPR _common_source_last "${_common_source_count} - 1")
    foreach(_index RANGE 0 ${_common_source_last})
      string(JSON _relative GET "${_freertos_integration_json}" common_sources ${_index})
      uef_check_package_relative_path("${_relative}" "FreeRTOS common source path")
      list(APPEND _freertos_sources "${UEF_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  string(JSON _port_source_count LENGTH "${_freertos_integration_json}"
    profiles "${UEF_FREERTOS_PROFILE}" port_sources)
  if(_port_source_count GREATER 0)
    math(EXPR _port_source_last "${_port_source_count} - 1")
    foreach(_index RANGE 0 ${_port_source_last})
      string(JSON _relative GET "${_freertos_integration_json}"
        profiles "${UEF_FREERTOS_PROFILE}" port_sources ${_index})
      uef_check_package_relative_path("${_relative}" "FreeRTOS port source path")
      list(APPEND _freertos_sources "${UEF_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  string(JSON _heap_source GET "${_freertos_integration_json}"
    profiles "${UEF_FREERTOS_PROFILE}" heap_source)
  if(_heap_source)
    uef_check_package_relative_path("${_heap_source}" "FreeRTOS heap source path")
    list(APPEND _freertos_sources "${UEF_FREERTOS_ROOT}/${_heap_source}")
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
      uef_check_package_relative_path("${_relative}" "FreeRTOS common include path")
      list(APPEND _freertos_include_directories "${UEF_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  string(JSON _port_include_count LENGTH "${_freertos_integration_json}"
    profiles "${UEF_FREERTOS_PROFILE}" include_directories)
  if(_port_include_count GREATER 0)
    math(EXPR _port_include_last "${_port_include_count} - 1")
    foreach(_index RANGE 0 ${_port_include_last})
      string(JSON _relative GET "${_freertos_integration_json}"
        profiles "${UEF_FREERTOS_PROFILE}" include_directories ${_index})
      uef_check_package_relative_path("${_relative}" "FreeRTOS profile include path")
      list(APPEND _freertos_include_directories "${UEF_FREERTOS_ROOT}/${_relative}")
    endforeach()
  endif()
  foreach(_freertos_include IN LISTS _freertos_include_directories)
    if(NOT IS_DIRECTORY "${_freertos_include}")
      message(FATAL_ERROR "Selected FreeRTOS include directory does not exist: ${_freertos_include}")
    endif()
  endforeach()
  string(JSON _freertos_define_count LENGTH "${_freertos_integration_json}"
    profiles "${UEF_FREERTOS_PROFILE}" defines)
  set(_freertos_definitions)
  if(_freertos_define_count GREATER 0)
    math(EXPR _freertos_define_last "${_freertos_define_count} - 1")
    foreach(_index RANGE 0 ${_freertos_define_last})
      string(JSON _define GET "${_freertos_integration_json}"
        profiles "${UEF_FREERTOS_PROFILE}" defines ${_index})
      if(_define MATCHES "[;\r\n]" OR _define MATCHES "^-")
        message(FATAL_ERROR "Unsafe FreeRTOS compile definition in profile '${UEF_FREERTOS_PROFILE}': ${_define}")
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
    uef_check_package_relative_path("${_relative}" "FreeRTOS license file path")
    set(_license_path "${UEF_FREERTOS_ROOT}/${_relative}")
    if(NOT EXISTS "${_license_path}")
      message(FATAL_ERROR "FreeRTOS license file does not exist: ${_license_path}")
    endif()
    list(APPEND _freertos_license_paths "${_license_path}")
  endforeach()
  set(UEF_UOS_SOURCES src/uos/freertos/uos_freertos.c)
else()
  message(FATAL_ERROR "UEF_UOS_BACKEND must be BAREMETAL or FREERTOS")
endif()

add_library(uef_core STATIC
  ${UEF_PORTABLE_SOURCES}
  ${UEF_UHAL_SOURCES}
  ${UEF_UOS_SOURCES}
  ${UEF_UPAL_SOURCES}
  ${UEF_UMID_SOURCES}
  ${UEF_UPROTO_SOURCES}
  ${UEF_UMATH_SOURCES}
  ${UEF_UCON_SOURCES})
add_library(UEF::Core ALIAS uef_core)

target_include_directories(uef_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(uef_core PUBLIC c_std_11)
set_target_properties(uef_core PROPERTIES
  C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
  EXPORT_NAME Core)
target_compile_definitions(uef_core PUBLIC
  ${UEF_TARGET_DEFINITIONS}
  ${UEF_UMATH_DEFINITIONS})

if(UEF_TARGET STREQUAL "CORTEX_M")
  target_include_directories(uef_core PUBLIC ${UEF_CMSIS_INCLUDE_DIR})
endif()
if(UEF_UOS_BACKEND STREQUAL "FREERTOS")
  target_sources(uef_core PRIVATE ${_freertos_sources})
  target_compile_definitions(uef_core PRIVATE ${_freertos_definitions})
  target_include_directories(uef_core PRIVATE
    ${_freertos_include_directories}
    "${UEF_FREERTOS_CONFIG_INCLUDE_DIR}")
endif()
if(UEF_ENABLE_FATFS)
  target_include_directories(uef_core PUBLIC ${UEF_FATFS_INCLUDE_DIR})
  target_compile_definitions(uef_core PRIVATE UEF_ENABLE_FATFS=1)
  if(UEF_FATFS_TARGET)
    target_link_libraries(uef_core PUBLIC ${UEF_FATFS_TARGET})
  endif()
endif()

if(MSVC)
  target_compile_options(uef_core PRIVATE /W4)
else()
  target_compile_options(uef_core PRIVATE -Wall -Wextra -Wpedantic)
  target_link_libraries(uef_core PUBLIC m)
endif()

if(UEF_BUILD_EXAMPLE)
  add_executable(uef_example examples/minimal/main.c)
  target_link_libraries(uef_example PRIVATE UEF::Core)
endif()

include(GNUInstallDirs)
install(TARGETS uef_core EXPORT UEFTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT UEFTargets NAMESPACE UEF:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/UEF)
if(UEF_UOS_BACKEND STREQUAL "FREERTOS")
  install(FILES ${_freertos_license_paths}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/UEF/FreeRTOS-Kernel)
endif()
