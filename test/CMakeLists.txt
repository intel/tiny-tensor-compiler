# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(CommonOptions)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

find_package(SPIRVTools)

add_library(test-lib STATIC main.cpp)
target_include_directories(test-lib PUBLIC ${PROJECT_SOURCE_DIR}/external)
target_compile_features(test-lib PUBLIC cxx_std_20)
target_link_libraries(test-lib PUBLIC tinytc-objects)
target_include_directories(test-lib PUBLIC ${PROJECT_SOURCE_DIR}/src)
set_cxx_common_options(test-lib)

#add_executable(test-lexer lexer.cpp)
#target_link_libraries(test-lexer PRIVATE test-lib)
#doctest_discover_tests(test-lexer)

add_executable(test-generator generator.cpp)
target_link_libraries(test-generator PRIVATE test-lib)
doctest_discover_tests(test-generator)
set_cxx_common_options(test-generator)

add_executable(test-visitor visitor.cpp)
target_link_libraries(test-visitor PRIVATE test-lib)
doctest_discover_tests(test-visitor)
set_cxx_common_options(test-visitor)

add_library(test-lib-linalg STATIC linalg_blas_a2.cpp linalg_blas_a3.cpp linalg_types.cpp)
target_link_libraries(test-lib-linalg PUBLIC tinytc)
if (BUILD_DOUBLE_PRECISION_TESTS)
    target_compile_definitions(test-lib-linalg PUBLIC ENABLE_DOUBLE_PRECISION)
endif()
set_cxx_common_options(test-lib-linalg)

configure_file(lit.site.cfg.py.in lit.site.cfg.py @ONLY)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in LIT_SITE_CONFIG)
string(CONFIGURE "${LIT_SITE_CONFIG}" LIT_SITE_CONFIG @ONLY)
file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
    CONTENT "${LIT_SITE_CONFIG}"
)

set(LIT_COMMAND lit "${CMAKE_CURRENT_BINARY_DIR}" -v)
add_test(NAME lit-check COMMAND ${LIT_COMMAND})
set_tests_properties(lit-check PROPERTIES LABELS "lit")
add_custom_target(lit-check COMMAND ${LIT_COMMAND})
add_dependencies(lit-check tinytc-oc tinytc-opt)

if(SPIRVTools_FOUND)
    set(SPIRV_VAL_SOURCES
        spv/alloca.ir
        spv/arith.ir
        spv/arith_unary.ir
        spv/barrier.ir
        spv/builtin.ir
        spv/cast.ir
        spv/calling_convention.ir
        spv/compare.ir
        spv/cooperative_matrix_scale.ir
        spv/expand.ir
        spv/for.ir
        spv/fuse.ir
        spv/if.ir
        spv/load.ir
        spv/size.ir
        spv/store.ir
        spv/subview.ir
        spv/work_group.ir
        spv/unique_function_type.ir
    )
    foreach(SOURCE IN LISTS SPIRV_VAL_SOURCES)
        get_filename_component(TEST_NAME ${SOURCE} NAME_WE)
        set(CHECK_COMMAND $<TARGET_FILE:tinytc-oc> -O0 -gspirv ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE} | ${SPIRVTools_SPIRV_VAL} -)
        list(JOIN CHECK_COMMAND " " CHECK_COMMAND_STR)
        add_test(NAME spirv-val-${TEST_NAME} COMMAND bash -c "${CHECK_COMMAND_STR}")
        add_custom_target(spirv-val-${TEST_NAME} COMMAND ${CHECK_COMMAND})
        add_dependencies(spirv-val-${TEST_NAME} tinytc-oc)
    endforeach()
endif()

if(BUILD_OPENCL)
    add_subdirectory(cl)
endif()
if(BUILD_SYCL)
    add_subdirectory(sycl)
endif()
if(BUILD_LEVEL_ZERO)
    add_subdirectory(ze)
endif()
