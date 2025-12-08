# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(GitVersion)

function(cpack_setup)
    git_version()
    set(CPACK_SOURCE_GENERATOR "TXZ")
    set(CPACK_SOURCE_PACKAGE_FILE_NAME
        "${CMAKE_PROJECT_NAME}-${GIT_MAJOR_VERSION}.${GIT_MINOR_VERSION}.${GIT_PATCH_VERSION}"
    )
    if(NOT "${GIT_COMMITS_SINCE_RELEASE}" STREQUAL "0")
        set(CPACK_SOURCE_PACKAGE_FILE_NAME
            "${CPACK_SOURCE_PACKAGE_FILE_NAME}-${GIT_COMMITS_SINCE_RELEASE}-${GIT_COMMIT}"
        )
    endif()
    set(CPACK_SOURCE_IGNORE_FILES
        "/\\\\.git/"
        "/\\\\.gitignore$"
        "\\\\.swp$"
        "/build/"
        "/build_debug/"
        "/build_iwyu/"
        "/coverity_scan/"
        "/__pycache__/"
    )

    set(GENERATED_FILE_LISTS_DIR "${PROJECT_BINARY_DIR}/generated_file_lists")

    configure_file("${PROJECT_SOURCE_DIR}/cmake/CPackGeneratedFiles.cmake.in" "CPackGeneratedFiles.cmake" @ONLY)
    set(CPACK_INSTALL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CPackGeneratedFiles.cmake")
    include(CPack)
    add_custom_target(dist
        COMMAND "${CMAKE_COMMAND}" --build "${PROJECT_BINARY_DIR}" --target package_source
        DEPENDS tinytc-objects
        VERBATIM
        USES_TERMINAL
    )
    if(BUILD_OPENCL)
        add_dependencies(dist tinytc_cl-objects)
    endif()
    if(BUILD_MOCHI)
        add_dependencies(dist mochi)
    endif()
endfunction()

