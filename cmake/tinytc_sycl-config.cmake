# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

function(load_dependencies type)
    find_dependency(tinytc REQUIRED)
    find_dependency(tinytc_ze REQUIRED)
    find_dependency(tinytc_cl REQUIRED)
endfunction()

@SHARED_STATIC_TEMPLATE@
