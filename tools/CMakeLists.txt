# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(argparser)
add_subdirectory(device_info)
if(BUILD_MOCHI)
    add_subdirectory(mochi)
endif()
add_subdirectory(offline_compiler)
add_subdirectory(opt)
