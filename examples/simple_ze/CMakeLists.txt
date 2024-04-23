# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

find_package(LevelZero REQUIRED)

add_executable(simple_ze main.c)
target_link_libraries(simple_ze PRIVATE tinytc tinytc_ze LevelZero::LevelZero)
set_c_common_options(simple_ze)
