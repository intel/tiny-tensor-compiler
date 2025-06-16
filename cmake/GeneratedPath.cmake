# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

function(get_path_of_generated_file)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "SOURCE;EXT" "")

    set(INPUT_FILE ${ARG_SOURCE})
    if(NOT IS_ABSOLUTE "${INPUT_FILE}")
        set(INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE})
    endif()
    file(RELATIVE_PATH INPUT_REL_PATH ${PROJECT_SOURCE_DIR} ${INPUT_FILE})
    get_filename_component(INPUT_REL_PATH ${INPUT_REL_PATH} DIRECTORY)
    get_filename_component(INPUT_NAME_WLE ${INPUT_FILE} NAME_WLE)
    set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/${INPUT_REL_PATH})
    file(MAKE_DIRECTORY ${OUTPUT_DIR})
    if(ARG_EXT) 
        set(OUTPUT_NAME "${INPUT_NAME_WLE}.${ARG_EXT}")
    else()
        set(OUTPUT_NAME ${INPUT_NAME_WLE})
    endif()
    set(INPUT_PATH "${INPUT_FILE}" PARENT_SCOPE)
    set(OUTPUT_REL_PATH "${INPUT_REL_PATH}/${OUTPUT_NAME}" PARENT_SCOPE)
    set(OUTPUT_PATH "${OUTPUT_DIR}/${OUTPUT_NAME}" PARENT_SCOPE)
endfunction()
