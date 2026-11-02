find_package(Threads REQUIRED)
add_executable(uetd_uhal_neutral_headers neutral_headers.c)
target_include_directories(uetd_uhal_neutral_headers PRIVATE ../../include)
target_compile_features(uetd_uhal_neutral_headers PRIVATE c_std_11)
target_compile_definitions(uetd_uhal_neutral_headers PRIVATE
  UETD_TARGET_CORTEX_M=1 UETD_CMSIS_DEVICE_HEADER="intentionally_absent_cmsis.h")
add_test(NAME uetd.uhal_neutral_headers COMMAND uetd_uhal_neutral_headers)
add_executable(uetd_uhal_host_contracts host_contracts.c)
target_link_libraries(uetd_uhal_host_contracts PRIVATE UETD::Core Threads::Threads)
add_test(NAME uetd.uhal_host COMMAND uetd_uhal_host_contracts)

# Register-model tests exercise the actual CPU sources. They validate contracts,
# not a target toolchain, electrical behavior or a real CMSIS implementation.
foreach(_core IN ITEMS 0 23 4)
  add_executable(uetd_uhal_cortex_m${_core}
    cortex_contracts.c
    ../../src/uhal/arch/cortex_m/atomic.c
    ../../src/uhal/arch/cortex_m/core.c
    ../../src/uhal/arch/cortex_m/irq.c
    ../../src/uhal/arch/cortex_m/cache.c)
  target_include_directories(uetd_uhal_cortex_m${_core} PRIVATE ../../include "${CMAKE_CURRENT_LIST_DIR}")
  target_compile_features(uetd_uhal_cortex_m${_core} PRIVATE c_std_11)
  target_compile_definitions(uetd_uhal_cortex_m${_core} PRIVATE
    UETD_TARGET_CORTEX_M=1 UETD_CMSIS_DEVICE_HEADER="mock_cmsis.h"
    MOCK_CORE=${_core} UHAL_IRQ_COUNT=32 NDEBUG)
  if(_core EQUAL 4)
    target_compile_definitions(uetd_uhal_cortex_m${_core} PRIVATE
      UHAL_HAS_IRQ_PRIORITY_MASK=1 UHAL_HAS_DWT=1 UHAL_HAS_DCACHE=1
      UHAL_HAS_ICACHE=1 UHAL_CACHE_LINE_SIZE=32)
  endif()
  add_test(NAME uetd.uhal_cortex_m${_core} COMMAND uetd_uhal_cortex_m${_core})
endforeach()

# I-cache-only devices must not inherit D-cache requirements.
add_executable(uetd_uhal_icache_only icache_only.c ../../src/uhal/arch/cortex_m/cache.c)
target_include_directories(uetd_uhal_icache_only PRIVATE ../../include "${CMAKE_CURRENT_LIST_DIR}")
target_compile_features(uetd_uhal_icache_only PRIVATE c_std_11)
target_compile_definitions(uetd_uhal_icache_only PRIVATE
  UETD_TARGET_CORTEX_M=1 UETD_CMSIS_DEVICE_HEADER="mock_cmsis.h" MOCK_CORE=4 UHAL_HAS_ICACHE=1)
add_test(NAME uetd.uhal_icache_only COMMAND uetd_uhal_icache_only)
