# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

include(CommonOptions)

find_package(SYCL REQUIRED)

function(embed_source TARGET SOURCE)
    set(OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE}.o)
    set(LIB ${SOURCE}_lib)
    add_custom_command(
       OUTPUT ${OBJ_FILE}
       COMMAND ${CMAKE_LINKER} -r -b binary -o ${OBJ_FILE} ${SOURCE} -z noexecstack
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
       DEPENDS ${SOURCE}
       COMMENT "Embedding ${SOURCE}"
    )
    add_library(${LIB} OBJECT IMPORTED)
    set_property(TARGET ${LIB} PROPERTY IMPORTED_OBJECTS ${OBJ_FILE})
    target_link_libraries(${TARGET} PRIVATE ${LIB})
endfunction()

add_executable(flash_attention main.cpp)
add_sycl_to_target(TARGET flash_attention SOURCES main.cpp)
target_link_libraries(flash_attention PRIVATE tinytc tinytc_sycl argparser)
embed_source(flash_attention flash_attention.template)
set_cxx_common_options(flash_attention)
