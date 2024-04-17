# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(InstallLib)

find_package(SYCL REQUIRED)

set(PUBLIC_HEADERS
    tinytc_sycl.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc-sycl INTERFACE)
add_library(tinytc::tinytc-sycl ALIAS tinytc-sycl)

target_link_libraries(tinytc-sycl INTERFACE tinytc tinytc-level-zero tinytc-opencl)

# include directories

target_sources(tinytc-sycl PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc-sycl tinytc)
