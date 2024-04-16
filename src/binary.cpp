// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "binary.hpp"
#include "error.hpp"
#include "tinytc/tinytc.h"

#include <cstdint>
#include <utility>

using namespace tinytc;

tinytc_binary::tinytc_binary(std::vector<std::uint8_t> data, bundle_format format,
                             std::uint32_t core_features)
    : data_(std::move(data)), format_(format), core_features_(core_features) {}

extern "C" {
tinytc_status_t tinytc_binary_get_raw(const_tinytc_binary_t bin, tinytc_bundle_format_t *format,
                                      uint64_t *data_size, uint8_t const **data) {
    if (bin == nullptr || format == nullptr || data_size == nullptr || data == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    *format = static_cast<tinytc_bundle_format_t>(bin->format());
    *data_size = static_cast<uint64_t>(bin->size());
    *data = bin->data();
    return tinytc_status_success;
}

tinytc_status_t tinytc_binary_get_core_features(const_tinytc_binary_t bin,
                                                uint32_t *core_features) {
    if (bin == nullptr || core_features == nullptr) {
        return tinytc_status_invalid_arguments;
    }
    *core_features = bin->core_features();
    return tinytc_status_success;
}

void tinytc_binary_destroy(tinytc_binary_t bin) { delete bin; }
}
