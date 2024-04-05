# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(InstallLib)

find_package(SYCL REQUIRED)

set(SOURCES
    device_info.cpp
    kernel.cpp
    runtime.cpp
)
set(PUBLIC_HEADERS
    sycl/device_info.hpp
    sycl/kernel.hpp
    sycl/runtime.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc-sycl ${SOURCES})
add_library(tinytc::tinytc-sycl ALIAS tinytc-sycl)
add_sycl_to_target(TARGET tinytc-sycl SOURCES ${SOURCES})

set_common_options(tinytc-sycl)
target_link_libraries(tinytc-sycl PRIVATE tinytc tinytc-level-zero tinytc-opencl)

# include directories

target_sources(tinytc-sycl PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc-sycl tinytc)
