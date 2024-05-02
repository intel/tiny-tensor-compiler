# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)
include(InstallLib)

find_package(LevelZero REQUIRED)
find_package(ocloc REQUIRED)

set(SOURCES
    device_info.cpp
    error.cpp
    kernel.cpp
    opencl_cc.cpp
    recipe_handler.cpp
)
set(PUBLIC_HEADERS
    tinytc_ze.h
    tinytc_ze.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc_ze ${SOURCES})
add_library(tinytc::tinytc_ze ALIAS tinytc_ze)

set_cxx_common_options(tinytc_ze)
target_link_libraries(tinytc_ze PRIVATE ocloc::ocloc)
target_link_libraries(tinytc_ze PUBLIC tinytc LevelZero::LevelZero)

# include directories

target_sources(tinytc_ze PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc_ze tinytc)

install(FILES
    "${PROJECT_SOURCE_DIR}/cmake/FindLevelZero.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tinytc_ze
)
