# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find SPIR-V Tools

# The following definitions are added on success
# 
#  SPIRVTools_FOUND - SPIR-V Tools was found
#  SPIRVTools_SPIRV_VAL - spirv-val executable
#  SPIRVTools_VERSION - SPIR-V Tools version
#
# The followings hints may be passed in the environment:
#
# RE2C_ROOT
#

if(SPIRVTools_SPIRV_VAL)
    set(SPIRVTools_FOUND TRUE)
else()
    find_program(SPIRVTools_SPIRV_VAL NAMES spirv-val
        HINTS
        ENV SPIRVTools_ROOT
        ENV PATH
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SPIRVTools DEFAULT_MSG SPIRVTools_SPIRV_VAL)

    execute_process(COMMAND ${SPIRVTools_SPIRV_VAL} --version
        OUTPUT_VARIABLE SPIRVTools_version_output
        ERROR_VARIABLE SPIRVTools_version_error
        RESULT_VARIABLE SPIRVTools_version_result
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT ${SPIRVTools_version_result} EQUAL 0)
        set(SPIRVTools_message "Command \"{SPIRVTools_SPIRV_VAL} --version\" failed:\n${SPIRVTools_version_output}\n${SPIRVTools_version_error}")
        if(SPIRVTools_FIND_REQUIRED)
            message(SEND_ERROR ${SPIRVTools_message})
        else()
            message(${SPIRVTools_message})
        endif()
    else()
        string(REGEX REPLACE "SPIRV-Tools ([v0-9\.]+) .*" "\\1" SPIRVTools_VERSION "${SPIRVTools_version_output}")
    endif()

    mark_as_advanced(SPIRVTools_SPIRV_VAL)
endif()
