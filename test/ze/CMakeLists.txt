# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(CommonOptions)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

find_package(LevelZero REQUIRED)

add_library(test-ze-lib STATIC test_runtime.cpp)
target_link_libraries(test-ze-lib PUBLIC test-lib tinytc-level-zero LevelZero::LevelZero)
set_common_options(test-ze-lib)

add_executable(test-ze-device device.cpp)
target_link_libraries(test-ze-device PRIVATE test-ze-lib)
doctest_discover_tests(test-ze-device)
set_common_options(test-ze-device)

add_executable(test-ze-smm smm.cpp)
target_link_libraries(test-ze-smm PRIVATE test-ze-lib)
doctest_discover_tests(test-ze-smm)
set_common_options(test-ze-smm)

