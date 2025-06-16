# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

function(get_generated_files VAR target)
    get_target_property(_sources ${target} SOURCES)
    set(_generated_files "")
    foreach(_src ${_sources})
        get_source_file_property(_generated "${_src}" GENERATED)
        if(${_generated} GREATER 0)
            list(APPEND _generated_files ${_src})
        endif()
    endforeach()
    set(${VAR} ${_generated_files} PARENT_SCOPE)
endfunction()

function(add_re2c_or_pregenerated_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET;IN;OUT;FLAGS" "")

    if(EXISTS ${ARG_OUT})
        message(STATUS "Pre-generated ${ARG_OUT} available -- skipping re2c dependency")
        target_sources(${ARG_TARGET} PRIVATE ${ARG_OUT})
    else()
        find_package(re2c REQUIRED)
        add_re2c_to_target(TARGET ${ARG_TARGET} SOURCES ${ARG_IN} FLAGS "${ARG_FLAGS}")
    endif()
endfunction()

function(add_bison_or_pregenerated_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET;IN;OUT" "")

    get_filename_component(file_name ${ARG_OUT} NAME)
    string(REGEX REPLACE "[.]cpp$" ".hpp" out_header ${ARG_OUT})
    string(REGEX REPLACE "${file_name}$" "location.hh" location_header ${ARG_OUT})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_OUT} AND
        EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${out_header} AND
        EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${location_header})
        message(STATUS "Pre-generated ${ARG_OUT}/${out_header}/${location_header} available -- skipping bison dependency")
        target_sources(${ARG_TARGET} PRIVATE ${ARG_OUT})
        add_flag_if_available_to_source_files(CXX "${ARG_OUT}" "-Wno-unused-but-set-variable")
        set(BISON_parser_OUTPUTS ${ARG_OUT})
    else()
        find_package(BISON 3.8.2 REQUIRED)
        BISON_TARGET(parser ${ARG_IN} ${CMAKE_CURRENT_BINARY_DIR}/${ARG_OUT}
            DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/${out_header})
        target_sources(${ARG_TARGET} PRIVATE ${BISON_parser_OUTPUTS})
    endif()
    add_flag_if_available_to_source_files(CXX "${BISON_parser_OUTPUTS}" "-Wno-unused-but-set-variable")
endfunction()

