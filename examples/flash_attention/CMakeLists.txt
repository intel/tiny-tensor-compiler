# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(EmbedSource)

find_package(SYCL REQUIRED)

add_executable(flash_attention main.cpp)
add_sycl_to_target(TARGET flash_attention SOURCES main.cpp)
target_link_libraries(flash_attention PRIVATE tinytc tinytc_sycl argparser)
embed_source(flash_attention flash_attention.template)
set_cxx_common_options(flash_attention)
