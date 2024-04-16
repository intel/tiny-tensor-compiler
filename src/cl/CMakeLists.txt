# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(InstallLib)

find_package(OpenCL REQUIRED)

set(SOURCES
    device_info.cpp
    error.cpp
    kernel.cpp
    runtime.cpp
)
set(PUBLIC_HEADERS
    cl/device_info.hpp
    cl/error.hpp
    cl/kernel.hpp
    cl/object_wrapper.hpp
    cl/runtime.hpp
    tinytc_cl.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc-opencl ${SOURCES})
add_library(tinytc::tinytc-opencl ALIAS tinytc-opencl)

set_common_options(tinytc-opencl)
target_link_libraries(tinytc-opencl PRIVATE tinytc OpenCL::OpenCL)

# include directories

target_sources(tinytc-opencl PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc-opencl tinytc)
