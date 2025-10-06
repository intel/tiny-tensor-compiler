# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2023 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

function(git_version)
    find_package(Git)

    set(GIT_COMMITS_SINCE_RELEASE 0 PARENT_SCOPE)
    set(GIT_COMMIT "unknown" PARENT_SCOPE)

    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --long --match "v[0-9]*"
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
            RESULT_VARIABLE status_code
            OUTPUT_VARIABLE output
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(${output} MATCHES "v([0-9]*)\.([0-9]*)\.([0-9]*)-([0-9]*)-g([a-zA-Z0-9]*)")
            set(GIT_MAJOR_VERSION ${CMAKE_MATCH_1} PARENT_SCOPE)
            set(GIT_MINOR_VERSION ${CMAKE_MATCH_2} PARENT_SCOPE)
            set(GIT_PATCH_VERSION ${CMAKE_MATCH_3} PARENT_SCOPE)
            set(GIT_COMMITS_SINCE_RELEASE ${CMAKE_MATCH_4} PARENT_SCOPE)
            set(GIT_COMMIT "${CMAKE_MATCH_5}" PARENT_SCOPE)
        else()
            set(status_code 1)
        endif()
    endif()
    if(NOT status_code EQUAL 0)
        set(GIT_MAJOR_VERSION ${PROJECT_VERSION_MAJOR} PARENT_SCOPE)
        set(GIT_MINOR_VERSION ${PROJECT_VERSION_MINOR} PARENT_SCOPE)
        set(GIT_PATCH_VERSION ${PROJECT_VERSION_PATCH} PARENT_SCOPE)
    endif()
endfunction()
