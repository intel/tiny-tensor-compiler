# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(EmbedSource)

find_package(SYCL REQUIRED)

set(SOURCES
    main.cpp
    oz_int8.cpp
)

add_executable(ozaki ${SOURCES})
add_sycl_to_target(TARGET ozaki SOURCES ${SOURCES})
target_link_libraries(ozaki PRIVATE tinytc tinytc_sycl argparser)
set_cxx_common_options(ozaki)

embed_source(ozaki acc_f.tinytc)
embed_source(ozaki gemm.tinytc)
embed_source(ozaki split_int8.tinytc)
