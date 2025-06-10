# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# Code COPIED from Double-Batched FFT Library 
# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CheckCompilerFlag)
include(CheckLinkerFlag)
include(GitVersion)

function(add_flag_if_available lang target flag)
    string(MAKE_C_IDENTIFIER ${flag} flag_c)
    check_compiler_flag(${lang} ${flag} HAVE_${lang}_FLAG${flag_c})
    if(HAVE_${lang}_FLAG${flag_c})
        target_compile_options(${target} PRIVATE ${flag})
    endif()
endfunction()

function(add_flag_if_available_to_source_files lang sources flag)
    string(MAKE_C_IDENTIFIER ${flag} flag_c)
    check_compiler_flag(${lang} ${flag} HAVE_${lang}_FLAG${flag_c})
    if(HAVE_${lang}_FLAG${flag_c})
        set_source_files_properties(${sources} PROPERTIES COMPILE_FLAGS ${flag})
    endif()
endfunction()

function(add_linker_flag_if_available lang target flag)
    string(MAKE_C_IDENTIFIER ${flag} flag_c)
    check_linker_flag(${lang} ${flag} HAVE_${lang}_FLAG${flag_c})
    if(HAVE_${lang}_FLAG${flag_c})
        target_link_options(${target} PRIVATE ${flag})
    endif()
endfunction()

function(set_common_options lang target)
    add_flag_if_available(${lang} ${target} -ffp-model=precise)
    add_flag_if_available(${lang} ${target} -Wall)
    add_flag_if_available(${lang} ${target} -Wextra)
    add_flag_if_available(${lang} ${target} -Wpedantic)
    add_flag_if_available(${lang} ${target} -Wundefined-func-template)
    add_flag_if_available(${lang} ${target} -Wformat)
    add_flag_if_available(${lang} ${target} -Wformat-security)
    add_flag_if_available(${lang} ${target} -Werror=format-security)
    add_flag_if_available(${lang} ${target} -Wstrict-aliasing)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_linker_flag_if_available(${lang} ${target} -Wl,-z,noexecstack)
        add_linker_flag_if_available(${lang} ${target} -Wl,-z,relro,-z,now)
        add_flag_if_available(${lang} ${target} -fstack-protector-strong)
    else()
        add_flag_if_available(${lang} ${target} -fstack-protector)
    endif()
    set_target_properties(${target} PROPERTIES
                          VERSION ${tiny_tensor_compiler_VERSION}
                          SOVERSION ${tiny_tensor_compiler_VERSION_MAJOR})
    set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_compile_definitions(${target} PRIVATE -D_FORTIFY_SOURCE=2)

    git_version()
    set_target_properties(${target} PROPERTIES
                          VERSION ${GIT_MAJOR_VERSION}.${GIT_MINOR_VERSION}.${GIT_PATCH_VERSION}
                          SOVERSION ${GIT_MAJOR_VERSION})
endfunction()

function(set_c_common_options target)
    set_common_options(C ${target})
endfunction()

function(set_cxx_common_options target)
    set_common_options(CXX ${target})
    target_compile_features(${target} PRIVATE cxx_std_20)
endfunction()
