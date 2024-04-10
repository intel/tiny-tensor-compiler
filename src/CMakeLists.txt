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

find_package(clir 0.5.1 REQUIRED ${type})
find_package(ocloc REQUIRED)
find_package(re2c REQUIRED)
find_package(BISON 3.8.2 REQUIRED)

set(SOURCES
    binary.cpp
    device_info.cpp
    error.cpp
    internal/compiler_options.cpp
    ir/builder.cpp
    ir/codegen_tools.cpp
    ir/data_type.cpp
    ir/gemm_generator.cpp
    ir/inst.cpp
    ir/location.cpp
    ir/node/data_type_node.cpp
    ir/node/inst_node.cpp
    ir/passes.cpp
    ir/precision_helper.cpp
    ir/prog.cpp
    ir/scalar_type.cpp
    ir/tiling.cpp
    ir/value.cpp
    ir/visitor/aa_results.cpp
    ir/visitor/alias_analysis.cpp
    ir/visitor/check_ir.cpp
    ir/visitor/dump_ir.cpp
    ir/visitor/equal.cpp
    ir/visitor/insert_barrier.cpp
    ir/visitor/lifetime_analysis.cpp
    ir/visitor/opencl_ast.cpp
    ir/visitor/stack.cpp
    ir/visitor/work_group_size.cpp
    opencl_cc.cpp
    parser/parse_context.cpp
    parser.cpp
    #recipe/small_gemm_batched.cpp
    #recipe/tall_and_skinny.cpp
    tinytc.h
    tinytc.hpp
    types.h
    types.hpp
)
set(RE2C_SOURCES
    parser/lexer.re
)
BISON_TARGET(parser parser/parser_impl.yy ${CMAKE_CURRENT_BINARY_DIR}/parser/parser_impl.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser/parser_impl.hpp)
set(PUBLIC_HEADERS
    binary.hpp
    bundle_format.hpp
    device_info.hpp
    internal/compiler_options.hpp
    ir/builder.hpp
    ir/data_type.hpp
    ir/error.hpp
    ir/func.hpp
    ir/gemm_generator.hpp
    ir/inst.hpp
    ir/location.hpp
    ir/passes.hpp
    ir/prog.hpp
    ir/region.hpp
    ir/scalar_type.hpp
    ir/slice.hpp
    ir/tiling.hpp
    ir/value.hpp
    opencl_cc.hpp
    parser.hpp
    recipe/small_gemm_batched.hpp
    recipe/tall_and_skinny.hpp
    runtime.hpp
    tensor_kernel.hpp
    tinytc.hpp
    tinytc-level-zero.hpp
    tinytc-opencl.hpp
)
list(TRANSFORM PUBLIC_HEADERS PREPEND "${PROJECT_SOURCE_DIR}/include/tinytc/")

add_flag_if_available_to_source_files("${BISON_parser_OUTPUTS}" "-Wno-unused-but-set-variable")

add_library(tinytc ${SOURCES} ${BISON_parser_OUTPUTS})
add_library(tinytc::tinytc ALIAS tinytc)
add_re2c_to_target(TARGET tinytc SOURCES ${RE2C_SOURCES})

set_common_options(tinytc)
target_link_libraries(tinytc PUBLIC clir::clir ocloc::ocloc)
target_compile_definitions(tinytc PUBLIC
    "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:TINYTC_STATIC_DEFINE>")

# Generate export header
set(tinytc_export_header "${CMAKE_BINARY_DIR}/include/tinytc/export.h")
generate_export_header(tinytc BASE_NAME TINYTC EXPORT_FILE_NAME "${tinytc_export_header}")

# Generate version header
set(tinytc_version_header_in "${PROJECT_SOURCE_DIR}/include/tinytc/version.h.in")
set(tinytc_version_header "${PROJECT_BINARY_DIR}/include/tinytc/version.h")
git_version()
configure_file(${tinytc_version_header_in} ${tinytc_version_header})

# include directories

target_include_directories(tinytc PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)
target_include_directories(tinytc PUBLIC
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