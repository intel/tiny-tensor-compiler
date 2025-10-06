// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "binary.hpp"
#include "error.hpp"
#include "tinytc/core.h"
#include "tinytc/types.h"
#include "util/casting.hpp"

#include <cstdint>
#include <memory>
#include <utility>

using namespace tinytc;

tinytc_binary::tinytc_binary(shared_handle<tinytc_compiler_context_t> ctx,
                             std::vector<std::uint8_t> data, bundle_format format,
                             tinytc_core_feature_flags_t core_features)
    : ctx_(std::move(ctx)), data_(std::move(data)), format_(format), core_features_(core_features) {
}

extern "C" {

tinytc_status_t tinytc_binary_create(tinytc_binary_t *bin, tinytc_compiler_context_t ctx,
                                     tinytc_bundle_format_t format, size_t data_size,
                                     uint8_t const *data,
                                     tinytc_core_feature_flags_t core_features) {
    if (bin == nullptr || ctx == nullptr || data == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] {
        *bin = std::make_unique<tinytc_binary>(shared_handle{ctx, true},
                                               std::vector<std::uint8_t>(data, data + data_size),
                                               enum_cast<bundle_format>(format), core_features)
                   .release();
    });
}

tinytc_status_t tinytc_binary_get_raw(const_tinytc_binary_t bin, tinytc_bundle_format_t *format,
                                      size_t *data_size, uint8_t const **data) {
    if (bin == nullptr || format == nullptr || data_size == nullptr || data == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    *format = static_cast<tinytc_bundle_format_t>(bin->format());
    *data_size = bin->size();
    *data = bin->data();
    return tinytc_status_success;
}

tinytc_status_t tinytc_binary_get_compiler_context(const_tinytc_binary_t bin,
                                                   tinytc_compiler_context_t *ctx) {
    if (bin == nullptr || ctx == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    return exception_to_status_code([&] { *ctx = bin->context(); });
}

tinytc_status_t tinytc_binary_get_core_features(const_tinytc_binary_t bin,
                                                tinytc_core_feature_flags_t *core_features) {
    if (bin == nullptr || core_features == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    *core_features = bin->core_features();
    return tinytc_status_success;
}

tinytc_status_t tinytc_binary_release(tinytc_binary_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    auto ref_count = obj->dec_ref();
    if (ref_count == 0) {
        delete obj;
    }
    return tinytc_status_success;
}

tinytc_status_t tinytc_binary_retain(tinytc_binary_t obj) {
    if (obj == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    obj->inc_ref();
    return tinytc_status_success;
}
}
