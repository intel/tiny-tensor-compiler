# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find SYCL
# TODO: Support other SYCL implementations besides Intel DPC++

# The following definitions are added on success
# 
#  SYCL_FOUND - SYCL was found
#  SYCL_COMPILE_OPTIONS - SYCL compile options
#  SYCL_LINK_OPTIONS - SYCL link options
#
# and the following imported target:
#
#  SYCL::SYCL - SYCL target
#

if(SYCL_COMPILE_OPTIONS AND SYCL_LINK_OPTIONS)
    set(SYCL_FOUND TRUE)
else()
    include(CheckCXXSourceCompiles)
    include(CMakePushCheckState)
    include(CheckCXXCompilerFlag)

    check_cxx_compiler_flag("-fsycl" HAS_FSYCL_FLAG)
    if(HAS_FSYCL_FLAG)
        set(SYCL_COMPILE_OPTIONS "-fsycl")
        set(SYCL_LINK_OPTIONS "-fsycl")
    endif()

    cmake_push_check_state()
    set(CMAKE_REQUIRED_FLAGS "${SYCL_COMPILE_OPTIONS}")
    set(CMAKE_REQUIRED_LINK_OPTIONS "${SYCL_LINK_OPTIONS}")
    check_cxx_source_compiles("#include <CL/sycl.hpp>
    int main() {
        sycl::queue{};
        return 0;
    }" SYCL_COMPILES)
    cmake_pop_check_state()

    if(NOT SYCL_COMPILES)
        unset(SYCL_COMPILE_OPTIONS)
    endif()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SYCL DEFAULT_MSG
        SYCL_COMPILE_OPTIONS
        SYCL_LINK_OPTIONS)

    mark_as_advanced(SYCL_LINK_OPTIONS)
endif()

if(SYCL_FOUND AND NOT TARGET SYCL::SYCL)
    add_library(SYCL::SYCL INTERFACE IMPORTED)
    set_target_properties(SYCL::SYCL PROPERTIES
        INTERFACE_COMPILE_OPTIONS "${SYCL_COMPILE_OPTIONS}"
        INTERFACE_LINK_OPTIONS "${SYCL_LINK_OPTIONS}")

    function(add_sycl_to_target)
        cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET" "SOURCES")
        get_target_property(TARGET_SOURCE_DIR ${ARG_TARGET} SOURCE_DIR)
        set_property(SOURCE ${ARG_SOURCES} DIRECTORY ${TARGET_SOURCE_DIR}
                     APPEND PROPERTY COMPILE_OPTIONS "${SYCL_COMPILE_OPTIONS}")
        target_link_options(${ARG_TARGET} PRIVATE "${SYCL_LINK_OPTIONS}")
    endfunction()
endif()
