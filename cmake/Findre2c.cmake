# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find re2c

# The following definitions are added on success
# 
#  re2c_FOUND - re2c was found
#  re2c_EXECUTABLE - re2c executable
#  re2c_VERSION - re2c version
#
# The followings hints may be passed in the environment:
#
# RE2C_ROOT
#

if(re2c_EXECUTABLE)
    set(re2c_FOUND TRUE)
else()
    find_program(re2c_EXECUTABLE NAMES re2c
        HINTS
        ENV RE2C_ROOT
        ENV PATH
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(re2c DEFAULT_MSG re2c_EXECUTABLE)

    execute_process(COMMAND ${re2c_EXECUTABLE} --version
        OUTPUT_VARIABLE re2c_version_output
        ERROR_VARIABLE re2c_version_error
        RESULT_VARIABLE re2c_version_result
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT ${re2c_version_result} EQUAL 0)
        set(re2c_message "Command \"{re2c_EXECUTABLE} --version\" failed:\n${re2c_version_output}\n${re2c_version_error}")
        if(re2c_FIND_REQUIRED)
            message(SEND_ERROR ${re2c_message})
        else()
            message(${re2c_message})
        endif()
    else()
        string(REGEX REPLACE "re2c ([0-9\.]+)" "\\1" re2c_VERSION "${re2c_version_output}")
    endif()

    mark_as_advanced(re2c_EXECUTABLE)
endif()

if(re2c_EXECUTABLE)
    function(add_re2c_to_target)
        cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET;FLAGS" "SOURCES")
        foreach(SOURCE IN LISTS ARG_SOURCES)
            set(INPUT_FILE ${SOURCE})
            if(NOT IS_ABSOLUTE "${INPUT_FILE}")
                set(INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE})
            endif()
            file(RELATIVE_PATH INPUT_REL_PATH ${PROJECT_SOURCE_DIR} ${INPUT_FILE})
            get_filename_component(INPUT_REL_PATH ${INPUT_REL_PATH} DIRECTORY)
            get_filename_component(INPUT_NAME ${INPUT_FILE} NAME)
            string(REGEX REPLACE "[.]re$" ".cpp" OUTPUT_NAME ${INPUT_NAME})
            set(OUTPUT_PATH ${PROJECT_BINARY_DIR}/${INPUT_REL_PATH})
            file(MAKE_DIRECTORY ${OUTPUT_PATH})
            set(OUTPUT_FILE ${OUTPUT_PATH}/${OUTPUT_NAME})
            add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                DEPENDS ${SOURCE}
                COMMAND ${re2c_EXECUTABLE} ${ARG_FLAGS} -o ${OUTPUT_FILE} ${INPUT_FILE}
                COMMENT "Generating lexer ${OUTPUT_FILE}"
            )
            target_sources(${ARG_TARGET} PRIVATE ${OUTPUT_FILE})
        endforeach()
    endfunction()
endif()
