# Copyright (C) 2025 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

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
