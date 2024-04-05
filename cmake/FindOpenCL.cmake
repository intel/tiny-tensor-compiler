# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find OpenCL

# The following definitions are added on success
# 
#  OpenCL_FOUND - OpenCL was found
#  OpenCL_INCLUDE_DIR - OpenCL include directory
#  OpenCL_LIBRARY - OpenCL loader library
#  OpenCL_COMPILE_DEFINITIONS - OpenCL compile definitions
#  OpenCL_VERSION_STRING - Maximum supported OpenCL version
#  OpenCL_VERSION_MAJOR - OpenCL version major
#  OpenCL_VERSION_MINOR - OpenCL version minor
#
# and the following imported target:
#
#  OpenCL::OpenCL - OpenCL library
#
# The followings hints may be passed in the environment:
#
# OpenCL_ROOT
# AMDAPPSDKROOT
# INTELOCLSDKROOT
# CUDA_PATH
# NVSDKCOMPUTE_ROOT
# ATISTREAMSDKROOT
# OCL_ROOT
#

function(check_opencl_version)
    include(CMakePushCheckState)
    include(CheckSymbolExists)

    foreach(version "3;0" "2;2" "2;0" "1;2" "1;1" "1;0")
        cmake_push_check_state()

        list(GET version 0 major_version)
        list(GET version 1 minor_version)
        set(compile_def "CL_TARGET_OPENCL_VERSION=${major_version}${minor_version}0")
        list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${compile_def}")
        list(APPEND CMAKE_REQUIRED_INCLUDES ${OpenCL_INCLUDE_DIR})
        check_symbol_exists("CL_VERSION_${major_version}_${minor_version}"
            "CL/cl.h"
            OPENCL_VERSION_${major_version}_${minor_version})

        if(OPENCL_VERSION_${major_version}_${minor_version})
            set(OpenCL_COMPILE_DEFINITIONS ${compile_def} PARENT_SCOPE)
            set(OpenCL_VERSION_STRING "${major_version}.${minor_version}" PARENT_SCOPE)
            set(OpenCL_VERSION_MAJOR "${major_version}" PARENT_SCOPE)
            set(OpenCL_VERSION_MINOR "${minor_version}" PARENT_SCOPE)
            break()
        endif()

        cmake_pop_check_state()
    endforeach()
endfunction()

if(OpenCL_INCLUDE_DIR AND OpenCL_LIBRARY AND OpenCL_COMPILE_DEFINITIONS AND OpenCL_VERSION_STRING)
    set(OpenCL_FOUND TRUE)
else()
    get_filename_component(COMPILER_PATH ${CMAKE_CXX_COMPILER} DIRECTORY)

    find_path(OpenCL_INCLUDE_DIR NAMES CL/cl.h OpenCL/cl.h
        HINTS
            ENV OpenCL_ROOT
            ENV CPATH
            ENV AMDAPPSDKROOT
            ENV INTELOCLSDKROOT
            ENV CUDA_PATH
            ENV NVSDKCOMPUTE_ROOT
            ENV ATISTREAMSDKROOT
            ENV OCL_ROOT
            "${COMPILER_PATH}/../include"
            "${COMPILER_PATH}/../include/sycl"
        PATH_SUFFIXES
            include
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        find_library(OpenCL_LIBRARY NAMES OpenCL
            HINTS
                ENV OpenCL_ROOT
                ENV LIBRARY_PATH
                ENV LD_LIBRARY_PATH
                ENV AMDAPPSDKROOT
                ENV INTELOCLSDKROOT
                ENV CUDA_PATH
                ENV NVSDKCOMPUTE_ROOT
                ENV ATISTREAMSDKROOT
                ENV OCL_ROOT
                "${COMPILER_PATH}/../lib"
            PATH_SUFFIXES
                lib/x86
                lib32
                lib
        )
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        find_library(OpenCL_LIBRARY NAMES OpenCL
            HINTS
                ENV OpenCL_ROOT
                ENV LIBRARY_PATH
                ENV LD_LIBRARY_PATH
                ENV AMDAPPSDKROOT
                ENV INTELOCLSDKROOT
                ENV CUDA_PATH
                ENV NVSDKCOMPUTE_ROOT
                ENV ATISTREAMSDKROOT
                ENV OCL_ROOT
                "${COMPILER_PATH}/../lib"
            PATH_SUFFIXES
                lib/x86_64
                lib/x64
                lib64
                lib
        )
    endif()

    check_opencl_version()

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(OpenCL DEFAULT_MSG
        OpenCL_INCLUDE_DIR
        OpenCL_LIBRARY
        OpenCL_COMPILE_DEFINITIONS
        OpenCL_VERSION_STRING)

    mark_as_advanced(OpenCL_INCLUDE_DIR
        OpenCL_LIBRARY
        OpenCL_COMPILE_DEFINITIONS
        OpenCL_VERSION_STRING
        OpenCL_VERSION_MAJOR
        OpenCL_VERSION_MINOR)
endif()

if(OpenCL_FOUND AND NOT TARGET OpenCL::OpenCL)
    add_library(OpenCL::OpenCL INTERFACE IMPORTED)
    set_target_properties(OpenCL::OpenCL PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "${OpenCL_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenCL_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${OpenCL_LIBRARY}")
endif()
