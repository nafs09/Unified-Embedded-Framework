#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "UETD::Core" for configuration "Release"
set_property(TARGET UETD::Core APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(UETD::Core PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/uetd_core.lib"
  )

list(APPEND _cmake_import_check_targets UETD::Core )
list(APPEND _cmake_import_check_files_for_UETD::Core "${_IMPORT_PREFIX}/lib/uetd_core.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
