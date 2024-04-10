# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(InstallLib)

find_package(LevelZero REQUIRED)

set(SOURCES
    device_info.cpp
    error.cpp
    kernel.cpp
    runtime.cpp
)
set(PUBLIC_HEADERS
    ze/device_info.hpp
    ze/error.hpp
    ze/kernel.hpp
    ze/runtime.hpp
    ze/shared_handle.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc-level-zero ${SOURCES})
add_library(tinytc::tinytc-level-zero ALIAS tinytc-level-zero)

set_common_options(tinytc-level-zero)
target_link_libraries(tinytc-level-zero PRIVATE tinytc LevelZero::LevelZero)

# include directories

target_sources(tinytc-level-zero PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc-level-zero tinytc)