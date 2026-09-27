cmake_minimum_required(VERSION 3.20)
project(UEF VERSION 1.1.0 LANGUAGES C)

option(UEF_BUILD_EXAMPLE "Build the portable component-lifecycle example" OFF)
# Select one target backend at configure time; the host simulator and Cortex-M CMSIS code provide
# the same UHAL contract and must never be compiled into one firmware/runtime library together.
set(UEF_TARGET "HOST" CACHE STRING "UHAL implementation: HOST or CORTEX_M")
set_property(CACHE UEF_TARGET PROPERTY STRINGS HOST CORTEX_M)
set(UEF_UOS_BACKEND "BAREMETAL" CACHE STRING "UOS backend: BAREMETAL or FREERTOS")
set_property(CACHE UEF_UOS_BACKEND PROPERTY STRINGS BAREMETAL FREERTOS)
set(UEF_CMSIS_INCLUDE_DIR "" CACHE PATH "CMSIS/device include directory for Cortex-M builds")
set(UEF_CMSIS_DEVICE_HEADER "" CACHE STRING "Quoted CMSIS device header name, for example stm32h743xx.h")
set(UEF_FREERTOS_INCLUDE_DIR "" CACHE PATH "Directory containing FreeRTOS.h and FreeRTOSConfig.h")
set(UEF_FREERTOS_TARGET "" CACHE STRING "Optional existing CMake target that provides the FreeRTOS kernel")
option(UEF_ENABLE_FATFS "Build the optional UMID diskio binding against an external FatFS package" OFF)
set(UEF_FATFS_INCLUDE_DIR "" CACHE PATH "Directory containing FatFS ff.h and diskio.h")
set(UEF_FATFS_TARGET "" CACHE STRING "Optional existing CMake target that provides FatFS")

set(UEF_PORTABLE_SOURCES
  src/ucore/assert.c
  src/ucore/math.c
  src/ucore/status.c
  src/ucore/time.c
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
# than pretending to provide tasking, while the FreeRTOS adapter forwards to the supplied kernel.
if(UEF_UOS_BACKEND STREQUAL "BAREMETAL")
  set(UEF_UOS_SOURCES src/uos/baremetal/uos_baremetal.c)
elseif(UEF_UOS_BACKEND STREQUAL "FREERTOS")
  if(NOT UEF_FREERTOS_INCLUDE_DIR)
    message(FATAL_ERROR "FREERTOS requires UEF_FREERTOS_INCLUDE_DIR")
  endif()
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
  ${UEF_UPROTO_SOURCES})
add_library(UEF::Core ALIAS uef_core)

target_include_directories(uef_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(uef_core PUBLIC c_std_11)
set_target_properties(uef_core PROPERTIES
  C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
  EXPORT_NAME Core)
target_compile_definitions(uef_core PUBLIC ${UEF_TARGET_DEFINITIONS})

if(UEF_TARGET STREQUAL "CORTEX_M")
  target_include_directories(uef_core PUBLIC ${UEF_CMSIS_INCLUDE_DIR})
endif()
if(UEF_UOS_BACKEND STREQUAL "FREERTOS")
  target_include_directories(uef_core PUBLIC ${UEF_FREERTOS_INCLUDE_DIR})
  if(UEF_FREERTOS_TARGET)
    target_link_libraries(uef_core PUBLIC ${UEF_FREERTOS_TARGET})
  endif()
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
