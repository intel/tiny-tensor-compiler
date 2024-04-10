# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(GNUInstallDirs)

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED COMPONENTS breathe sphinx_book_theme)

set(DOXYFILE_TEMPLATE Doxyfile.in)
set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

get_target_property(DOC_HEADERS tinytc HEADER_SET)
if(BUILD_LEVEL_ZERO)
    get_target_property(DOC_ZE_HEADERS tinytc-level-zero HEADER_SET)
    list(APPEND DOC_HEADERS ${DOC_ZE_HEADERS})
endif()
if(BUILD_OPENCL)
    get_target_property(DOC_CL_HEADERS tinytc-opencl HEADER_SET)
    list(APPEND DOC_HEADERS ${DOC_CL_HEADERS})
endif()
if(BUILD_SYCL)
    get_target_property(DOC_SYCL_HEADERS tinytc-sycl HEADER_SET)
    list(APPEND DOC_HEADERS ${DOC_SYCL_HEADERS})
endif()

string(REPLACE ";" " " DOC_HEADERS_STRING "${DOC_HEADERS}")
configure_file(${DOXYFILE_TEMPLATE} ${DOXYFILE} @ONLY)

set(DOXYGEN_XML_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/xml)
set(DOXYGEN_XML_INDEX ${DOXYGEN_XML_OUTPUT_DIR}/index.xml)

add_custom_command(OUTPUT ${DOXYGEN_XML_INDEX}
    MAIN_DEPENDENCY ${DOXYFILE_TEMPLATE} ${DOXYFILE}
    DEPENDS ${DOC_HEADERS}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(doc-doxygen ALL DEPENDS ${DOXYGEN_XML_INDEX})

set(DOC_FILES
    index.rst
    api/introduction.rst
    api/index.rst
    api/jit.rst
    api/parser.rst
    api/runtime.rst
    api/ir/builder.rst
    api/ir/builder_example.cpp
    api/ir/builder_example.ir
    api/ir/error.rst
    api/ir/generator.rst
    api/ir/index.rst
    api/ir/passes.rst
    api/ir/tiling.rst
    api/recipe/index.rst
    api/recipe/small_gemm_batched.rst
    api/recipe/tall_and_skinny.rst
    api/cl/error.rst
    api/cl/index.rst
    api/cl/info.rst
    api/cl/runtime.rst
    api/sycl/info.rst
    api/sycl/index.rst
    api/sycl/runtime.rst
    api/ze/error.rst
    api/ze/index.rst
    api/ze/info.rst
    api/ze/runtime.rst
    manual/build.rst
    manual/calling_convention.rst
    manual/tensor-ir.rst
    manual/usage.rst
)

set(SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SPHINX_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/_build)
set(SPHINX_INDEX ${SPHINX_BUILD_DIR}/index.html)
add_custom_command(OUTPUT ${SPHINX_INDEX}
    MAIN_DEPENDENCY ${SPHINX_SOURCE_DIR}/conf.py
    DEPENDS ${DOC_FILES} ${DOXYGEN_XML_INDEX}
    COMMAND ${Sphinx_BUILD_COMMAND}
        ${SPHINX_SOURCE_DIR} ${SPHINX_BUILD_DIR}
        -Dbreathe_projects.api=${DOXYGEN_XML_OUTPUT_DIR}
    COMMAND_EXPAND_LISTS
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(docs ALL DEPENDS ${SPHINX_INDEX})

install(DIRECTORY ${SPHINX_BUILD_DIR}/ DESTINATION ${CMAKE_INSTALL_DOCDIR})