# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GeneratedFiles)
include(GNUInstallDirs)

set(SOURCES main.cpp
    codegen.cpp
    inst.cpp
    objects.cpp
    omochi.cpp
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

add_re2c_or_pregenerated_to_target(TARGET mochi IN lexer.re OUT lexer.cpp)
add_re2c_or_pregenerated_to_target(TARGET mochi
    IN omochi_lexer.re
    OUT omochi_lexer.cpp
    FLAGS "--input-encoding;utf8")
add_bison_or_pregenerated_to_target(TARGET mochi IN parser_impl.yy OUT parser_impl.cpp)
