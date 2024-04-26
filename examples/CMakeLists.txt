# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(builder)
add_subdirectory(jit)

if(BUILD_LEVEL_ZERO)
    add_subdirectory(simple_ze)
endif()

if(BUILD_OPENCL)
    add_subdirectory(simple_cl)
endif()

if(BUILD_SYCL)
    add_subdirectory(benchmark)
    add_subdirectory(matrix_chain)
    add_subdirectory(tall_and_skinny)
endif()
