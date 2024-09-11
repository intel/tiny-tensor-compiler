# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)

add_executable(tinytc-opt main.cpp args.cpp)
target_link_libraries(tinytc-opt PRIVATE tinytc)
set_cxx_common_options(tinytc-opt)

set_target_properties(tinytc-opt PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)
set_target_properties(tinytc-opt PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

install(TARGETS tinytc-opt
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
