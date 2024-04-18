# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CTest)
include(CommonOptions)
include(${PROJECT_SOURCE_DIR}/external/doctest/cmake/doctest.cmake)

add_library(test-lib STATIC main.cpp)
target_include_directories(test-lib PUBLIC ${PROJECT_SOURCE_DIR}/external)
target_compile_features(test-lib PUBLIC cxx_std_20)
target_link_libraries(test-lib PUBLIC tinytc-objects)
target_include_directories(test-lib PUBLIC ${PROJECT_SOURCE_DIR}/src)
set_common_options(test-lib)

#add_executable(test-lexer lexer.cpp)
#target_link_libraries(test-lexer PRIVATE test-lib)
#doctest_discover_tests(test-lexer)

add_executable(test-generator generator.cpp)
target_link_libraries(test-generator PRIVATE test-lib)
doctest_discover_tests(test-generator)
set_common_options(test-generator)

add_executable(test-visitor visitor.cpp)
target_link_libraries(test-visitor PRIVATE test-lib)
doctest_discover_tests(test-visitor)
set_common_options(test-visitor)

configure_file(lit.site.cfg.py.in lit.site.cfg.py @ONLY)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in LIT_SITE_CONFIG)
string(CONFIGURE "${LIT_SITE_CONFIG}" LIT_SITE_CONFIG @ONLY)
file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
    CONTENT "${LIT_SITE_CONFIG}"
)

set(LIT_COMMAND lit "${CMAKE_CURRENT_BINARY_DIR}" -v)
add_test(lit-check ${LIT_COMMAND})
set_tests_properties(lit-check PROPERTIES LABELS "lit")
add_custom_target(lit-check COMMAND ${LIT_COMMAND})
add_dependencies(lit-check tinytc-oc)


if(BUILD_OPENCL)
    add_subdirectory(cl)
endif()
if(BUILD_SYCL)
    add_subdirectory(sycl)
endif()
if(BUILD_LEVEL_ZERO)
    add_subdirectory(ze)
endif()
