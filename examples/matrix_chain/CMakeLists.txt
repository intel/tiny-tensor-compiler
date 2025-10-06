# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

find_package(SYCL REQUIRED)

set(SOURCES
    main.cpp
    test_ader.cpp
    test_volume.cpp
    test_multi.cpp
)

add_executable(matrix_chain ${SOURCES})
add_sycl_to_target(TARGET matrix_chain SOURCES ${SOURCES})
target_link_libraries(matrix_chain PRIVATE tinytc tinytc_sycl argparser)
set_cxx_common_options(matrix_chain)
