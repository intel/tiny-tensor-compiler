# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

find_package(OpenCL REQUIRED)

add_executable(simple_cl main.c)
target_link_libraries(simple_cl PRIVATE tinytc tinytc_cl OpenCL::OpenCL)
set_c_common_options(simple_cl)
