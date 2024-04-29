# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

function(load_dependencies type)
    find_dependency(tinytc REQUIRED)

    set(TINYTC_ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
    list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR})
    find_dependency(OpenCL REQUIRED)
    set(CMAKE_MODULE_PATH ${TINYTC_ORIGINAL_CMAKE_MODULE_PATH})
endfunction()

@SHARED_STATIC_TEMPLATE@
