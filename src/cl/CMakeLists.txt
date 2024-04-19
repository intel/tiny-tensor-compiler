# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(InstallLib)

find_package(OpenCL REQUIRED)

set(SOURCES
    device_info.cpp
    error.cpp
    kernel.cpp
)
set(PUBLIC_HEADERS
    tinytc_cl.h
    tinytc_cl.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc_cl ${SOURCES})
add_library(tinytc::tinytc_cl ALIAS tinytc_cl)

set_cxx_common_options(tinytc_cl)
target_link_libraries(tinytc_cl PRIVATE tinytc)
target_link_libraries(tinytc_cl PUBLIC OpenCL::OpenCL)

# include directories

target_sources(tinytc_cl PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc_cl tinytc)
