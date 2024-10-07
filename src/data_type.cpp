// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context.hpp"
#include "error.hpp"
#include "location.hpp"
#include "node/data_type_node.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_scalar_type_get(tinytc_data_type_t *dt, tinytc_compiler_context_t ctx,
                                       tinytc_scalar_type_t type) {
    if (dt == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code(
        [&] { *dt = scalar_data_type::get(ctx, enum_cast<scalar_type>(type)); });
}

tinytc_status_t tinytc_memref_type_get(tinytc_data_type_t *dt, tinytc_data_type_t scalar_ty,
                                       uint32_t shape_size, const int64_t *shape,
                                       uint32_t stride_size, const int64_t *stride,
                                       tinytc_address_space_t addrspace,
                                       const tinytc_location_t *loc) {
    if (dt == nullptr || (shape_size != 0 && shape == nullptr) ||
        (stride_size != 0 && stride == nullptr)) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        *dt = memref_data_type::get(scalar_ty, array_view{shape, shape_size},
                                    array_view{stride, stride_size},
                                    enum_cast<address_space>(addrspace), get_optional(loc));
    });
}

tinytc_status_t tinytc_group_type_get(tinytc_data_type_t *dt, tinytc_data_type_t memref_ty,
                                      int64_t offset, const tinytc_location_t *loc) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code(
        [&] { *dt = group_data_type::get(memref_ty, offset, get_optional(loc)); });
}
}
