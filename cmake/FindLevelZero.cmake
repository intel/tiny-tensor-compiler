# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Try to find Level Zero

# The following definitions are added on success
# 
#  LevelZero_FOUND - Level Zero was found
#  LevelZero_INCLUDE_DIR - Level Zero include directory
#  LevelZero_LIBRARY - Level Zero loader library
#
# and the following imported target:
#
#  LevelZero::LevelZero - LevelZero library
#
# The followings hints may be passed in the environment:
#
# LevelZero_ROOT
# L0_ROOT
#

if(LevelZero_INCLUDE_DIR AND LevelZero_LIBRARY)
    set(LevelZero_FOUND TRUE)
else()
    find_path(LevelZero_INCLUDE_DIR NAMES level_zero/ze_api.h
        HINTS
            ENV LevelZero_ROOT
            ENV L0_ROOT
            ENV CPATH
        PATH_SUFFIXES
            include
    )

    find_library(LevelZero_LIBRARY NAMES ze_loader
        HINTS
        ENV LevelZero_ROOT
        ENV L0_ROOT
        ENV LIBRARY_PATH
        ENV LD_LIBRARY_PATH
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(LevelZero DEFAULT_MSG LevelZero_INCLUDE_DIR LevelZero_LIBRARY)

    mark_as_advanced(LevelZero_INCLUDE_DIR LevelZero_LIBRARY)
endif()

if(LevelZero_FOUND AND NOT TARGET LevelZero::LevelZero)
    add_library(LevelZero::LevelZero INTERFACE IMPORTED)
    set_target_properties(LevelZero::LevelZero PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${LevelZero_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${LevelZero_LIBRARY}")
endif()
