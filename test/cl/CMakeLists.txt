# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(CommonOptions)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

find_package(OpenCL REQUIRED)

add_library(test-cl-lib STATIC test_runtime.cpp)
target_link_libraries(test-cl-lib PUBLIC test-lib tinytc_cl-objects OpenCL::OpenCL)
target_include_directories(test-cl-lib PUBLIC ${PROJECT_SOURCE_DIR}/src/cl)
set_cxx_common_options(test-cl-lib)

add_executable(test-cl-device device.cpp)
target_link_libraries(test-cl-device PRIVATE test-cl-lib)
doctest_discover_tests(test-cl-device)
set_cxx_common_options(test-cl-lib)

add_executable(test-cl-smm smm.cpp)
target_link_libraries(test-cl-smm PRIVATE test-cl-lib)
doctest_discover_tests(test-cl-smm)
set_cxx_common_options(test-cl-lib)
