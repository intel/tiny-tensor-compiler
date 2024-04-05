# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find ocloc library

# The following definitions are added on success
# 
#  ocloc_FOUND - ocloc library was found
#  ocloc_INCLUDE_DIR - ocloc include directory
#  ocloc_LIBRARY - ocloc library
#
# and the following imported target:
#
#  ocloc::ocloc - ocloc library
#
# The followings hints may be passed in the environment:
#
# ocloc_ROOT
#

if(ocloc_INCLUDE_DIR AND ocloc_LIBRARY)
    set(ocloc_FOUND TRUE)
else()
    find_path(ocloc_INCLUDE_DIR NAMES ocloc_api.h
        HINTS
            ENV ocloc_ROOT
            ENV CPATH
        PATH_SUFFIXES
            include
    )

    find_library(ocloc_LIBRARY NAMES ocloc
        HINTS
            ENV ocloc_ROOT
            ENV LIBRARY_PATH
            ENV LD_LIBRARY_PATH
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(ocloc DEFAULT_MSG ocloc_INCLUDE_DIR ocloc_LIBRARY)

    mark_as_advanced(ocloc_INCLUDE_DIR ocloc_LIBRARY)
endif()

if(ocloc_FOUND AND NOT TARGET ocloc::ocloc)
    add_library(ocloc::ocloc INTERFACE IMPORTED)
    set_target_properties(ocloc::ocloc PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ocloc_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${ocloc_LIBRARY}")
endif()
