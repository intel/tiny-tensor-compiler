# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)

set(SOURCES main.cpp
    codegen.cpp
    inst.cpp
    objects.cpp
    parser.cpp
)

add_executable(mochi ${SOURCES})
target_link_libraries(mochi PRIVATE argparser)
set_cxx_common_options(mochi)
target_include_directories(mochi PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/utilities>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lexer.cpp)
    message(STATUS "Pre-generated lexer available -- skipping re2c dependency")
    target_sources(mochi PRIVATE lexer.cpp)
else()
    find_package(re2c REQUIRED)
    add_re2c_to_target(TARGET mochi SOURCES lexer.re)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/parser_impl.cpp)
    message(STATUS "Pre-generated parser available -- skipping bison dependency")
    target_sources(mochi PRIVATE parser_impl.cpp)
else()
    find_package(BISON 3.8.2 REQUIRED)
    BISON_TARGET(mochi_parser parser_impl.yy ${CMAKE_CURRENT_BINARY_DIR}/parser_impl.cpp
                 DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser_impl.hpp)
    target_sources(mochi PRIVATE ${BISON_mochi_parser_OUTPUTS})
endif()

