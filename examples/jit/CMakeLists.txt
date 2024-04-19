# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

add_executable(jit-example main.cpp)
target_link_libraries(jit-example PRIVATE tinytc)
set_cxx_common_options(jit-example)
