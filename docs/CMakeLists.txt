# Copyright (C) 2024 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(GNUInstallDirs)

find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED COMPONENTS breathe sphinx_book_theme sphinx_tabs)

set(DOXYFILE_TEMPLATE Doxyfile.in)
set(DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

get_target_property(DOC_HEADERS tinytc HEADER_SET)
if(BUILD_LEVEL_ZERO)
    get_target_property(DOC_ZE_HEADERS tinytc_ze HEADER_SET)
    list(APPEND DOC_HEADERS ${DOC_ZE_HEADERS})
endif()
if(BUILD_OPENCL)
    get_target_property(DOC_CL_HEADERS tinytc_cl HEADER_SET)
    list(APPEND DOC_HEADERS ${DOC_CL_HEADERS})
endif()
if(BUILD_SYCL)
    get_target_property(DOC_SYCL_HEADERS tinytc_sycl HEADER_SET)
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
    api/builder_capi.rst
    api/builder_cxxapi.rst
    api/core_capi.rst
    api/core_cxxapi.rst
    api/index.rst
    api/cl/capi.rst
    api/cl/cxxapi.rst
    api/cl/index.rst
    api/sycl/cxxapi.rst
    api/sycl/index.rst
    api/ze/capi.rst
    api/ze/cxxapi.rst
    api/ze/index.rst
    dev/coopmatrix_layout.rst
    manual/build.rst
    manual/builder.rst
    manual/calling_convention.rst
    manual/core.rst
    manual/tensor-ir.rst
    manual/tutorial_matrix_chain.rst
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
