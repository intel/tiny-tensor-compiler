// Copyright (C) 2024 Intel Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "node/value_node.hpp"

tinytc_value::tinytc_value(tinytc_data_type_t ty, tinytc::location const &lc)
    : ty_{std::move(ty)}, loc_{lc} {}
