# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

function(get_generated_files VAR target)
    get_target_property(_sources ${target} SOURCES)
    set(_generated_files "")
    foreach(_src ${_sources})
        get_source_file_property(_generated "${_src}" GENERATED)
        if(${_generated} GREATER 0)
            list(APPEND _generated_files ${_src})
        endif()
    endforeach()
    set(${VAR} ${_generated_files} PARENT_SCOPE)
endfunction()
