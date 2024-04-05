# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

function(add_flag_if_available target flag)
    string(MAKE_C_IDENTIFIER ${flag} flag_c)
    check_cxx_compiler_flag(${flag} HAVE_FLAG${flag_c})
    if(HAVE_FLAG${flag_c})
        target_compile_options(${target} PRIVATE ${flag})
    endif()
endfunction()

function(add_flag_if_available_to_source_files sources flag)
    string(MAKE_C_IDENTIFIER ${flag} flag_c)
    check_cxx_compiler_flag(${flag} HAVE_FLAG${flag_c})
    if(HAVE_FLAG${flag_c})
        set_source_files_properties(${sources} PROPERTIES COMPILE_FLAGS ${flag})
    endif()
endfunction()

function(add_linker_flag_if_available target flag)
    string(MAKE_C_IDENTIFIER ${flag} flag_c)
    check_linker_flag(CXX ${flag} HAVE_FLAG${flag_c})
    if(HAVE_FLAG${flag_c})
        target_link_options(${target} PRIVATE ${flag})
    endif()
endfunction()

function(set_common_options target)
    add_flag_if_available(${target} -ffp-model=precise)
    add_flag_if_available(${target} -Wall)
    add_flag_if_available(${target} -Wextra)
    add_flag_if_available(${target} -Wpedantic)
    add_flag_if_available(${target} -Wundefined-func-template)
    add_flag_if_available(${target} -Wformat)
    add_flag_if_available(${target} -Wformat-security)
    add_flag_if_available(${target} -Werror=format-security)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_linker_flag_if_available(${target} -Wl,-z,noexecstack)
        add_linker_flag_if_available(${target} -Wl,-z,relro,-z,now)
        add_flag_if_available(${target} -fstack-protector-strong)
    else()
        add_flag_if_available(${target} -fstack-protector)
    endif()
    set_target_properties(${target} PROPERTIES
                          VERSION ${tiny_tensor_compiler_VERSION}
                          SOVERSION ${tiny_tensor_compiler_VERSION_MAJOR})
    set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_compile_definitions(${target} PRIVATE -D_FORTIFY_SOURCE=2)
endfunction()
