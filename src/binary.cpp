// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "binary.hpp"

#include <utility>

using namespace tinytc;

tinytc_binary::tinytc_binary(std::vector<std::uint8_t> data, bundle_format format,
                             std::unordered_map<std::string, kernel_metadata> metadata_map,
                             std::uint32_t core_features)
    : data_(std::move(data)), format_(format), metadata_(std::move(metadata_map)),
      core_features_(core_features) {}

