# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

find_package(SYCL REQUIRED)

add_library(test-sycl-lib STATIC test_runtime.cpp)
target_link_libraries(test-sycl-lib PUBLIC test-lib tinytc_sycl SYCL::SYCL)
set_cxx_common_options(test-sycl-lib)

add_executable(test-sycl-linalg linalg.cpp)
target_link_libraries(test-sycl-linalg PRIVATE test-sycl-lib test-lib-linalg)
doctest_discover_tests(test-sycl-linalg)
set_cxx_common_options(test-sycl-linalg)
