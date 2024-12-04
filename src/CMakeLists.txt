# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)
include(GenerateExportHeader)
include(GitVersion)
include(InstallLib)

if (BUILD_SHARED_LIBS)
    set(type shared)
else ()
    set(type static)
endif ()

find_package(re2c REQUIRED)
find_package(BISON 3.8.2 REQUIRED)

set(SOURCES
    analysis/aa_results.cpp
    analysis/alias.cpp
    analysis/cfg.cpp
    analysis/gcd.cpp
    analysis/equal.cpp
    analysis/stack.cpp
    binary.cpp
    codegen_tools.cpp
    compiler.cpp
    compiler_context.cpp
    compiler_context_cache.cpp
    coopmatrix_info.cpp
    data_type.cpp
    device_info.cpp
    error.cpp
    func.cpp
    gemm_tools.cpp
    half.cpp
    inst.cpp
    location.cpp
    node/data_type_node.cpp
    node/function_node.cpp
    node/inst_node.cpp
    node/region_node.cpp
    node/program_node.cpp
    node/value_node.cpp
    parser/parse_context.cpp
    parser.cpp
    pass/alignment_propagation.cpp
    pass/check_ir.cpp
    pass/clone.cpp
    pass/constant_folding.cpp
    pass/constant_propagation.cpp
    pass/convert_to_spirv.cpp
    pass/dead_code_elimination.cpp
    pass/dump_cfg.cpp
    pass/dump_def_use.cpp
    pass/dump_gcd.cpp
    pass/dump_ir.cpp
    pass/insert_barrier.cpp
    pass/insert_lifetime_stop.cpp
    pass/lower_foreach.cpp
    pass/lower_linalg.cpp
    pass/slot_tracker.cpp
    pass/stack.cpp
    pass/work_group_size.cpp
    prog.cpp
    recipe.cpp
    recipe/small_gemm_batched.cpp
    recipe/tall_and_skinny.cpp
    region.cpp
    scalar_type.cpp
    spv/capex_util.cpp
    spv/converter.cpp
    spv/inst_assembler.cpp
    spv/module.cpp
    spv/names.cpp
    spv/opencl.std.cpp
    spv/pass/assemble.cpp
    spv/pass/assign_ids.cpp
    spv/pass/dump_asm.cpp
    spv/pass/capex.cpp
    spv/uniquifier.cpp
    tiling.cpp
    value.cpp
    support/walk.cpp
)
set(RE2C_SOURCES
    parser/lexer.re
)
BISON_TARGET(parser parser/parser_impl.yy ${CMAKE_CURRENT_BINARY_DIR}/parser/parser_impl.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser_impl.hpp)
set(PUBLIC_HEADERS
    tinytc.h
    tinytc.hpp
    types.h
    types.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_flag_if_available_to_source_files(CXX "${BISON_parser_OUTPUTS}" "-Wno-unused-but-set-variable")

add_library(tinytc-objects OBJECT ${SOURCES} ${BISON_parser_OUTPUTS})
add_re2c_to_target(TARGET tinytc-objects SOURCES ${RE2C_SOURCES})
set_cxx_common_options(tinytc-objects)
target_compile_definitions(tinytc-objects PUBLIC
    "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:TINYTC_STATIC_DEFINE>")

add_library(tinytc $<TARGET_OBJECTS:tinytc-objects>)
add_library(tinytc::tinytc ALIAS tinytc)
set_cxx_common_options(tinytc)

# Generate export header
set(tinytc_export_header "${CMAKE_BINARY_DIR}/include/tinytc/export.h")
generate_export_header(tinytc BASE_NAME TINYTC EXPORT_FILE_NAME "${tinytc_export_header}")

# Generate version header
set(tinytc_version_header_in "${PROJECT_SOURCE_DIR}/include/tinytc/version.h.in")
set(tinytc_version_header "${PROJECT_BINARY_DIR}/include/tinytc/version.h")
git_version()
configure_file(${tinytc_version_header_in} ${tinytc_version_header})

# include directories

target_include_directories(tinytc-objects PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)
target_include_directories(tinytc-objects PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
)

target_sources(tinytc PUBLIC FILE_SET HEADERS
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include
              ${PROJECT_BINARY_DIR}/include
    FILES ${PUBLIC_HEADERS}
          "${tinytc_export_header}"
          "${tinytc_version_header}")


# install

install_lib(tinytc tinytc)

# subdirs
if(BUILD_OPENCL)
    add_subdirectory(cl)
endif()
if(BUILD_SYCL)
    add_subdirectory(sycl)
endif()
if(BUILD_LEVEL_ZERO)
    add_subdirectory(ze)
endif()
