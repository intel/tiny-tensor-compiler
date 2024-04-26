# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

add_executable(builder_c main.c)
target_link_libraries(builder_c PRIVATE tinytc)
set_c_common_options(builder_c)

add_executable(builder_cxx main.cpp)
target_link_libraries(builder_cxx PRIVATE tinytc)
set_cxx_common_options(builder_cxx)
