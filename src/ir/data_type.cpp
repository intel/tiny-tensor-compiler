// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "error.hpp"
#include "ir/node/data_type_node.hpp"
#include "tinytc/tinytc.h"
#include "tinytc/tinytc.hpp"

#include <memory>
#include <utility>

extern "C" {
tinytc_status_t tinytc_scalar_type_create(tinytc_data_type_t *dt, tinytc_scalar_type_t type) {
    if (dt == nullptr) {
        return tinytc_invalid_arguments;
    }

    return tinytc::exception_to_status_code([&] {
        auto st = tinytc::scalar_type{std::underlying_type_t<tinytc::scalar_type>(type)};
        *dt = std::make_unique<tinytc::scalar_data_type>(st).release();
    });
}

tinytc_status_t TINYTC_EXPORT tinytc_memref_type_create(tinytc_data_type_t *dt,
                                                        tinytc_scalar_type_t scalar_ty,
                                                        uint32_t shape_size, int64_t const *shape,
                                                        uint32_t stride_size,
                                                        int64_t const *stride) {
    if (dt == nullptr) {
        return tinytc_invalid_arguments;
    }

    return tinytc::exception_to_status_code([&] {
        auto st = tinytc::scalar_type{std::underlying_type_t<tinytc::scalar_type>(scalar_ty)};
        auto shape_vec = std::vector<std::int64_t>(shape, shape + shape_size);
        auto stride_vec = std::vector<std::int64_t>();
        if (stride_size > 0) {
            stride_vec.insert(stride_vec.end(), stride, stride + stride_size);
        }
        *dt = std::make_unique<tinytc::memref_data_type>(st, std::move(shape_vec),
                                                         std::move(stride_vec))
                  .release();
    });
}

tinytc_status_t TINYTC_EXPORT tinytc_group_type_create(tinytc_data_type_t *dt,
                                                       tinytc_data_type_t memref_ty) {
    if (dt == nullptr) {
        return tinytc_invalid_arguments;
    }

    return tinytc::exception_to_status_code([&] {
        *dt =
            std::make_unique<tinytc::group_data_type>(tinytc::data_type(memref_ty, true)).release();
    });
}

tinytc_status_t tinytc_data_type_release(tinytc_data_type_t dt) {
    if (dt == nullptr) {
        return tinytc_invalid_arguments;
    }

    return tinytc::exception_to_status_code([&] {
        auto ref_count = dt->dec_ref();
        if (ref_count == 0) {
            delete dt;
        }
    });
}

tinytc_status_t tinytc_data_type_retain(tinytc_data_type_t dt) {
    if (dt == nullptr) {
        return tinytc_invalid_arguments;
    }
    return tinytc::exception_to_status_code([&] { dt->inc_ref(); });
}
}
