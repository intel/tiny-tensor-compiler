# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

function(load_dependencies type)
    find_dependency(tinytc REQUIRED)
    find_dependency(tinytc-level-zero REQUIRED)
    find_dependency(tinytc-opencl REQUIRED)
endfunction()

@SHARED_STATIC_TEMPLATE@
