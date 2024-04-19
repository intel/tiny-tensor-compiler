# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

find_package(SYCL REQUIRED)

add_executable(tall_and_skinny main.cpp args.cpp)
add_sycl_to_target(TARGET tall_and_skinny SOURCES main.cpp)
target_link_libraries(tall_and_skinny PRIVATE tinytc tinytc_sycl)
set_cxx_common_options(tall_and_skinny)
