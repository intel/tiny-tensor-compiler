# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Code COPIED from SharedStaticStarter
# Copyright (C) 2021 Alex Reinking
# SPDX-License-Identifier: MIT

set(known_comps static shared)
set(comp_static NO)
set(comp_shared NO)

foreach (comp IN LISTS ${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
    if (comp IN_LIST known_comps)
        set(comp_${comp} YES)
    else ()
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "Unknown component `${comp}`.")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        return()
    endif ()
endforeach ()

unset(known_comps)

if (comp_static AND comp_shared)
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
        "The components `static` and `shared` are mutually exclusive.")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    return()
endif ()

set(static_targets "${CMAKE_CURRENT_LIST_DIR}/${CMAKE_FIND_PACKAGE_NAME}-static-targets.cmake")
set(shared_targets "${CMAKE_CURRENT_LIST_DIR}/${CMAKE_FIND_PACKAGE_NAME}-shared-targets.cmake")

macro(load_targets type)
    if (NOT EXISTS "${${type}_targets}")
        set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE
            "Requested `${type}` libraries not found. (Missing: ${${type}_targets}.)")
        set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
        return()
    endif ()
    load_dependencies(${type})
    include("${${type}_targets}")
endmacro()

if (comp_static)
    load_targets(static)
elseif (comp_shared)
    load_targets(shared)
elseif (DEFINED tinytc_SHARED_LIBS AND tinytc_SHARED_LIBS)
    load_targets(shared)
elseif (DEFINED tinytc_SHARED_LIBS AND NOT tinytc_SHARED_LIBS)
    load_targets(static)
elseif (BUILD_SHARED_LIBS)
    if (EXISTS "${shared_targets}")
        load_targets(shared)
    else ()
        load_targets(static)
    endif ()
else ()
    if (EXISTS "${static_targets}")
        load_targets(static)
    else ()
        load_targets(shared)
    endif ()
endif ()

unset(static_targets)
unset(shared_targets)
unset(load_targets)
unset(comp_static)
unset(comp_shared)
unset(load_dependencies)
