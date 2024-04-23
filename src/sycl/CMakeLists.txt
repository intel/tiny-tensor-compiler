# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(InstallLib)

find_package(SYCL REQUIRED)

set(SOURCES
    device_info.cpp
    error.cpp
    recipe_handler.cpp
)
set(PUBLIC_HEADERS
    tinytc_sycl.h
    tinytc_sycl.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc_sycl ${SOURCES})
add_library(tinytc::tinytc_sycl ALIAS tinytc_sycl)
add_sycl_to_target(TARGET tinytc_sycl SOURCES ${SOURCES})

set_cxx_common_options(tinytc_sycl)
target_link_libraries(tinytc_sycl PRIVATE tinytc)
target_link_libraries(tinytc_sycl PUBLIC tinytc_ze tinytc_cl)

# include directories

target_sources(tinytc_sycl PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc_sycl tinytc)
