# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

find_package(SYCL REQUIRED)

add_library(test-sycl-lib STATIC test_runtime.cpp)
target_link_libraries(test-sycl-lib PUBLIC test-lib tinytc-sycl SYCL::SYCL)
set_common_options(test-sycl-lib)

add_executable(test-sycl-smm smm.cpp)
target_link_libraries(test-sycl-smm PRIVATE test-sycl-lib)
doctest_discover_tests(test-sycl-smm)
set_common_options(test-sycl-smm)
