# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)

add_executable(tinytc-opt main.cpp)
target_link_libraries(tinytc-opt PRIVATE tinytc)
target_compile_features(tinytc-opt PRIVATE cxx_std_20)
set_common_options(tinytc-opt)

set_target_properties(tinytc-opt PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)

install(TARGETS tinytc-opt
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
