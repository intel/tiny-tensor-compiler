# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

function(install_lib target namespace)
    if (BUILD_SHARED_LIBS)
        set(type shared)
    else ()
        set(type static)
    endif ()

    set_target_properties(${target} PROPERTIES INSTALL_RPATH "\$ORIGIN")
    set_target_properties(${target} PROPERTIES INSTALL_RPATH_USE_LINK_PATH True)

    file(READ ${PROJECT_SOURCE_DIR}/cmake/template.cmake SHARED_STATIC_TEMPLATE)

    set(CONFIG_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${target})

    install(TARGETS ${target} EXPORT ${target}-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET HEADERS
    )
    install(EXPORT ${target}-targets
        FILE ${target}-${type}-targets.cmake
        NAMESPACE "${namespace}::"
        DESTINATION ${CONFIG_DESTINATION}
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${target}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config.cmake"
        INSTALL_DESTINATION ${CONFIG_DESTINATION}
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${target}-config-version.cmake"
        DESTINATION ${CONFIG_DESTINATION}
    )
endfunction()
