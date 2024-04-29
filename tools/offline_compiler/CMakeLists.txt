# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)

add_executable(tinytc-oc main.cpp)
target_link_libraries(tinytc-oc PRIVATE tinytc)
set_target_properties(tinytc-oc PROPERTIES OUTPUT_NAME "tinytc")
set_cxx_common_options(tinytc-oc)

set_target_properties(tinytc-oc PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)
set_target_properties(tinytc-oc PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

install(TARGETS tinytc-oc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
