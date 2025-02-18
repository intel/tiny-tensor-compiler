# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

function(load_dependencies type)
    set(is_shared OFF)
    if ("${type}" STREQUAL "shared")
        set(is_shared ON)
    endif ()
endfunction()

@SHARED_STATIC_TEMPLATE@
