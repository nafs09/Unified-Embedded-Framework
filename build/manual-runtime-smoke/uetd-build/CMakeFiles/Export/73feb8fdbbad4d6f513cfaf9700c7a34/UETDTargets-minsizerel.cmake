#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "UETD::Core" for configuration "MinSizeRel"
set_property(TARGET UETD::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(UETD::Core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "C"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/uetd_core.lib"
  )

list(APPEND _cmake_import_check_targets UETD::Core )
list(APPEND _cmake_import_check_files_for_UETD::Core "${_IMPORT_PREFIX}/lib/uetd_core.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
