# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(CommonOptions)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

find_package(LevelZero REQUIRED)

add_library(test-ze-lib STATIC test_runtime.cpp)
target_link_libraries(test-ze-lib PUBLIC test-lib tinytc_ze LevelZero::LevelZero)
set_cxx_common_options(test-ze-lib)

add_executable(test-ze-device device.cpp)
target_link_libraries(test-ze-device PRIVATE test-ze-lib)
doctest_discover_tests(test-ze-device)
set_cxx_common_options(test-ze-device)

add_executable(test-ze-linalg linalg.cpp)
target_link_libraries(test-ze-linalg PRIVATE test-ze-lib test-lib-linalg)
doctest_discover_tests(test-ze-linalg)
set_cxx_common_options(test-ze-linalg)

