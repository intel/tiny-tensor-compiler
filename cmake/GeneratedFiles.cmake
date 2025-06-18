# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GeneratedPath)

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

function(write_generated_files target)
    get_generated_files(GENERATED_FILES ${target})
    file(WRITE "${PROJECT_BINARY_DIR}/generated_file_lists/${target}.list" "${GENERATED_FILES}")
endfunction()

function(add_re2c_or_pregenerated_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET;FLAGS" "SOURCES")

    foreach(SOURCE IN LISTS ARG_SOURCES)
        get_path_of_generated_file(SOURCE ${SOURCE} EXT "cpp")

        if(EXISTS ${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH})
            message(STATUS "Pre-generated ${OUTPUT_REL_PATH} available -- skipping re2c dependency")
            target_sources(${ARG_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH})
        else()
            find_package(re2c REQUIRED)
            add_re2c_to_target(TARGET ${ARG_TARGET} SOURCES ${SOURCE} FLAGS "${ARG_FLAGS}")
        endif()
    endforeach()
endfunction()

function(add_bison_or_pregenerated_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "HAVE_LOCATION" "TARGET" "SOURCES")

    foreach(SOURCE IN LISTS ARG_SOURCES)
        get_path_of_generated_file(SOURCE ${SOURCE} EXT "cpp")

        string(REGEX REPLACE "[.]cpp$" ".hpp" header_rel_path ${OUTPUT_REL_PATH})
        if(ARG_HAVE_LOCATION)
            get_filename_component(rel_path ${OUTPUT_REL_PATH} DIRECTORY)
            set(location_hh "${rel_path}/location.hh")
        endif()

        if(EXISTS ${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH} AND
                EXISTS ${PROJECT_SOURCE_DIR}/${header_rel_path} AND
                (NOT location_hh OR EXISTS ${PROJECT_SOURCE_DIR}/${location_hh}))
            message(STATUS "Pre-generated ${OUTPUT_REL_PATH},${header_rel_path} available -- skipping bison dependency")
            target_sources(${ARG_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH})
            set(BISON_parser_OUTPUTS "${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH}")
            if(location_hh)
                set(location_hh "${PROJECT_SOURCE_DIR}/${location_hh}")
            endif()
        else()
            find_package(BISON 3.8.2 REQUIRED)
            BISON_TARGET(parser ${INPUT_PATH} ${OUTPUT_PATH}
                         DEFINES_FILE ${PROJECT_BINARY_DIR}/${header_rel_path})
            if(location_hh)
                set(location_hh "${PROJECT_BINARY_DIR}/${location_hh}")
            endif()
        endif()
        target_sources(${ARG_TARGET} PRIVATE ${BISON_parser_OUTPUTS})
        if(location_hh)
            set_property(SOURCE "${location_hh}" PROPERTY GENERATED 1)
        target_sources(${ARG_TARGET} PRIVATE ${location_hh})
        endif()
        add_flag_if_available_to_source_files(CXX "${BISON_parser_OUTPUTS}" "-Wno-unused-but-set-variable")
    endforeach()
endfunction()

function(add_mochi_or_pregenerated_to_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET;FLAGS" "SOURCES;SEARCH_PATHS")

    set(search_paths "")
    foreach(search_path IN LISTS ARG_SEARCH_PATHS)
        list(APPEND search_paths "-I\"${search_path}\"")
    endforeach()

    foreach(SOURCE IN LISTS ARG_SOURCES)
        get_path_of_generated_file(SOURCE ${SOURCE})

        if(EXISTS ${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH})
            message(STATUS "Pre-generated ${OUTPUT_REL_PATH} available -- skipping mochi dependency")
            target_sources(${ARG_TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/${OUTPUT_REL_PATH})
        else()
            find_package(ClangFormat)
            if(ClangFormat_FOUND)
                add_custom_command(
                    OUTPUT ${OUTPUT_PATH}
                    DEPENDS mochi ${SOURCE}
                    COMMAND mochi ${ARG_FLAGS} -o ${OUTPUT_PATH} ${INPUT_PATH} ${search_paths}
                    COMMAND ${ClangFormat_EXECUTABLE} -i ${OUTPUT_PATH}
                    COMMENT "Generating code ${OUTPUT_REL_PATH}"
                )
            else()
                add_custom_command(
                    OUTPUT ${OUTPUT_PATH}
                    DEPENDS mochi ${SOURCE}
                    COMMAND mochi ${ARG_FLAGS} -o ${OUTPUT_PATH} ${INPUT_PATH} ${search_paths}
                    COMMENT "Generating code ${OUTPUT_REL_PATH}"
                )
            endif()
            target_sources(${ARG_TARGET} PRIVATE ${OUTPUT_PATH})
        endif()
    endforeach()
endfunction()
