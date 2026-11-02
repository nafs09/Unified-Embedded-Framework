cmake_minimum_required(VERSION 3.20)
project(manual_uos_uapp C)
if(NOT UETD_ROOT)
  message(FATAL_ERROR "Set UETD_ROOT to the UETD source root")
endif()
# These values shape public structs, so configure the library before compiling any consumer.
set(UAPP_SM_MAX_STATES 1 CACHE STRING "Manual runtime states" FORCE)
set(UAPP_SM_MAX_TRANSITIONS 0 CACHE STRING "Manual runtime transitions" FORCE)
set(UAPP_SM_EVENT_QUEUE_CAPACITY 8 CACHE STRING "Manual event queue" FORCE)
set(UAPP_SM_EVENT_PAYLOAD_CAPACITY 16 CACHE STRING "Manual event payload" FORCE)
set(UETD_UOS_BACKEND FREERTOS CACHE STRING "Manual runtime backend" FORCE)
set(UETD_FREERTOS_CONFIG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Manual FreeRTOS configuration" FORCE)

if(TARGET UETD::Core)
  message(FATAL_ERROR "Configure this generated runtime before adding UETD; ABI settings must match")
endif()
add_subdirectory("${UETD_ROOT}" uetd-build)
add_library(manual_runtime STATIC
  minimal.c
  tasks.c
)
target_link_libraries(manual_runtime PUBLIC UETD::Core)
target_include_directories(manual_runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(manual_runtime PUBLIC c_std_11)
# Supply your application and callbacks. An external smoke/application source is optional.
if(MANUAL_APPLICATION_SOURCE)
  add_executable(manual_application "${MANUAL_APPLICATION_SOURCE}")
  target_link_libraries(manual_application PRIVATE manual_runtime)
endif()
