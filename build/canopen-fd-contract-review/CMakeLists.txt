cmake_minimum_required(VERSION 3.20)
project(UETDCANContracts LANGUAGES C)
enable_testing()
set(PROJECT_SOURCE_DIR "C:/NEXUS/Unified Embedded Framework/UETD")
include("${PROJECT_SOURCE_DIR}/tests/generate_scaffold_contracts.cmake")
uetd_add_scaffold_contract_test(uetd_medi_can_scaffold_contracts UETD_NOT_SUPPORTED
  SOURCES "${PROJECT_SOURCE_DIR}/src/uproto/future/medi_can/medi_can.c")
uetd_add_scaffold_contract_test(uetd_canopen_fd_scaffold_contracts UETD_NOT_SUPPORTED
  SOURCES "${PROJECT_SOURCE_DIR}/src/uproto/future/industrial/canopen_fd.c")
uetd_add_scaffold_contract_test(uetd_canopen_cc_scaffold_contracts UETD_NOT_SUPPORTED
  SOURCES "${PROJECT_SOURCE_DIR}/src/uproto/future/industrial/canopen.c")
add_executable(uetd_fdcan_frame_contracts
  "${PROJECT_SOURCE_DIR}/tests/upal_fdcan_fd_scaffolds.c"
  "${PROJECT_SOURCE_DIR}/src/upal/fdcan_fd.c")
target_include_directories(uetd_fdcan_frame_contracts PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_compile_features(uetd_fdcan_frame_contracts PRIVATE c_std_11)
add_test(NAME uetd.fdcan_frame_contracts COMMAND uetd_fdcan_frame_contracts)
