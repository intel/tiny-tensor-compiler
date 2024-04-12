// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "location.hpp"
#include "node/data_type_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"
#include "util.hpp"

#include <memory>
#include <type_traits>
#include <utility>

using namespace tinytc;

extern "C" {
tinytc_status_t tinytc_scalar_type_create(tinytc_data_type_t *dt, tinytc_scalar_type_t type,
                                          const tinytc_location_t *loc) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        *dt = std::make_unique<scalar_data_type>(enum_cast<scalar_type>(type), get_optional(loc))
                  .release();
    });
}

tinytc_status_t tinytc_memref_type_create(tinytc_data_type_t *dt, tinytc_scalar_type_t scalar_ty,
                                          uint32_t shape_size, const int64_t *shape,
                                          uint32_t stride_size, const int64_t *stride,
                                          const tinytc_location_t *loc) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        auto shape_vec = std::vector<std::int64_t>(shape, shape + shape_size);
        auto stride_vec = std::vector<std::int64_t>();
        if (stride_size > 0) {
            stride_vec.insert(stride_vec.end(), stride, stride + stride_size);
        }
        *dt = std::make_unique<memref_data_type>(enum_cast<scalar_type>(scalar_ty),
                                                 std::move(shape_vec), std::move(stride_vec),
                                                 get_optional(loc))
                  .release();
    });
}

tinytc_status_t tinytc_group_type_create(tinytc_data_type_t *dt, tinytc_data_type_t memref_ty,
                                         const tinytc_location_t *loc) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        *dt = std::make_unique<group_data_type>(data_type(memref_ty, true), get_optional(loc))
                  .release();
    });
}

tinytc_status_t tinytc_data_type_release(tinytc_data_type_t dt) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }

    return exception_to_status_code([&] {
        auto ref_count = dt->dec_ref();
        if (ref_count == 0) {
            delete dt;
        }
    });
}

tinytc_status_t tinytc_data_type_retain(tinytc_data_type_t dt) {
    if (dt == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { dt->inc_ref(); });
}
}
