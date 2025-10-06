# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

add_library(argparser STATIC argparser.cpp)
target_include_directories(argparser PRIVATE ${PROJECT_SOURCE_DIR}/utilities)
target_include_directories(argparser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_cxx_common_options(argparser)

add_library(argparser_common STATIC argparser_common.cpp)
target_link_libraries(argparser_common PRIVATE argparser tinytc)
target_include_directories(argparser_common PRIVATE ${PROJECT_SOURCE_DIR}/utilities)
set_cxx_common_options(argparser_common)

add_executable(test-argparser test.cpp)
target_link_libraries(test-argparser PRIVATE argparser)
set_cxx_common_options(test-argparser)
