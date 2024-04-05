# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

find_package(SYCL REQUIRED)

add_executable(tinytc-bench main.cpp args.cpp)
add_sycl_to_target(TARGET tinytc-bench SOURCES main.cpp)
target_link_libraries(tinytc-bench PRIVATE tinytc tinytc-sycl)
set_common_options(tinytc-bench)
