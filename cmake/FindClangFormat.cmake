# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find clang-format

# The following definitions are added on success
# 
#  ClangFormat_FOUND - ClangFormat was found
#  ClangFormat_EXECUTABLE - ClangFormat executable
#  ClangFormat_VERSION - ClangFormat version
#
# The followings hints may be passed in the environment:
#
# CLANGFORMAT_ROOT
#

if(ClangFormat_EXECUTABLE)
    set(ClangFormat_FOUND TRUE)
else()
    find_program(ClangFormat_EXECUTABLE NAMES clang-format
        HINTS
        ENV CLANGFORMAT_ROOT
        ENV PATH
    )

    execute_process(COMMAND ${ClangFormat_EXECUTABLE} --version
        OUTPUT_VARIABLE ClangFormat_version_output
        ERROR_VARIABLE ClangFormat_version_error
        RESULT_VARIABLE ClangFormat_version_result
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    macro(ClangFormat_version_failed)
        set(ClangFormat_message "Command \"${ClangFormat_EXECUTABLE} --version\" failed:\n${ClangFormat_version_output}\n${ClangFormat_version_error}")
        if(ClangFormat_FIND_REQUIRED)
            message(SEND_ERROR ${ClangFormat_message})
        else()
            message(${ClangFormat_message})
        endif()
    endmacro()

    if(NOT ${ClangFormat_version_result} EQUAL 0)
        ClangFormat_version_failed()
    else()
        if(${ClangFormat_version_output} MATCHES "[a-zA-Z-_ ]*([0-9\.]+)")
            set(ClangFormat_VERSION ${CMAKE_MATCH_1})
        else()
            ClangFormat_version_failed()
        endif()
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(ClangFormat DEFAULT_MSG
                                      ClangFormat_EXECUTABLE ClangFormat_VERSION)

    mark_as_advanced(ClangFormat_EXECUTABLE ClangFormat_VERSION)
endif()
