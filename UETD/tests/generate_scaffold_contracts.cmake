function(uetd_add_scaffold_contract_test _name _expected_status)
  set(_options)
  set(_one_value_args)
  set(_multi_value_args SOURCES)
  cmake_parse_arguments(UT "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

  if(NOT UT_SOURCES)
    message(FATAL_ERROR "${_name}: no scaffold source files were selected")
  endif()
  list(LENGTH UT_SOURCES _source_count)

  set(_headers)
  set(_test_body "")
  set(_operation_count 0)
  foreach(_source IN LISTS UT_SOURCES)
    file(STRINGS "${_source}" _include_lines REGEX "^[ \t]*#include[ \t]*[<\"]")
    foreach(_include_line IN LISTS _include_lines)
      string(REGEX MATCH "#include[ \t]*[<\"]([^>\"]+)[>\"]" _include_match "${_include_line}")
      if(_include_match)
        list(APPEND _headers "${CMAKE_MATCH_1}")
      endif()
    endforeach()

    file(STRINGS "${_source}" _source_lines)
    list(LENGTH _source_lines _line_count)
    set(_line_index 0)
    while(_line_index LESS _line_count)
      list(GET _source_lines ${_line_index} _line)
      string(STRIP "${_line}" _line)
      if(_line MATCHES "^([A-Za-z_][A-Za-z0-9_]*)[ ]+([A-Za-z_][A-Za-z0-9_]*)[ ]*\\(")
        set(_signature "${_line}")
        math(EXPR _line_index "${_line_index} + 1")
        while(NOT _signature MATCHES "\\)[ ]*\\{[ ]*$" AND _line_index LESS _line_count)
          list(GET _source_lines ${_line_index} _continuation)
          string(STRIP "${_continuation}" _continuation)
          string(APPEND _signature " ${_continuation}")
          math(EXPR _line_index "${_line_index} + 1")
        endwhile()

        if(NOT _signature MATCHES "^([A-Za-z_][A-Za-z0-9_]*)[ ]+([A-Za-z_][A-Za-z0-9_]*)[ ]*\\((.*)\\)[ ]*\\{")
          message(FATAL_ERROR "${_name}: could not parse function definition in ${_source}: ${_signature}")
        endif()
        set(_return_type "${CMAKE_MATCH_1}")
        set(_function "${CMAKE_MATCH_2}")
        set(_parameters "${CMAKE_MATCH_3}")
        set(_arguments)
        if(NOT _parameters STREQUAL "void" AND NOT _parameters STREQUAL "")
          string(REPLACE "," ";" _parameter_list "${_parameters}")
          foreach(_parameter IN LISTS _parameter_list)
            string(STRIP "${_parameter}" _parameter)
            if(_parameter MATCHES "\\*" OR _parameter MATCHES "\\[[^]]*\\]")
              list(APPEND _arguments "((void*)workspace.bytes)")
            else()
              string(REGEX REPLACE "[ ]+[A-Za-z_][A-Za-z0-9_]*$" "" _parameter_type "${_parameter}")
              if(_parameter_type STREQUAL "${_parameter}" OR _parameter_type STREQUAL "")
                message(FATAL_ERROR "${_name}: could not parse parameter type in ${_source}: ${_parameter}")
              endif()
              list(APPEND _arguments "(${_parameter_type}){0}")
            endif()
          endforeach()
        endif()
        list(JOIN _arguments ", " _argument_text)

        if(_return_type STREQUAL "void")
          string(APPEND _test_body
            "    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));\n"
            "    memcpy(before, workspace.bytes, sizeof(before));\n"
            "    ${_function}(${_argument_text});\n")
        else()
          if(_return_type STREQUAL "bool")
            set(_neutral_value "false")
          elseif(_return_type STREQUAL "uetd_status_t" OR _return_type STREQUAL "umath_status_t")
            set(_neutral_value "${_expected_status}")
          else()
            set(_neutral_value "0")
          endif()
          string(APPEND _test_body
            "    memset(workspace.bytes, 0xA5, sizeof(workspace.bytes));\n"
            "    memcpy(before, workspace.bytes, sizeof(before));\n"
            "    if (${_function}(${_argument_text}) != ${_neutral_value}) {\n"
            "        printf(\"FAIL ${_function}: unexpected fail-closed return\\n\");\n"
            "        ++failures;\n"
            "    }\n")
        endif()
        string(APPEND _test_body
          "    if (memcmp(workspace.bytes, before, sizeof(before)) != 0) {\n"
          "        printf(\"FAIL ${_function}: changed caller-owned storage\\n\");\n"
          "        ++failures;\n"
          "    }\n")
        math(EXPR _operation_count "${_operation_count} + 1")
      else()
        math(EXPR _line_index "${_line_index} + 1")
      endif()
    endwhile()
  endforeach()

  if(NOT _operation_count)
    message(FATAL_ERROR "${_name}: no scaffold operations were discovered")
  endif()
  list(REMOVE_DUPLICATES _headers)

  set(_test_source "${CMAKE_CURRENT_BINARY_DIR}/${_name}.c")
  file(WRITE "${_test_source}" "#include <stdbool.h>\n#include <stddef.h>\n#include <stdio.h>\n#include <string.h>\n")
  foreach(_header IN LISTS _headers)
    file(APPEND "${_test_source}" "#include <${_header}>\n")
  endforeach()
  file(APPEND "${_test_source}"
    "\nstatic union { long double align; void* pointer_align; unsigned char bytes[512]; } workspace;\n"
    "static unsigned char before[sizeof(workspace.bytes)];\n\n"
    "int main(void) {\n"
    "    size_t failures = 0u;\n"
    "${_test_body}"
    "    if (failures != 0u) {\n"
    "        printf(\"%zu ${_name} check(s) failed across ${_operation_count} operations\\n\", failures);\n"
    "        return 1;\n"
    "    }\n"
    "    printf(\"Validated ${_operation_count} ${_name} operations from ${_source_count} source files\\n\");\n"
    "    return 0;\n"
    "}\n")

  add_executable(${_name} ${UT_SOURCES} "${_test_source}")
  target_include_directories(${_name} PRIVATE "${PROJECT_SOURCE_DIR}/include")
  target_compile_features(${_name} PRIVATE c_std_11)
  if(UETD_UMATH_DEFINITIONS)
    target_compile_definitions(${_name} PRIVATE ${UETD_UMATH_DEFINITIONS})
  endif()
  string(REGEX REPLACE "^uetd_" "" _test_label "${_name}")
  add_test(NAME "uetd.${_test_label}" COMMAND ${_name})
  message(STATUS "${_name}: discovered ${_operation_count} scaffold operations in ${_source_count} source files")
endfunction()
