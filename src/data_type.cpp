// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "compiler_context.hpp"
#include "compiler_context_cache.hpp"
#include "error.hpp"
#include "location.hpp"
#include "node/data_type_node.hpp"
#include "support/util.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "tinytc/types.h"
#include "tinytc/types.hpp"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

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

tinytc_status_t tinytc_memref_type_get(tinytc_data_type_t *dt, tinytc_compiler_context_t ctx,
                                       tinytc_scalar_type_t scalar_ty, uint32_t shape_size,
                                       const int64_t *shape, uint32_t stride_size,
                                       const int64_t *stride, tinytc_address_space_t addrspace,
                                       const tinytc_location_t *loc) {
    if (dt == nullptr || ctx == nullptr || (shape_size != 0 && shape == nullptr) ||
        (stride_size != 0 && stride == nullptr)) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        auto shape_span = std::span<const std::int64_t>{};
        if (shape != nullptr) {
            shape_span = std::span<const std::int64_t>(shape, static_cast<std::size_t>(shape_size));
        }
        auto stride_span = std::span<const std::int64_t>{};
        if (stride != nullptr) {
            stride_span =
                std::span<const std::int64_t>(stride, static_cast<std::size_t>(stride_size));
        }

        *dt = memref_data_type::get(ctx, enum_cast<scalar_type>(scalar_ty), std::move(shape_span),
                                    std::move(stride_span), enum_cast<address_space>(addrspace),
                                    get_optional(loc));
    });
}

tinytc_status_t tinytc_group_type_get(tinytc_data_type_t *dt, tinytc_compiler_context_t ctx,
                                      tinytc_data_type_t memref_ty, int64_t offset,
                                      const tinytc_location_t *loc) {
    if (dt == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code(
        [&] { *dt = group_data_type::get(ctx, memref_ty, offset, get_optional(loc)); });
}
}
