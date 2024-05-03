# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)
include(InstallLib)

find_package(OpenCL REQUIRED)
find_package(re2c REQUIRED)

set(SOURCES
    device_info.cpp
    error.cpp
    kernel.cpp
    recipe_handler.cpp
)
set(RE2C_SOURCES
    device_info_helper.re
)
set(PUBLIC_HEADERS
    tinytc_cl.h
    tinytc_cl.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_library(tinytc_cl-objects OBJECT ${SOURCES})
add_re2c_to_target(TARGET tinytc_cl-objects SOURCES ${RE2C_SOURCES} FLAGS "--tags")
set_cxx_common_options(tinytc_cl-objects)
target_link_libraries(tinytc_cl-objects PUBLIC tinytc OpenCL::OpenCL)
target_include_directories(tinytc_cl-objects PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
)

add_library(tinytc_cl $<TARGET_OBJECTS:tinytc_cl-objects>)
add_library(tinytc::tinytc_cl ALIAS tinytc_cl)

set_cxx_common_options(tinytc_cl)
target_link_libraries(tinytc_cl PUBLIC tinytc OpenCL::OpenCL)

# include directories

target_sources(tinytc_cl PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
    FILES ${PUBLIC_HEADERS})


# install

install_lib(tinytc_cl tinytc)

install(FILES
    "${PROJECT_SOURCE_DIR}/cmake/FindOpenCL.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tinytc_cl
)
