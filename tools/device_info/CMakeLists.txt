# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GNUInstallDirs)

if(BUILD_LEVEL_ZERO)
    find_package(LevelZero REQUIRED)

    add_executable(tinytc-ze-info main_ze.cpp)
    target_link_libraries(tinytc-ze-info PRIVATE tinytc tinytc_ze LevelZero::LevelZero)
    set_cxx_common_options(tinytc-ze-info)

    set_target_properties(tinytc-ze-info PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)
    set_target_properties(tinytc-ze-info PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

    install(TARGETS tinytc-ze-info
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(BUILD_OPENCL)
    find_package(OpenCL REQUIRED)

    add_executable(tinytc-cl-info main_cl.cpp)
    target_link_libraries(tinytc-cl-info PRIVATE tinytc tinytc_cl OpenCL::OpenCL)
    set_cxx_common_options(tinytc-cl-info)

    set_target_properties(tinytc-cl-info PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)
    set_target_properties(tinytc-cl-info PROPERTIES INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

    install(TARGETS tinytc-cl-info
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
