# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.23)

project(tiny_tensor_compiler VERSION 0.3.1 LANGUAGES C CXX)

include(CMakeDependentOption)
option(BUILD_DOCUMENTATION "Build documentation" OFF)
option(BUILD_SYCL "Build support for SYCL run-time" ON)
cmake_dependent_option(BUILD_LEVEL_ZERO
    "Build support for Level Zero run-time; required when SYCL is enabled"
    ON "NOT BUILD_SYCL" ON)
cmake_dependent_option(BUILD_OPENCL
    "Build support for OpenCL run-time; required when SYCL is enabled"
    ON "NOT BUILD_SYCL" ON)

include(CTest)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(DEFINED tinytc_SHARED_LIBS)
    set(BUILD_SHARED_LIBS "${tinytc_SHARED_LIBS}")
endif()
if (NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET AND NOT DEFINED CMAKE_VISIBILITY_INLINES_HIDDEN)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Setting build type to \"Release\".")
endif()

add_subdirectory(examples)
add_subdirectory(src)
add_subdirectory(tools)

if(BUILD_DOCUMENTATION)
    add_subdirectory(docs)
endif()

enable_testing()
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
